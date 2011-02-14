/* 
 * Copyright (C) 2010-2011 Daiki Ueno <ueno@unixuser.org>
 * Copyright (C) 2010-2011 Red Hat, Inc.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA
 */

#include <math.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif  /* HAVE_CONFIG_H */

#include "eek-key.h"
#include "eek-section.h"
#include "eek-renderer.h"

enum {
    PROP_0,
    PROP_KEYBOARD,
    PROP_PCONTEXT,
    PROP_LAST
};

G_DEFINE_TYPE (EekRenderer, eek_renderer, G_TYPE_OBJECT);

#define EEK_RENDERER_GET_PRIVATE(obj)                                  \
    (G_TYPE_INSTANCE_GET_PRIVATE ((obj), EEK_TYPE_RENDERER, EekRendererPrivate))

struct _EekRendererPrivate
{
    EekKeyboard *keyboard;
    PangoContext *pcontext;

    EekColor *foreground;
    EekColor *background;
    gdouble border_width;

    gdouble scale;
    PangoFontDescription *font;
    GHashTable *outline_surface_cache;
    cairo_surface_t *keyboard_surface;
    gulong symbol_index_changed_handler;
};

struct {
    gint category;
    gdouble scale;
} symbol_category_scale_factors[EEK_SYMBOL_CATEGORY_LAST] = {
    { EEK_SYMBOL_CATEGORY_LETTER, 1.0 },
    { EEK_SYMBOL_CATEGORY_FUNCTION, 0.5 },
    { EEK_SYMBOL_CATEGORY_KEYNAME, 0.5 }
};

/* eek-keyboard-drawing.c */
extern void _eek_rounded_polygon               (cairo_t     *cr,
                                                gdouble      radius,
                                                EekPoint    *points,
                                                gint         num_points);

static void eek_renderer_real_render_key_label (EekRenderer *self,
                                                PangoLayout *layout,
                                                EekKey      *key);

static void invalidate                         (EekRenderer *renderer);
static void render_key                         (EekRenderer *self,
                                                cairo_t     *cr,
                                                EekKey      *key);
static void on_symbol_index_changed            (EekKeyboard *keyboard,
                                                gint         group,
                                                gint         level,
                                                gpointer     user_data);

struct _CreateKeyboardSurfaceCallbackData {
    cairo_t *cr;
    EekRenderer *renderer;
};
typedef struct _CreateKeyboardSurfaceCallbackData CreateKeyboardSurfaceCallbackData;

static void
create_keyboard_surface_key_callback (EekElement *element,
                                      gpointer    user_data)
{
    CreateKeyboardSurfaceCallbackData *data = user_data;
    EekRendererPrivate *priv = EEK_RENDERER_GET_PRIVATE(data->renderer);
    EekBounds bounds;

    cairo_save (data->cr);

    eek_element_get_bounds (element, &bounds);
    cairo_translate (data->cr, bounds.x * priv->scale, bounds.y * priv->scale); 
    cairo_rectangle (data->cr,
                     0.0,
                     0.0,
                     bounds.width * priv->scale,
                     bounds.height * priv->scale);
    cairo_clip (data->cr);
    render_key (data->renderer, data->cr, EEK_KEY(element));

    cairo_restore (data->cr);
}

static void
create_keyboard_surface_section_callback (EekElement *element,
                                          gpointer    user_data)
{
    CreateKeyboardSurfaceCallbackData *data = user_data;
    EekRendererPrivate *priv = EEK_RENDERER_GET_PRIVATE(data->renderer);
    EekBounds bounds;
    gint angle;

    cairo_save (data->cr);

    eek_element_get_bounds (element, &bounds);
    cairo_translate (data->cr, bounds.x * priv->scale, bounds.y * priv->scale);

    angle = eek_section_get_angle (EEK_SECTION(element));
    cairo_rotate (data->cr, angle * G_PI / 180);
    
    eek_container_foreach_child (EEK_CONTAINER(element),
                                 create_keyboard_surface_key_callback,
                                 data);

    cairo_restore (data->cr);
}

static cairo_surface_t *
create_keyboard_surface (EekRenderer *renderer)
{
    EekRendererPrivate *priv = EEK_RENDERER_GET_PRIVATE(renderer);
    EekBounds bounds;
    cairo_surface_t *keyboard_surface;
    CreateKeyboardSurfaceCallbackData data;

    eek_element_get_bounds (EEK_ELEMENT(priv->keyboard), &bounds);
    keyboard_surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32,
                                                   bounds.width * priv->scale,
                                                   bounds.height * priv->scale);
    data.cr = cairo_create (keyboard_surface);
    data.renderer = renderer;

    cairo_translate (data.cr, bounds.x * priv->scale, bounds.y * priv->scale);

    /* blank background */
    cairo_set_source_rgba (data.cr,
                           priv->background->red,
                           priv->background->green,
                           priv->background->blue,
                           priv->background->alpha);
    cairo_rectangle (data.cr,
                     0.0,
                     0.0,
                     cairo_image_surface_get_width (keyboard_surface),
                     cairo_image_surface_get_height (keyboard_surface));
    cairo_fill (data.cr);

    /* draw sections */
    eek_container_foreach_child (EEK_CONTAINER(priv->keyboard),
                                 create_keyboard_surface_section_callback,
                                 &data);
    cairo_destroy (data.cr);
    return keyboard_surface;
}

static void
render_key_outline (EekRenderer *renderer,
                    cairo_t     *cr,
                    EekKey      *key)
{
    EekRendererPrivate *priv = EEK_RENDERER_GET_PRIVATE(renderer);
    EekOutline *outline;
    EekBounds bounds;
    cairo_pattern_t *pat;
    gdouble scale;
    gint i;
    gulong oref;

    /* need to rescale so that the border fit inside the clipping
       region */
    eek_element_get_bounds (EEK_ELEMENT(key), &bounds);
    scale = MIN((bounds.width - priv->border_width) / bounds.width,
                (bounds.height - priv->border_width) / bounds.height);

    oref = eek_key_get_oref (key);
    if (oref == 0)
        return;

    outline = eek_keyboard_get_outline (priv->keyboard, oref);
    outline = eek_outline_copy (outline);
    for (i = 0; i < outline->num_points; i++) {
        outline->points[i].x *= priv->scale * scale;
        outline->points[i].y *= priv->scale * scale;
    }

    cairo_translate (cr,
                     priv->border_width / 2 * priv->scale,
                     priv->border_width / 2 * priv->scale);

    /* paint the background with gradient */
    pat = cairo_pattern_create_linear (0.0,
                                       0.0,
                                       0.0,
                                       bounds.height * priv->scale * 5.0);
    cairo_pattern_add_color_stop_rgba (pat,
                                       1,
                                       priv->background->red * 0.5,
                                       priv->background->green * 0.5,
                                       priv->background->blue * 0.5,
                                       priv->background->alpha);
    cairo_pattern_add_color_stop_rgba (pat,
                                       0,
                                       priv->background->red,
                                       priv->background->green,
                                       priv->background->blue,
                                       priv->background->alpha);

    cairo_set_source (cr, pat);
    _eek_rounded_polygon (cr,
                          outline->corner_radius,
                          outline->points,
                          outline->num_points);
    cairo_fill (cr);

    cairo_pattern_destroy (pat);

    /* paint the border */
    cairo_set_line_width (cr, priv->border_width);
    cairo_set_line_join (cr, CAIRO_LINE_JOIN_ROUND);

    cairo_set_source_rgba
        (cr, 
         ABS(priv->background->red - priv->foreground->red) * 0.7,
         ABS(priv->background->green - priv->foreground->green) * 0.7,
         ABS(priv->background->blue - priv->foreground->blue) * 0.7,
         priv->foreground->alpha);

    _eek_rounded_polygon (cr,
                          outline->corner_radius,
                          outline->points,
                          outline->num_points);
    cairo_stroke (cr);

    eek_outline_free (outline);
}

struct _CalculateFontSizeCallbackData {
    gdouble size;
    gdouble em_size;
    EekRenderer *renderer;
};
typedef struct _CalculateFontSizeCallbackData CalculateFontSizeCallbackData;

static void
calculate_font_size_key_callback (EekElement *element, gpointer user_data)
{
    CalculateFontSizeCallbackData *data = user_data;
    EekRendererPrivate *priv = EEK_RENDERER_GET_PRIVATE(data->renderer);
    gdouble sx, sy;
    PangoFontDescription *font;
    const PangoFontDescription *base_font;
    PangoRectangle extents = { 0, };
    PangoLayout *layout;
    gdouble size;
    EekSymbol *symbol;
    EekBounds bounds;
    gchar *label = NULL;

    symbol = eek_key_get_symbol (EEK_KEY(element));
    if (symbol &&
        eek_symbol_get_category (symbol) == EEK_SYMBOL_CATEGORY_LETTER)
        label = eek_symbol_get_label (symbol);
    if (!label)
        label = g_strdup ("M");

    base_font = pango_context_get_font_description (priv->pcontext);
    font = pango_font_description_copy (base_font);

    eek_element_get_bounds (element, &bounds);
    size = eek_bounds_long_side (&bounds) * PANGO_SCALE;
    pango_font_description_set_size (font, size);
    layout = pango_layout_new (priv->pcontext);
    pango_layout_set_font_description (layout, font);
    pango_font_description_free (font);

    pango_layout_set_text (layout, label, -1);
    g_free (label);

    pango_layout_get_extents (layout, NULL, &extents);
    g_object_unref (layout);

    sx = sy = 1.0;
    if (extents.width > bounds.width * PANGO_SCALE)
        sx = bounds.width * PANGO_SCALE / extents.width;
    if (extents.height > bounds.height * PANGO_SCALE)
        sy = bounds.height * PANGO_SCALE / extents.height;

    size *= MIN(sx, sy);
    if (size >= pango_font_description_get_size (base_font)) {
        if (size < data->size)
            data->size = size;
        if (size < data->em_size)
            data->em_size = size;
    }
}

static void
calculate_font_size_section_callback (EekElement *element, gpointer user_data)
{
    eek_container_foreach_child (EEK_CONTAINER(element),
                                 calculate_font_size_key_callback,
                                 user_data);
}

static gdouble
calculate_font_size (EekRenderer *renderer)
{
    EekRendererPrivate *priv = EEK_RENDERER_GET_PRIVATE(renderer);
    CalculateFontSizeCallbackData data;
    PangoFontDescription *base_font;

    base_font = pango_context_get_font_description (priv->pcontext);
    data.size = G_MAXDOUBLE;
    data.em_size = G_MAXDOUBLE;
    data.renderer = renderer;
    eek_container_foreach_child (EEK_CONTAINER(priv->keyboard),
                                 calculate_font_size_section_callback,
                                 &data);
    return data.size > 0 ? data.size : data.em_size;
}

static EekKeyboard *
get_keyboard (EekKey *key)
{
    EekElement *parent;

    parent = eek_element_get_parent (EEK_ELEMENT(key));
    g_return_val_if_fail (EEK_IS_SECTION(parent), NULL);

    parent = eek_element_get_parent (parent);
    g_return_val_if_fail (EEK_IS_KEYBOARD(parent), NULL);

    return EEK_KEYBOARD(parent);
}

static void
render_key (EekRenderer *self,
            cairo_t     *cr,
            EekKey      *key)
{
    EekRendererPrivate *priv = EEK_RENDERER_GET_PRIVATE(self);
    EekOutline *outline;
    cairo_surface_t *outline_surface;
    EekBounds bounds;
    PangoLayout *layout;
    PangoRectangle extents = { 0, };
    gulong oref;
    EekKeyboard *keyboard;

    eek_element_get_bounds (EEK_ELEMENT(key), &bounds);
    oref = eek_key_get_oref (key);
    if (oref == 0)
        return;

    keyboard = get_keyboard (key);
    outline = eek_keyboard_get_outline (keyboard, oref);
    outline_surface = g_hash_table_lookup (priv->outline_surface_cache,
                                           outline);
    if (!outline_surface) {
        cairo_t *cr;

        outline_surface =
            cairo_image_surface_create (CAIRO_FORMAT_ARGB32,
                                        bounds.width * priv->scale,
                                        bounds.height * priv->scale);
        cr = cairo_create (outline_surface);

        /* blank background */
        eek_element_get_bounds (EEK_ELEMENT(key), &bounds);
        cairo_set_source_rgba (cr, 0.0, 0.0, 0.0, 0.0);
        cairo_rectangle (cr,
                         0.0,
                         0.0,
                         bounds.width * priv->scale,
                         bounds.height * priv->scale);
        cairo_fill (cr);

        render_key_outline (self, cr, key);

        g_hash_table_insert (priv->outline_surface_cache,
                             outline,
                             outline_surface);
    }

    cairo_set_source_surface (cr, outline_surface, 0.0, 0.0);
    cairo_paint (cr);

    layout = pango_cairo_create_layout (cr);
    eek_renderer_real_render_key_label (self, layout, key);
    pango_layout_get_extents (layout, NULL, &extents);

    cairo_save (cr);
    cairo_move_to
        (cr,
         (bounds.width * priv->scale - extents.width / PANGO_SCALE) / 2,
         (bounds.height * priv->scale - extents.height / PANGO_SCALE) / 2);
    cairo_set_source_rgba (cr,
                           priv->foreground->red,
                           priv->foreground->green,
                           priv->foreground->blue,
                           priv->foreground->alpha);
    pango_cairo_show_layout (cr, layout);
    cairo_restore (cr);
    g_object_unref (layout);
}

static void
prepare_render_key (EekRenderer *self,
                    cairo_t     *cr,
                    EekKey      *key,
                    gdouble      scale,
                    gboolean     rotate)
{
    EekElement *section;
    EekBounds bounds, rotated_bounds;
    gint angle;
    gdouble s;

    eek_renderer_get_key_bounds (self, key, &bounds, FALSE);
    eek_renderer_get_key_bounds (self, key, &rotated_bounds, TRUE);

    section = eek_element_get_parent (EEK_ELEMENT(key));
    angle = eek_section_get_angle (EEK_SECTION(section));

    cairo_scale (cr, scale, scale);
    if (rotate) {
        s = sin (angle * G_PI / 180);
        if (s < 0)
            cairo_translate (cr, 0, - bounds.width * s);
        else
            cairo_translate (cr, bounds.height * s, 0);
        cairo_rotate (cr, angle * G_PI / 180);
    }
}

static void
eek_renderer_real_render_key_label (EekRenderer *self,
                                    PangoLayout *layout,
                                    EekKey      *key)
{
    EekRendererPrivate *priv = EEK_RENDERER_GET_PRIVATE(self);
    EekSymbol *symbol;
    EekSymbolCategory category;
    gchar *label;
    EekBounds bounds;
    PangoFontDescription *font;
    gdouble size, scale;
    gint i;

    symbol = eek_key_get_symbol_with_fallback (key, 0, 0);
    if (!symbol)
        return;

    label = eek_symbol_get_label (symbol);
    if (!label)
        return;

    if (!priv->font) {
        PangoFontDescription *base_font;
        gdouble size;

        size = calculate_font_size (self);
        base_font = pango_context_get_font_description (priv->pcontext);
        priv->font = pango_font_description_copy (base_font);
        pango_font_description_set_size (priv->font, size);
    }

    eek_element_get_bounds (EEK_ELEMENT(key), &bounds);
    scale = MIN((bounds.width - priv->border_width) / bounds.width,
                (bounds.height - priv->border_width) / bounds.height);

    font = pango_font_description_copy (priv->font);
    size = pango_font_description_get_size (font);
    category = eek_symbol_get_category (symbol);
    for (i = 0; i < G_N_ELEMENTS(symbol_category_scale_factors); i++)
        if (symbol_category_scale_factors[i].category == category) {
            size *= symbol_category_scale_factors[i].scale;
            break;
        }
    pango_font_description_set_size (font, size * priv->scale * scale);
    pango_layout_set_font_description (layout, font);
    pango_layout_set_text (layout, label, -1);
    g_free (label);
    pango_layout_set_width (layout,
                            PANGO_SCALE * bounds.width * priv->scale * scale);
    pango_layout_set_ellipsize (layout, PANGO_ELLIPSIZE_END);
}

static void
eek_renderer_real_render_key_outline (EekRenderer *self,
                                      cairo_t     *cr,
                                      EekKey      *key,
                                      gdouble      scale,
                                      gboolean     rotate)
{
    cairo_save (cr);
    prepare_render_key (self, cr, key, scale, rotate);
    render_key_outline (self, cr, key);
    cairo_restore (cr);
}

static void
eek_renderer_real_render_key (EekRenderer *self,
                              cairo_t     *cr,
                              EekKey      *key,
                              gdouble      scale,
                              gboolean     rotate)
{
    cairo_save (cr);
    prepare_render_key (self, cr, key, scale, rotate);
    render_key (self, cr, key);
    cairo_restore (cr);
}

static void
eek_renderer_real_render_keyboard (EekRenderer *self,
                                   cairo_t     *cr)
{
    EekRendererPrivate *priv = EEK_RENDERER_GET_PRIVATE(self);

    if (!priv->keyboard_surface)
        priv->keyboard_surface = create_keyboard_surface (self);

    cairo_set_source_surface (cr, priv->keyboard_surface, 0.0, 0.0);
    cairo_paint (cr);
}

static void
eek_renderer_set_property (GObject      *object,
                           guint         prop_id,
                           const GValue *value,
                           GParamSpec   *pspec)
{
    EekRendererPrivate *priv = EEK_RENDERER_GET_PRIVATE(object);

    switch (prop_id) {
    case PROP_KEYBOARD:
        priv->keyboard = g_value_get_object (value);
        g_object_ref (priv->keyboard);

        priv->symbol_index_changed_handler =
            g_signal_connect (priv->keyboard, "symbol-index-changed",
                              G_CALLBACK(on_symbol_index_changed),
                              object);
        break;
    case PROP_PCONTEXT:
        priv->pcontext = g_value_get_object (value);
        g_object_ref (priv->pcontext);
        break;
    }
}

static void
eek_renderer_dispose (GObject *object)
{
    EekRendererPrivate *priv = EEK_RENDERER_GET_PRIVATE(object);

    if (priv->keyboard) {
        if (g_signal_handler_is_connected (priv->keyboard,
                                           priv->symbol_index_changed_handler))
            g_signal_handler_disconnect (priv->keyboard,
                                         priv->symbol_index_changed_handler);
        g_object_unref (priv->keyboard);
        priv->keyboard = NULL;
    }
    if (priv->pcontext) {
        g_object_unref (priv->pcontext);
        priv->pcontext = NULL;
    }

    /* this will release all allocated surfaces and font if any */
    invalidate (EEK_RENDERER(object));

    G_OBJECT_CLASS (eek_renderer_parent_class)->dispose (object);
}

static void
eek_renderer_finalize (GObject *object)
{
    EekRendererPrivate *priv = EEK_RENDERER_GET_PRIVATE(object);
    g_hash_table_destroy (priv->outline_surface_cache);
    G_OBJECT_CLASS (eek_renderer_parent_class)->finalize (object);
}

static void
eek_renderer_class_init (EekRendererClass *klass)
{
    GObjectClass      *gobject_class = G_OBJECT_CLASS (klass);
    GParamSpec        *pspec;

    g_type_class_add_private (gobject_class,
                              sizeof (EekRendererPrivate));

    klass->render_key_label = eek_renderer_real_render_key_label;
    klass->render_key_outline = eek_renderer_real_render_key_outline;
    klass->render_key = eek_renderer_real_render_key;
    klass->render_keyboard = eek_renderer_real_render_keyboard;

    gobject_class->set_property = eek_renderer_set_property;
    gobject_class->dispose = eek_renderer_dispose;
    gobject_class->finalize = eek_renderer_finalize;

    pspec = g_param_spec_object ("keyboard",
                                 "Keyboard",
                                 "Keyboard",
                                 EEK_TYPE_KEYBOARD,
                                 G_PARAM_WRITABLE);
    g_object_class_install_property (gobject_class,
                                     PROP_KEYBOARD,
                                     pspec);

    pspec = g_param_spec_object ("pango-context",
                                 "Pango Context",
                                 "Pango Context",
                                 PANGO_TYPE_CONTEXT,
                                 G_PARAM_WRITABLE);
    g_object_class_install_property (gobject_class,
                                     PROP_PCONTEXT,
                                     pspec);
}

static void
free_surface (gpointer data)
{
    cairo_surface_destroy (data);
}

static void
eek_renderer_init (EekRenderer *self)
{
    EekRendererPrivate *priv;

    priv = self->priv = EEK_RENDERER_GET_PRIVATE(self);
    priv->keyboard = NULL;
    priv->pcontext = NULL;
    priv->foreground = eek_color_new (0.3, 0.3, 0.3, 1.0);
    priv->background = eek_color_new (1.0, 1.0, 1.0, 1.0);
    priv->border_width = 3.0;
    priv->scale = 1.0;
    priv->font = NULL;
    priv->outline_surface_cache =
        g_hash_table_new_full (g_direct_hash,
                               g_direct_equal,
                               NULL,
                               free_surface);
    priv->keyboard_surface = NULL;
    priv->symbol_index_changed_handler = 0;
}

static void
invalidate (EekRenderer *renderer)
{
    EekRendererPrivate *priv = EEK_RENDERER_GET_PRIVATE(renderer);

    if (priv->outline_surface_cache)
        g_hash_table_remove_all (priv->outline_surface_cache);

    if (priv->keyboard_surface) {
        cairo_surface_destroy (priv->keyboard_surface);
        priv->keyboard_surface = NULL;
    }
}

static void
on_symbol_index_changed (EekKeyboard *keyboard,
                         gint         group,
                         gint         level,
                         gpointer     user_data)
{
    EekRenderer *renderer = user_data;
    invalidate (renderer);
}

EekRenderer *
eek_renderer_new (EekKeyboard  *keyboard,
                  PangoContext *pcontext)
{
    EekRenderer *renderer;

    renderer = g_object_new (EEK_TYPE_RENDERER,
                             "keyboard", keyboard,
                             "pango-context", pcontext,
                             NULL);

    return renderer;
}

void
eek_renderer_set_allocation_size (EekRenderer *renderer,
                                  gdouble      width,
                                  gdouble      height)
{
    EekRendererPrivate *priv;
    EekBounds bounds;
    gdouble scale;

    g_return_if_fail (EEK_IS_RENDERER(renderer));
    g_return_if_fail (width > 0.0 && height > 0.0);

    priv = EEK_RENDERER_GET_PRIVATE(renderer);

    eek_element_get_bounds (EEK_ELEMENT(priv->keyboard), &bounds);
    scale = width > height ? height / bounds.height :
        width / bounds.width;

    if (scale != priv->scale) {
        priv->scale = scale;
        invalidate (renderer);
    }
}

void
eek_renderer_get_size (EekRenderer *renderer,
                       gdouble     *width,
                       gdouble     *height)
{
    EekRendererPrivate *priv;
    EekBounds bounds;

    g_return_if_fail (EEK_IS_RENDERER(renderer));

    priv = EEK_RENDERER_GET_PRIVATE(renderer);

    eek_element_get_bounds (EEK_ELEMENT(priv->keyboard), &bounds);
    if (width)
        *width = bounds.width * priv->scale;
    if (height)
        *height = bounds.height * priv->scale;
}

void
eek_renderer_get_key_bounds (EekRenderer *renderer,
                             EekKey      *key,
                             EekBounds   *bounds,
                             gboolean     rotate)
{
    EekRendererPrivate *priv;
    EekElement *section;
    EekBounds section_bounds, keyboard_bounds;
    gint angle = 0;
    EekPoint points[4], min, max;
    gint i;

    g_return_if_fail (EEK_IS_RENDERER(renderer));
    g_return_if_fail (EEK_IS_KEY(key));
    g_return_if_fail (bounds != NULL);

    priv = EEK_RENDERER_GET_PRIVATE(renderer);

    section = eek_element_get_parent (EEK_ELEMENT(key));

    eek_element_get_bounds (EEK_ELEMENT(key), bounds);
    eek_element_get_bounds (section, &section_bounds);
    eek_element_get_bounds (EEK_ELEMENT(priv->keyboard), &keyboard_bounds);

    if (!rotate) {
        bounds->x += keyboard_bounds.x + section_bounds.x;
        bounds->y += keyboard_bounds.y + section_bounds.y;
        bounds->x *= priv->scale;
        bounds->y *= priv->scale;
        bounds->width *= priv->scale;
        bounds->height *= priv->scale;
        return;
    }
    points[0].x = bounds->x;
    points[0].y = bounds->y;
    points[1].x = points[0].x + bounds->width;
    points[1].y = points[0].y;
    points[2].x = points[1].x;
    points[2].y = points[1].y + bounds->height;
    points[3].x = points[0].x;
    points[3].y = points[2].y;

    if (rotate)
        angle = eek_section_get_angle (EEK_SECTION(section));

    min = points[2];
    max = points[0];
    for (i = 0; i < G_N_ELEMENTS(points); i++) {
        eek_point_rotate (&points[i], angle);
        if (points[i].x < min.x)
            min.x = points[i].x;
        if (points[i].x > max.x)
            max.x = points[i].x;
        if (points[i].y < min.y)
            min.y = points[i].y;
        if (points[i].y > max.y)
            max.y = points[i].y;
    }
    bounds->x = keyboard_bounds.x + section_bounds.x + min.x;
    bounds->y = keyboard_bounds.y + section_bounds.y + min.y;
    bounds->width = (max.x - min.x);
    bounds->height = (max.y - min.y);
    bounds->x *= priv->scale;
    bounds->y *= priv->scale;
    bounds->width *= priv->scale;
    bounds->height *= priv->scale;
}

gdouble
eek_renderer_get_scale (EekRenderer *renderer)
{
    EekRendererPrivate *priv;

    g_assert (EEK_IS_RENDERER(renderer));

    priv = EEK_RENDERER_GET_PRIVATE(renderer);

    return priv->scale;
}

PangoLayout *
eek_renderer_create_pango_layout (EekRenderer  *renderer)
{
    EekRendererPrivate *priv;

    g_assert (EEK_IS_RENDERER(renderer));

    priv = EEK_RENDERER_GET_PRIVATE(renderer);

    return pango_layout_new (priv->pcontext);
}

void
eek_renderer_render_key_label (EekRenderer *renderer,
                               PangoLayout *layout,
                               EekKey      *key)
{
    g_return_if_fail (EEK_IS_RENDERER(renderer));
    g_return_if_fail (EEK_IS_KEY(key));

    EEK_RENDERER_GET_CLASS(renderer)->
        render_key_label (renderer, layout, key);
}

void
eek_renderer_render_key_outline (EekRenderer *renderer,
                                 cairo_t     *cr,
                                 EekKey      *key,
                                 gdouble      scale,
                                 gboolean     rotate)
{
    g_return_if_fail (EEK_IS_RENDERER(renderer));
    g_return_if_fail (EEK_IS_KEY(key));
    g_return_if_fail (scale >= 0.0);

    EEK_RENDERER_GET_CLASS(renderer)->render_key_outline (renderer,
                                                          cr,
                                                          key,
                                                          scale,
                                                          rotate);
}

void
eek_renderer_render_key (EekRenderer *renderer,
                         cairo_t     *cr,
                         EekKey      *key,
                         gdouble      scale,
                         gboolean     rotate)
{
    g_return_if_fail (EEK_IS_RENDERER(renderer));
    g_return_if_fail (EEK_IS_KEY(key));
    g_return_if_fail (scale >= 0.0);

    EEK_RENDERER_GET_CLASS(renderer)->
        render_key (renderer, cr, key, scale, rotate);
}

void
eek_renderer_render_keyboard (EekRenderer *renderer,
                              cairo_t     *cr)
{
    g_return_if_fail (EEK_IS_RENDERER(renderer));
    EEK_RENDERER_GET_CLASS(renderer)->render_keyboard (renderer, cr);
}

void
eek_renderer_set_foreground (EekRenderer *renderer,
                             EekColor    *foreground)
{
    EekRendererPrivate *priv;

    g_return_if_fail (EEK_IS_RENDERER(renderer));
    g_return_if_fail (foreground);

    priv = EEK_RENDERER_GET_PRIVATE(renderer);
    priv->foreground = g_boxed_copy (EEK_TYPE_COLOR, foreground);
}

void
eek_renderer_set_background (EekRenderer *renderer,
                             EekColor    *background)
{
    EekRendererPrivate *priv;

    g_return_if_fail (EEK_IS_RENDERER(renderer));
    g_return_if_fail (background);

    priv = EEK_RENDERER_GET_PRIVATE(renderer);
    priv->background = g_boxed_copy (EEK_TYPE_COLOR, background);
}

void
eek_renderer_get_foreground (EekRenderer *renderer,
                             EekColor    *foreground)
{
    EekRendererPrivate *priv;

    g_return_if_fail (EEK_IS_RENDERER(renderer));
    g_return_if_fail (foreground);

    priv = EEK_RENDERER_GET_PRIVATE(renderer);
    *foreground = *priv->foreground;
}

void
eek_renderer_get_background (EekRenderer *renderer,
                             EekColor    *background)
{
    EekRendererPrivate *priv;

    g_return_if_fail (EEK_IS_RENDERER(renderer));
    g_return_if_fail (background);

    priv = EEK_RENDERER_GET_PRIVATE(renderer);
    *background = *priv->background;
}

struct _FindKeyByPositionCallbackData {
    EekPoint point;
    EekPoint origin;
    gint angle;
    EekKey *key;
    EekRenderer *renderer;
};
typedef struct _FindKeyByPositionCallbackData FindKeyByPositionCallbackData;

static gboolean
sign (EekPoint *p1, EekPoint *p2, EekPoint *p3)
{
    return (p1->x - p3->x) * (p2->y - p3->y) -
        (p2->x - p3->x) * (p1->y - p3->y);
}

static gint
find_key_by_position_key_callback (EekElement *element,
                                   gpointer user_data)
{
    FindKeyByPositionCallbackData *data = user_data;
    EekBounds bounds;
    EekRendererPrivate *priv;
    EekPoint points[4];
    gint i;
    gboolean b1, b2, b3;

    priv = EEK_RENDERER_GET_PRIVATE(data->renderer);

    eek_element_get_bounds (element, &bounds);

    points[0].x = bounds.x;
    points[0].y = bounds.y;
    points[1].x = points[0].x + bounds.width;
    points[1].y = points[0].y;
    points[2].x = points[1].x;
    points[2].y = points[1].y + bounds.height;
    points[3].x = points[0].x;
    points[3].y = points[2].y;

    for (i = 0; i < G_N_ELEMENTS(points); i++) {
        eek_point_rotate (&points[i], data->angle);
        points[i].x += data->origin.x;
        points[i].y += data->origin.y;
        points[i].x *= priv->scale;
        points[i].y *= priv->scale;
    }

    b1 = sign (&data->point, &points[0], &points[1]) < 0.0;
    b2 = sign (&data->point, &points[1], &points[2]) < 0.0;
    b3 = sign (&data->point, &points[2], &points[0]) < 0.0;

    if (b1 == b2 && b2 == b3) {
        data->key = EEK_KEY(element);
        return 0;
    }

    b1 = sign (&data->point, &points[2], &points[3]) < 0.0;
    b2 = sign (&data->point, &points[3], &points[0]) < 0.0;
    b3 = sign (&data->point, &points[0], &points[2]) < 0.0;

    if (b1 == b2 && b2 == b3) {
        data->key = EEK_KEY(element);
        return 0;
    }

    return -1;
}

static gint
find_key_by_position_section_callback (EekElement *element,
                                       gpointer user_data)
{
    FindKeyByPositionCallbackData *data = user_data;
    EekBounds bounds;
    EekRendererPrivate *priv;
    EekPoint origin;

    priv = EEK_RENDERER_GET_PRIVATE(data->renderer);

    origin = data->origin;
    eek_element_get_bounds (element, &bounds);
    data->origin.x += bounds.x;
    data->origin.y += bounds.y;
    data->angle = eek_section_get_angle (EEK_SECTION(element));

    eek_container_find (EEK_CONTAINER(element),
                        find_key_by_position_key_callback,
                        data);
    data->origin = origin;
    return data->key ? 0 : -1;
}

EekKey *
eek_renderer_find_key_by_position (EekRenderer *renderer,
                                   gdouble      x,
                                   gdouble      y)
{
    EekRendererPrivate *priv;
    EekBounds bounds;
    FindKeyByPositionCallbackData data;

    g_assert (EEK_IS_RENDERER(renderer));

    priv = EEK_RENDERER_GET_PRIVATE(renderer);
    eek_element_get_bounds (EEK_ELEMENT(priv->keyboard), &bounds);

    if (x < bounds.x * priv->scale ||
        y < bounds.y * priv->scale ||
        x > bounds.width * priv->scale ||
        y > bounds.height * priv->scale)
        return NULL;

    data.point.x = x;
    data.point.y = y;
    data.origin.x = bounds.x;
    data.origin.y = bounds.y;
    data.key = NULL;
    data.renderer = renderer;

    eek_container_find (EEK_CONTAINER(priv->keyboard),
                        find_key_by_position_section_callback,
                        &data);
    return data.key;
}
