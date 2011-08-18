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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif  /* HAVE_CONFIG_H */

#include <math.h>

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

    EekColor default_foreground_color;
    EekColor default_background_color;
    gdouble border_width;

    gdouble allocation_width;
    gdouble allocation_height;
    gdouble scale;

    PangoFontDescription *ascii_font;
    PangoFontDescription *font;
    GHashTable *outline_surface_cache;
    GHashTable *active_outline_surface_cache;
    cairo_surface_t *keyboard_surface;
    gulong symbol_index_changed_handler;

    EekTheme *theme;
};

static const EekColor DEFAULT_FOREGROUND_COLOR = {0.3, 0.3, 0.3, 1.0};
static const EekColor DEFAULT_BACKGROUND_COLOR = {1.0, 1.0, 1.0, 1.0};

struct _TextProperty {
    gint category;
    gboolean ascii;
    gdouble scale;
    gboolean ellipses;
};
typedef struct _TextProperty TextProperty;

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
                                                EekKey      *key,
                                                gboolean     active);
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
    render_key (data->renderer, data->cr, EEK_KEY(element), FALSE);

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
    EekColor foreground, background;

    eek_renderer_get_foreground_color (renderer,
                                       EEK_ELEMENT(priv->keyboard),
                                       &foreground);
    eek_renderer_get_background_color (renderer,
                                       EEK_ELEMENT(priv->keyboard),
                                       &background);

    eek_element_get_bounds (EEK_ELEMENT(priv->keyboard), &bounds);
    keyboard_surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32,
                                                   bounds.width * priv->scale,
                                                   bounds.height * priv->scale);
    data.cr = cairo_create (keyboard_surface);
    data.renderer = renderer;

    cairo_translate (data.cr, bounds.x * priv->scale, bounds.y * priv->scale);

    /* blank background */
    cairo_set_source_rgba (data.cr,
                           background.red,
                           background.green,
                           background.blue,
                           background.alpha);
    cairo_paint (data.cr);

    cairo_set_source_rgba (data.cr,
                           foreground.red,
                           foreground.green,
                           foreground.blue,
                           foreground.alpha);

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
                    EekKey      *key,
                    gboolean     active)
{
    EekRendererPrivate *priv = EEK_RENDERER_GET_PRIVATE(renderer);
    EekOutline *outline;
    EekBounds bounds;
    gdouble scale;
    gint i;
    gulong oref;
    EekThemeNode *theme_node;
    EekColor foreground, background, gradient_start, gradient_end, border_color;
    EekGradientType gradient_type;
    gint border_width;
    gint border_radius;

    oref = eek_key_get_oref (key);
    if (oref == 0)
        return;

    theme_node = g_object_get_data (G_OBJECT(key),
                                    active ?
                                    "theme-node-pressed" :
                                    "theme-node");
    if (theme_node) {
        eek_theme_node_get_foreground_color (theme_node, &foreground);
        eek_theme_node_get_background_color (theme_node, &background);
        eek_theme_node_get_background_gradient (theme_node,
                                                &gradient_type,
                                                &gradient_start,
                                                &gradient_end);
        border_width = eek_theme_node_get_border_width (theme_node,
                                                        EEK_SIDE_TOP);
        border_radius = eek_theme_node_get_border_radius (theme_node,
                                                          EEK_SIDE_TOP);
        eek_theme_node_get_border_color (theme_node, EEK_SIDE_TOP,
                                         &border_color);
    } else {
        foreground = priv->default_foreground_color;
        background = priv->default_background_color;
        gradient_type = EEK_GRADIENT_NONE;
        border_width = priv->border_width;
        border_radius = -1;
        border_color.red = ABS(background.red - foreground.red) * 0.7;
        border_color.green = ABS(background.green - foreground.green) * 0.7;
        border_color.blue = ABS(background.blue - foreground.blue) * 0.7;
        border_color.alpha = foreground.alpha;
    }

    /* need to rescale so that the border fit inside the clipping
       region */
    eek_element_get_bounds (EEK_ELEMENT(key), &bounds);
    scale = MIN((bounds.width - border_width) / bounds.width,
                (bounds.height - border_width) / bounds.height);

    outline = eek_keyboard_get_outline (priv->keyboard, oref);
    outline = eek_outline_copy (outline);
    for (i = 0; i < outline->num_points; i++) {
        outline->points[i].x *= priv->scale * scale;
        outline->points[i].y *= priv->scale * scale;
    }

    cairo_translate (cr,
                     border_width / 2 * priv->scale,
                     border_width / 2 * priv->scale);

    if (gradient_type != EEK_GRADIENT_NONE) {
        cairo_pattern_t *pat;
        gdouble cx, cy;

        switch (gradient_type) {
        case EEK_GRADIENT_VERTICAL:
            pat = cairo_pattern_create_linear (0.0,
                                               0.0,
                                               0.0,
                                               bounds.height * priv->scale);
            break;
        case EEK_GRADIENT_HORIZONTAL:
            pat = cairo_pattern_create_linear (0.0,
                                               0.0,
                                               bounds.width * priv->scale,
                                               0.0);
            break;
        case EEK_GRADIENT_RADIAL:
            cx = bounds.width / 2 * priv->scale;
            cy = bounds.height / 2 * priv->scale;
            pat = cairo_pattern_create_radial (cx,
                                               cy,
                                               0,
                                               cx,
                                               cy,
                                               MIN(cx, cy));
            break;
        default:
            g_assert_not_reached ();
            break;
        }

        cairo_pattern_add_color_stop_rgba (pat,
                                           1,
                                           gradient_start.red * 0.5,
                                           gradient_start.green * 0.5,
                                           gradient_start.blue * 0.5,
                                           gradient_start.alpha);
        cairo_pattern_add_color_stop_rgba (pat,
                                           0,
                                           gradient_end.red,
                                           gradient_end.green,
                                           gradient_end.blue,
                                           gradient_end.alpha);
        cairo_set_source (cr, pat);
        cairo_pattern_destroy (pat);
    } else {
        cairo_set_source_rgba (cr,
                               background.red,
                               background.green,
                               background.blue,
                               background.alpha);
    }

    _eek_rounded_polygon (cr,
                          border_radius >= 0 ? border_radius : outline->corner_radius,
                          outline->points,
                          outline->num_points);
    cairo_fill (cr);

    /* paint the border */
    cairo_set_line_width (cr, border_width);
    cairo_set_line_join (cr, CAIRO_LINE_JOIN_ROUND);

    cairo_set_source_rgba (cr,
                           border_color.red,
                           border_color.green,
                           border_color.blue,
                           border_color.alpha);

    _eek_rounded_polygon (cr,
                          border_radius >= 0 ? border_radius : outline->corner_radius,
                          outline->points,
                          outline->num_points);
    cairo_stroke (cr);

    eek_outline_free (outline);
}

struct _CalculateFontSizeCallbackData {
    gdouble size;
    gboolean ascii;
    EekRenderer *renderer;
    const PangoFontDescription *base_font;
};
typedef struct _CalculateFontSizeCallbackData CalculateFontSizeCallbackData;

static void
calculate_font_size_key_callback (EekElement *element, gpointer user_data)
{
    CalculateFontSizeCallbackData *data = user_data;
    EekRendererPrivate *priv = EEK_RENDERER_GET_PRIVATE(data->renderer);
    gdouble sx, sy;
    PangoFontDescription *font;
    PangoRectangle extents = { 0, };
    PangoLayout *layout;
    gdouble size;
    EekBounds bounds;
    const gchar *label = NULL;

    if (data->ascii)
        label = "M";
    else {
        EekSymbol *symbol;

        symbol = eek_key_get_symbol (EEK_KEY(element));
        if (symbol &&
            eek_symbol_get_category (symbol) == EEK_SYMBOL_CATEGORY_LETTER)
            label = eek_symbol_get_label (symbol);
        if (!label)
            label = "M";
    }

    font = pango_font_description_copy (data->base_font);

    eek_element_get_bounds (element, &bounds);
    size = eek_bounds_long_side (&bounds) * PANGO_SCALE;
    pango_font_description_set_size (font, size);
    layout = pango_layout_new (priv->pcontext);
    pango_layout_set_font_description (layout, font);
    pango_font_description_free (font);

    pango_layout_set_text (layout, label, -1);

    pango_layout_get_extents (layout, NULL, &extents);
    g_object_unref (layout);

    sx = sy = 1.0;
    if (extents.width > bounds.width * PANGO_SCALE)
        sx = bounds.width * PANGO_SCALE / extents.width;
    if (extents.height > bounds.height * PANGO_SCALE)
        sy = bounds.height * PANGO_SCALE / extents.height;

    size *= MIN(sx, sy);
    if (size < data->size)
        data->size = size;
}

static void
calculate_font_size_section_callback (EekElement *element, gpointer user_data)
{
    eek_container_foreach_child (EEK_CONTAINER(element),
                                 calculate_font_size_key_callback,
                                 user_data);
}

static gdouble
calculate_font_size (EekRenderer                *renderer,
                     const PangoFontDescription *base_font,
                     gboolean                    ascii)
{
    EekRendererPrivate *priv = EEK_RENDERER_GET_PRIVATE(renderer);
    CalculateFontSizeCallbackData data;

    data.size = G_MAXDOUBLE;
    data.ascii = ascii;
    data.renderer = renderer;
    data.base_font = base_font;
    eek_container_foreach_child (EEK_CONTAINER(priv->keyboard),
                                 calculate_font_size_section_callback,
                                 &data);
    return data.size;
}

static void
render_key (EekRenderer *self,
            cairo_t     *cr,
            EekKey      *key,
            gboolean     active)
{
    EekRendererPrivate *priv = EEK_RENDERER_GET_PRIVATE(self);
    EekOutline *outline;
    cairo_surface_t *outline_surface;
    EekBounds bounds;
    gulong oref;
    EekSymbol *symbol;
    GHashTable *outline_surface_cache;

    eek_element_get_bounds (EEK_ELEMENT(key), &bounds);
    oref = eek_key_get_oref (key);
    if (oref == 0)
        return;

    if (active)
        outline_surface_cache = priv->active_outline_surface_cache;
    else
        outline_surface_cache = priv->outline_surface_cache;

    outline = eek_keyboard_get_outline (priv->keyboard, oref);
    outline_surface = g_hash_table_lookup (outline_surface_cache, outline);
    if (!outline_surface) {
        cairo_t *cr;

        outline_surface =
            cairo_image_surface_create (CAIRO_FORMAT_ARGB32,
                                        bounds.width * priv->scale,
                                        bounds.height * priv->scale);
        cr = cairo_create (outline_surface);

        /* blank background */
        cairo_set_source_rgba (cr, 0.0, 0.0, 0.0, 0.0);
        cairo_paint (cr);

        cairo_save (cr);
        eek_renderer_apply_transformation_for_key (self, cr, key, 1.0, FALSE);
        render_key_outline (self, cr, key, active);
        cairo_restore (cr);

        cairo_destroy (cr);

        g_hash_table_insert (outline_surface_cache,
                             outline,
                             outline_surface);
    }

    cairo_set_source_surface (cr, outline_surface, 0.0, 0.0);
    cairo_paint (cr);

    symbol = eek_key_get_symbol_with_fallback (key, 0, 0);
    if (EEK_RENDERER_GET_CLASS(self)->render_key_icon &&
        symbol && eek_symbol_get_icon_name (symbol)) {
        eek_renderer_render_key_icon (self, cr, key, 1.0, 0);
    } else {
        PangoLayout *layout;
        PangoRectangle extents = { 0, };
        EekColor foreground;

        layout = pango_cairo_create_layout (cr);
        eek_renderer_render_key_label (self, layout, key);
        pango_layout_get_extents (layout, NULL, &extents);

        cairo_save (cr);
        cairo_move_to
            (cr,
             (bounds.width * priv->scale - extents.width / PANGO_SCALE) / 2,
             (bounds.height * priv->scale - extents.height / PANGO_SCALE) / 2);

        eek_renderer_get_foreground_color (self, EEK_ELEMENT(key), &foreground);
        cairo_set_source_rgba (cr,
                               foreground.red,
                               foreground.green,
                               foreground.blue,
                               foreground.alpha);

        pango_cairo_show_layout (cr, layout);
        cairo_restore (cr);
        g_object_unref (layout);
    }
}

void
eek_renderer_apply_transformation_for_key (EekRenderer *self,
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

static const TextProperty *
get_text_property_for_category (EekSymbolCategory category)
{
    static const TextProperty props[EEK_SYMBOL_CATEGORY_LAST] = {
        { EEK_SYMBOL_CATEGORY_LETTER, FALSE, 1.0, FALSE },
        { EEK_SYMBOL_CATEGORY_FUNCTION, TRUE, 0.5, FALSE },
        { EEK_SYMBOL_CATEGORY_KEYNAME, TRUE, 0.5, TRUE }
    };
    gint i;

    for (i = 0; i < G_N_ELEMENTS(props); i++)
        if (props[i].category == category)
            return &props[i];

    g_return_val_if_reached (NULL);
}

static void
eek_renderer_real_render_key_label (EekRenderer *self,
                                    PangoLayout *layout,
                                    EekKey      *key)
{
    EekRendererPrivate *priv = EEK_RENDERER_GET_PRIVATE(self);
    EekSymbol *symbol;
    EekSymbolCategory category;
    const gchar *label;
    EekBounds bounds;
    const TextProperty *prop;
    PangoFontDescription *font;
    gdouble scale;

    symbol = eek_key_get_symbol_with_fallback (key, 0, 0);
    if (!symbol)
        return;

    label = eek_symbol_get_label (symbol);
    if (!label)
        return;

    if (!priv->font) {
        const PangoFontDescription *base_font;
        gdouble ascii_size, size;
        EekThemeNode *theme_node;

        theme_node = g_object_get_data (G_OBJECT(key), "theme-node");
        if (theme_node)
            base_font = eek_theme_node_get_font (theme_node);
        else
            base_font = pango_context_get_font_description (priv->pcontext);
        ascii_size = calculate_font_size (self, base_font, TRUE);
        priv->ascii_font = pango_font_description_copy (base_font);
        pango_font_description_set_size (priv->ascii_font, ascii_size);

        size = calculate_font_size (self, base_font, FALSE);
        priv->font = pango_font_description_copy (base_font);
        pango_font_description_set_size (priv->font, size);
    }

    eek_element_get_bounds (EEK_ELEMENT(key), &bounds);
    scale = MIN((bounds.width - priv->border_width) / bounds.width,
                (bounds.height - priv->border_width) / bounds.height);

    category = eek_symbol_get_category (symbol);
    prop = get_text_property_for_category (category);

    font = pango_font_description_copy (prop->ascii ?
                                        priv->ascii_font :
                                        priv->font);
    pango_font_description_set_size (font,
                                     pango_font_description_get_size (font) *
                                     prop->scale * priv->scale * scale);
    pango_layout_set_font_description (layout, font);
    pango_font_description_free (font);

    pango_layout_set_text (layout, label, -1);
    pango_layout_set_width (layout,
                            PANGO_SCALE * bounds.width * priv->scale * scale);
    if (prop->ellipses)
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
    eek_renderer_apply_transformation_for_key (self, cr, key, scale, rotate);
    render_key_outline (self, cr, key, eek_key_is_pressed (key));
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
    eek_renderer_apply_transformation_for_key (self, cr, key, scale, rotate);
    render_key (self, cr, key, eek_key_is_pressed (key));
    cairo_restore (cr);
}

static void
eek_renderer_real_render_keyboard (EekRenderer *self,
                                   cairo_t     *cr)
{
    EekRendererPrivate *priv = EEK_RENDERER_GET_PRIVATE(self);
    cairo_pattern_t *source;

    g_return_if_fail (priv->keyboard);
    g_return_if_fail (priv->allocation_width > 0.0);
    g_return_if_fail (priv->allocation_height > 0.0);

    if (!priv->keyboard_surface)
        priv->keyboard_surface = create_keyboard_surface (self);

    cairo_set_source_surface (cr, priv->keyboard_surface, 0.0, 0.0);
    source = cairo_get_source (cr);
    cairo_pattern_set_extend (source, CAIRO_EXTEND_PAD);
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
    default:
        g_object_set_property (object,
                               g_param_spec_get_name (pspec),
                               value);
        break;
    }
}

static void
eek_renderer_get_property (GObject    *object,
                           guint       prop_id,
                           GValue     *value,
                           GParamSpec *pspec)
{
    EekRendererPrivate *priv = EEK_RENDERER_GET_PRIVATE(object);

    switch (prop_id) {
    case PROP_KEYBOARD:
        g_value_set_object (value, priv->keyboard);
        break;
    default:
        g_object_get_property (object,
                               g_param_spec_get_name (pspec),
                               value);
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
    g_hash_table_destroy (priv->active_outline_surface_cache);
    pango_font_description_free (priv->ascii_font);
    pango_font_description_free (priv->font);
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
    gobject_class->get_property = eek_renderer_get_property;
    gobject_class->dispose = eek_renderer_dispose;
    gobject_class->finalize = eek_renderer_finalize;

    pspec = g_param_spec_object ("keyboard",
                                 "Keyboard",
                                 "Keyboard",
                                 EEK_TYPE_KEYBOARD,
                                 G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE);
    g_object_class_install_property (gobject_class,
                                     PROP_KEYBOARD,
                                     pspec);

    pspec = g_param_spec_object ("pango-context",
                                 "Pango Context",
                                 "Pango Context",
                                 PANGO_TYPE_CONTEXT,
                                 G_PARAM_CONSTRUCT_ONLY | G_PARAM_WRITABLE);
    g_object_class_install_property (gobject_class,
                                     PROP_PCONTEXT,
                                     pspec);
}

static void
eek_renderer_init (EekRenderer *self)
{
    EekRendererPrivate *priv;

    priv = self->priv = EEK_RENDERER_GET_PRIVATE(self);
    priv->keyboard = NULL;
    priv->pcontext = NULL;
    priv->default_foreground_color = DEFAULT_FOREGROUND_COLOR;
    priv->default_background_color = DEFAULT_BACKGROUND_COLOR;
    priv->border_width = 1.0;
    priv->allocation_width = 0.0;
    priv->allocation_height = 0.0;
    priv->scale = 1.0;
    priv->font = NULL;
    priv->outline_surface_cache =
        g_hash_table_new_full (g_direct_hash,
                               g_direct_equal,
                               NULL,
                               (GDestroyNotify)cairo_surface_destroy);
    priv->active_outline_surface_cache =
        g_hash_table_new_full (g_direct_hash,
                               g_direct_equal,
                               NULL,
                               (GDestroyNotify)cairo_surface_destroy);
    priv->keyboard_surface = NULL;
    priv->symbol_index_changed_handler = 0;
}

static void
invalidate (EekRenderer *renderer)
{
    EekRendererPrivate *priv = EEK_RENDERER_GET_PRIVATE(renderer);

    if (priv->outline_surface_cache)
        g_hash_table_remove_all (priv->outline_surface_cache);

    if (priv->active_outline_surface_cache)
        g_hash_table_remove_all (priv->active_outline_surface_cache);

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

    priv->allocation_width = width;
    priv->allocation_height = height;

    eek_element_get_bounds (EEK_ELEMENT(priv->keyboard), &bounds);

    if (bounds.height * width / bounds.width <= height)
        scale = width / bounds.width;
    else if (bounds.width * height / bounds.height <= width)
        scale = height / bounds.height;
    else {
        if (bounds.width * height < bounds.height * width)
            scale = bounds.width / width;
        else
            scale = bounds.height / height;
    }

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
eek_renderer_render_key_icon (EekRenderer *renderer,
                                 cairo_t     *cr,
                                 EekKey      *key,
                                 gdouble      scale,
                                 gboolean     rotate)
{
    g_return_if_fail (EEK_IS_RENDERER(renderer));
    g_return_if_fail (EEK_IS_KEY(key));
    g_return_if_fail (scale >= 0.0);

    EEK_RENDERER_GET_CLASS(renderer)->render_key_icon (renderer,
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
eek_renderer_set_default_foreground_color (EekRenderer    *renderer,
                                           const EekColor *color)
{
    EekRendererPrivate *priv;

    g_return_if_fail (EEK_IS_RENDERER(renderer));
    g_return_if_fail (color);

    priv = EEK_RENDERER_GET_PRIVATE(renderer);
    priv->default_foreground_color = *color;
}

void
eek_renderer_set_default_background_color (EekRenderer    *renderer,
                                           const EekColor *color)
{
    EekRendererPrivate *priv;

    g_return_if_fail (EEK_IS_RENDERER(renderer));
    g_return_if_fail (color);

    priv = EEK_RENDERER_GET_PRIVATE(renderer);
    priv->default_background_color = *color;
}

void
eek_renderer_get_foreground_color (EekRenderer *renderer,
                                   EekElement  *element,
                                   EekColor    *color)
{
    EekRendererPrivate *priv;
    EekThemeNode *theme_node;

    g_return_if_fail (EEK_IS_RENDERER(renderer));
    g_return_if_fail (color);

    priv = EEK_RENDERER_GET_PRIVATE(renderer);

    theme_node = g_object_get_data (G_OBJECT(element), "theme-node");
    if (theme_node)
        eek_theme_node_get_foreground_color (theme_node, color);
    else
        *color = priv->default_foreground_color;
}

void
eek_renderer_get_background_color (EekRenderer *renderer,
                                   EekElement  *element,
                                   EekColor    *color)
{
    EekRendererPrivate *priv;
    EekThemeNode *theme_node;

    g_return_if_fail (EEK_IS_RENDERER(renderer));
    g_return_if_fail (color);

    priv = EEK_RENDERER_GET_PRIVATE(renderer);

    theme_node = g_object_get_data (G_OBJECT(element), "theme-node");
    if (theme_node)
        eek_theme_node_get_background_color (theme_node, color);
    else
        *color = priv->default_background_color;
}

void
eek_renderer_get_background_gradient (EekRenderer     *renderer,
                                      EekElement      *element,
                                      EekGradientType *type,
                                      EekColor        *start,
                                      EekColor        *end)
{
    EekThemeNode *theme_node;

    g_return_if_fail (EEK_IS_RENDERER(renderer));
    g_return_if_fail (EEK_IS_ELEMENT(element));
    g_return_if_fail (type);
    g_return_if_fail (start);
    g_return_if_fail (end);

    theme_node = g_object_get_data (G_OBJECT(element), "theme-node");
    if (theme_node)
        eek_theme_node_get_background_gradient (theme_node, type, start, end);
    else
        *type = EEK_GRADIENT_NONE;
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
    EekPoint origin;

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

struct _CreateThemeNodeData {
    EekThemeContext *context;
    EekThemeNode *parent;
    EekRenderer *renderer;
};
typedef struct _CreateThemeNodeData CreateThemeNodeData;

void
create_theme_node_key_callback (EekElement *element,
                                gpointer    user_data)
{
    CreateThemeNodeData *data = user_data;
    EekRendererPrivate *priv;
    EekThemeNode *theme_node;

    priv = EEK_RENDERER_GET_PRIVATE(data->renderer);

    theme_node = eek_theme_node_new (data->context,
                                     data->parent,
                                     priv->theme,
                                     EEK_TYPE_KEY,
                                     eek_element_get_name (element),
                                     "key",
                                     NULL,
                                     NULL);
    g_object_set_data_full (G_OBJECT(element),
                            "theme-node",
                            theme_node,
                            (GDestroyNotify)g_object_unref);

    theme_node = eek_theme_node_new (data->context,
                                     data->parent,
                                     priv->theme,
                                     EEK_TYPE_KEY,
                                     eek_element_get_name (element),
                                     "key",
                                     "active",
                                     NULL);
    g_object_set_data_full (G_OBJECT(element),
                            "theme-node-pressed",
                            theme_node,
                            (GDestroyNotify)g_object_unref);
}

void
create_theme_node_section_callback (EekElement *element,
                                    gpointer    user_data)
{
    CreateThemeNodeData *data = user_data;
    EekRendererPrivate *priv;
    EekThemeNode *theme_node, *parent;

    priv = EEK_RENDERER_GET_PRIVATE(data->renderer);

    theme_node = eek_theme_node_new (data->context,
                                     data->parent,
                                     priv->theme,
                                     EEK_TYPE_SECTION,
                                     eek_element_get_name (element),
                                     "section",
                                     NULL,
                                     NULL);
    g_object_set_data_full (G_OBJECT(element),
                            "theme-node",
                            theme_node,
                            (GDestroyNotify)g_object_unref);

    parent = data->parent;
    data->parent = theme_node;
    eek_container_foreach_child (EEK_CONTAINER(element),
                                 create_theme_node_key_callback,
                                 data);
    data->parent = parent;
}

void
eek_renderer_set_theme (EekRenderer *renderer,
                        EekTheme    *theme)
{
    EekRendererPrivate *priv;
    EekThemeContext *theme_context;
    EekThemeNode *theme_node;
    CreateThemeNodeData data;

    g_assert (EEK_IS_RENDERER(renderer));
    g_assert (EEK_IS_THEME(theme));

    priv = EEK_RENDERER_GET_PRIVATE(renderer);
    g_assert (priv->keyboard);

    if (priv->theme)
        g_object_unref (priv->theme);
    priv->theme = g_object_ref (theme);

    theme_context = eek_theme_context_new ();
    theme_node = eek_theme_node_new (theme_context,
                                     NULL,
                                     priv->theme,
                                     EEK_TYPE_KEYBOARD,
                                     "keyboard",
                                     "keyboard",
                                     NULL,
                                     NULL);
    g_object_set_data_full (G_OBJECT(priv->keyboard),
                            "theme-node",
                            theme_node,
                            (GDestroyNotify)g_object_unref);

    data.context = theme_context;
    data.parent = theme_node;
    data.renderer = renderer;
    eek_container_foreach_child (EEK_CONTAINER(priv->keyboard),
                                 create_theme_node_section_callback,
                                 &data);
}
