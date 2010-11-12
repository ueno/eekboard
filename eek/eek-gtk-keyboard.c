/* 
 * Copyright (C) 2010 Daiki Ueno <ueno@unixuser.org>
 * Copyright (C) 2010 Red Hat, Inc.
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

/**
 * SECTION:eek-gtk-keyboard
 * @short_description: #EekKeyboard that can be converted into a #GtkWidget
 */
#include <string.h>
#include <math.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif  /* HAVE_CONFIG_H */

#include "eek-gtk-keyboard.h"
#include "eek-drawing.h"
#include "eek-keyboard.h"
#include "eek-section.h"
#include "eek-key.h"
#include "eek-keysym.h"

G_DEFINE_TYPE (EekGtkKeyboard, eek_gtk_keyboard, EEK_TYPE_KEYBOARD);

#define EEK_GTK_KEYBOARD_GET_PRIVATE(obj)                                  \
    (G_TYPE_INSTANCE_GET_PRIVATE ((obj), EEK_TYPE_GTK_KEYBOARD, EekGtkKeyboardPrivate))

enum {
    KEY_SURFACE_NORMAL = 0,
    KEY_SURFACE_LARGE,
    KEY_SURFACE_LAST
};

static const gdouble key_surface_scale[KEY_SURFACE_LAST] = {
    1.0,
    1.5
};

struct _EekGtkKeyboardPrivate
{
    GtkWidget *widget;

    cairo_surface_t *keyboard_surface;
    GHashTable *key_surfaces;

    PangoFontDescription *fonts[EEK_KEYSYM_CATEGORY_LAST];

    gdouble scale;

    EekKey *key;
};

static void prepare_keyboard_surface (EekGtkKeyboard *keyboard);

static void
eek_gtk_keyboard_real_set_keysym_index (EekKeyboard *self,
                                        gint         group,
                                        gint         level)
{
    gint g, l;

    eek_keyboard_get_keysym_index (self, &g, &l);
    EEK_KEYBOARD_CLASS(eek_gtk_keyboard_parent_class)->
        set_keysym_index (self, group, level);
    if (g != group || l != level) {
        EekGtkKeyboard *keyboard = EEK_GTK_KEYBOARD(self);
        EekGtkKeyboardPrivate *priv =
            EEK_GTK_KEYBOARD_GET_PRIVATE(keyboard);
        GtkStyle *style;
        GtkStateType state;
        GtkAllocation allocation;
        cairo_t *cr;

        if (!priv->widget || !gtk_widget_get_realized (priv->widget))
            return;

        prepare_keyboard_surface (keyboard);
        gtk_widget_get_allocation (priv->widget, &allocation);

        cr = gdk_cairo_create (GDK_DRAWABLE (gtk_widget_get_window (priv->widget)));
        style = gtk_widget_get_style (priv->widget);
        state = gtk_widget_get_state (priv->widget);
        gdk_cairo_set_source_color (cr, &style->fg[state]);

        cairo_set_source_surface (cr, priv->keyboard_surface, 0, 0);
        cairo_rectangle (cr, 0, 0, allocation.width, allocation.height);
        cairo_fill (cr);
        cairo_destroy (cr);
    }
}

static void
eek_gtk_keyboard_dispose (GObject *object)
{
    EekGtkKeyboardPrivate *priv = EEK_GTK_KEYBOARD_GET_PRIVATE(object);

    if (priv->widget) {
        g_object_unref (priv->widget);
        priv->widget = NULL;
    }
    G_OBJECT_CLASS (eek_gtk_keyboard_parent_class)->dispose (object);
}

static void
eek_gtk_keyboard_finalize (GObject *object)
{
    EekGtkKeyboardPrivate *priv = EEK_GTK_KEYBOARD_GET_PRIVATE(object);
    gint i;

    g_hash_table_unref (priv->key_surfaces);

    for (i = 0; i < EEK_KEYSYM_CATEGORY_LAST; i++)
        pango_font_description_free (priv->fonts[i]);
    G_OBJECT_CLASS (eek_gtk_keyboard_parent_class)->finalize (object);
}

static void
eek_gtk_keyboard_class_init (EekGtkKeyboardClass *klass)
{
    EekKeyboardClass *keyboard_class = EEK_KEYBOARD_CLASS (klass);
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    g_type_class_add_private (gobject_class,
                              sizeof (EekGtkKeyboardPrivate));

    keyboard_class->set_keysym_index = eek_gtk_keyboard_real_set_keysym_index;
    gobject_class->dispose = eek_gtk_keyboard_dispose;
    gobject_class->finalize = eek_gtk_keyboard_finalize;
}

static void
key_surface_free (gpointer user_data)
{
    cairo_surface_t **key_surfaces = user_data;
    gint i;

    for (i = 0; i < KEY_SURFACE_LAST; i++)
        cairo_surface_destroy (key_surfaces[i]);
    g_slice_free1 (sizeof (cairo_surface_t *) * KEY_SURFACE_LAST, key_surfaces);
}

static void
eek_gtk_keyboard_init (EekGtkKeyboard *self)
{
    EekGtkKeyboardPrivate *priv;

    priv = self->priv = EEK_GTK_KEYBOARD_GET_PRIVATE(self);
    priv->widget = NULL;
    priv->keyboard_surface = NULL;
    priv->key_surfaces =
        g_hash_table_new_full (g_direct_hash,
                               g_direct_equal,
                               NULL,
                               key_surface_free);
    memset (priv->fonts, 0, sizeof priv->fonts);
    priv->scale = 1.0;
    priv->key = NULL;
}

/**
 * eek_gtk_keyboard_new:
 *
 * Create a new #EekGtkKeyboard.
 */
EekKeyboard*
eek_gtk_keyboard_new (void)
{
    return g_object_new (EEK_TYPE_GTK_KEYBOARD, NULL);
}

struct _DrawingContext
{
    EekGtkKeyboard *keyboard;
    cairo_t *cr;
    GdkColor *fg, *bg;
};
typedef struct _DrawingContext DrawingContext;

static void on_key_pressed (EekKey *key, gpointer user_data);
static void on_key_released (EekKey *key, gpointer user_data);

static void
prepare_keyboard_pixmap_key_callback (EekElement *element,
                                      gpointer    user_data)
{
    DrawingContext *context = user_data;
    EekGtkKeyboardPrivate *priv =
        EEK_GTK_KEYBOARD_GET_PRIVATE(context->keyboard);
    EekKey *key = EEK_KEY(element);
    EekBounds bounds;
    EekOutline *outline;
    cairo_surface_t **key_surfaces;

    eek_element_get_bounds (element, &bounds);

    g_signal_connect (key, "pressed", G_CALLBACK(on_key_pressed),
                      context->keyboard);
    g_signal_connect (key, "released", G_CALLBACK(on_key_released),
                      context->keyboard);

    outline = eek_key_get_outline (key);
    key_surfaces = g_hash_table_lookup (priv->key_surfaces, outline);
    if (!key_surfaces) {
        cairo_t *cr;
        gint i;

        key_surfaces = g_slice_alloc (sizeof (cairo_surface_t *) *
                                      KEY_SURFACE_LAST);
        for (i = 0; i < KEY_SURFACE_LAST; i++) {
            gdouble scale = key_surface_scale[i];
            key_surfaces[i] = cairo_image_surface_create (CAIRO_FORMAT_ARGB32,
                                                          bounds.width * scale,
                                                          bounds.height * scale);
            cr = cairo_create (key_surfaces[i]);
            cairo_scale (cr, priv->scale * scale, priv->scale * scale);
            gdk_cairo_set_source_color (cr, context->bg);
            cairo_rectangle (cr, 0, 0, bounds.width, bounds.height);
            gdk_cairo_set_source_color (cr, context->fg);
            eek_draw_outline (cr, outline);
            cairo_destroy (cr);
        }

        g_hash_table_insert (priv->key_surfaces, outline, key_surfaces);
    }

    cairo_save (context->cr);

    cairo_translate (context->cr, bounds.x, bounds.y);
    cairo_set_source_surface (context->cr, key_surfaces[0], 0, 0);
    cairo_rectangle (context->cr, 0, 0, bounds.width, bounds.height);
    cairo_fill (context->cr);

    cairo_move_to (context->cr, 0, 0);
    gdk_cairo_set_source_color (context->cr, context->fg);
    eek_draw_key_label (context->cr, key, priv->fonts);

    cairo_restore (context->cr);
}

static void
prepare_keyboard_pixmap_section_callback (EekElement *element,
                                          gpointer    user_data)
{
    DrawingContext *context = user_data;
    EekBounds bounds;
    gint angle;

    eek_element_get_bounds (element, &bounds);
    angle = eek_section_get_angle (EEK_SECTION(element));
    cairo_save (context->cr);
    cairo_translate (context->cr,
                     bounds.x,
                     bounds.y);
    cairo_rotate (context->cr, angle * M_PI / 180);
    eek_container_foreach_child (EEK_CONTAINER(element),
                                 prepare_keyboard_pixmap_key_callback,
                                 context);
    cairo_restore (context->cr);
}

static void
drawing_context_init (DrawingContext *context,
                      cairo_t        *cr,
                      EekGtkKeyboard *keyboard)
{
    EekGtkKeyboardPrivate *priv = keyboard->priv;
    GtkStyle *style;
    GtkStateType state;

    context->cr = cr;
    context->keyboard = keyboard;
    
    style = gtk_widget_get_style (priv->widget);
    state = gtk_widget_get_state (priv->widget);
    context->fg = &style->fg[state];
    context->bg = &style->bg[state];
}

static void
prepare_keyboard_surface (EekGtkKeyboard *keyboard)
{
    EekGtkKeyboardPrivate *priv = keyboard->priv;
    GtkAllocation allocation;
    GtkStyle *style;
    GtkStateType state;
    DrawingContext context;
    cairo_t *cr;

    gtk_widget_get_allocation (priv->widget, &allocation);
    priv->keyboard_surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32,
                                                         allocation.width,
                                                         allocation.height);

    /* blank background */
    cr = cairo_create (priv->keyboard_surface);
    cairo_scale (cr, priv->scale, priv->scale);

    style = gtk_widget_get_style (priv->widget);
    state = gtk_widget_get_state (priv->widget);
    gdk_cairo_set_source_color (cr, &style->base[state]);

    cairo_rectangle (cr, 0, 0, allocation.width, allocation.height);
    cairo_fill (cr);

    /* draw sections on the canvas */
    drawing_context_init (&context, cr, keyboard);
    eek_container_foreach_child (EEK_CONTAINER(keyboard),
                                 prepare_keyboard_pixmap_section_callback,
                                 &context);
    cairo_destroy (cr);
}

static gboolean
on_draw (GtkWidget *widget,
         cairo_t   *cr,
         gpointer   user_data)
{
    EekGtkKeyboard *keyboard = user_data;
    EekGtkKeyboardPrivate *priv =
        EEK_GTK_KEYBOARD_GET_PRIVATE(keyboard);
    GtkStyle *style;
    GtkStateType state;
    GtkAllocation allocation;

    g_return_val_if_fail (widget == priv->widget, FALSE);

    style = gtk_widget_get_style (widget);
    state = gtk_widget_get_state (widget);

    if (!priv->keyboard_surface) {
        PangoFontDescription *base_font;
        PangoContext *context;
        PangoLayout *layout;

        /* compute font sizes which fit in each key shape */
        context = gtk_widget_get_pango_context (widget);
        layout = pango_layout_new (context);
        base_font = style->font_desc;
        pango_layout_set_font_description (layout, base_font);
        eek_get_fonts (EEK_KEYBOARD(keyboard), layout, priv->fonts);
        g_object_unref (layout);

        prepare_keyboard_surface (keyboard);
    }
    g_return_val_if_fail (priv->keyboard_surface, FALSE);

    gdk_cairo_set_source_color (cr, &style->fg[state]);

    gtk_widget_get_allocation (widget, &allocation);
    cairo_set_source_surface (cr,
                              priv->keyboard_surface,
                              0, 0);
    cairo_rectangle (cr,
                     0, 0,
                     allocation.width, allocation.height);
    cairo_fill (cr);

    return TRUE;
}

static void
redraw_key (EekGtkKeyboard *keyboard,
            EekKey         *key,
            gint            key_surface_type)
{
    EekGtkKeyboardPrivate *priv =
        EEK_GTK_KEYBOARD_GET_PRIVATE(keyboard);
    EekBounds bounds;
    EekOutline *outline;
    gdouble x, y;
    int width, height;
    GtkStyle *style;
    GtkStateType state;
    cairo_surface_t **key_surfaces, *large_surface;
    cairo_t *cr;

    g_return_if_fail (priv->keyboard_surface);

    outline = eek_key_get_outline (key);
    key_surfaces = g_hash_table_lookup (priv->key_surfaces, outline);
    g_return_if_fail (key_surfaces);

    large_surface = key_surfaces[KEY_SURFACE_LARGE];
    width = cairo_image_surface_get_width (large_surface);
    height = cairo_image_surface_get_height (large_surface);

    eek_element_get_bounds (EEK_ELEMENT(key), &bounds);
    eek_element_get_absolute_position (EEK_ELEMENT(key), &x, &y);

    x -= (width - bounds.width) / 2;
    y -= (height - bounds.height) / 2;

    cr = gdk_cairo_create (GDK_DRAWABLE (gtk_widget_get_window (priv->widget)));

    style = gtk_widget_get_style (priv->widget);
    state = gtk_widget_get_state (priv->widget);
    gdk_cairo_set_source_color (cr, &style->fg[state]);

    switch (key_surface_type) {
    case KEY_SURFACE_NORMAL:
        cairo_set_source_surface (cr, priv->keyboard_surface, 0, 0);
        cairo_scale (cr, priv->scale, priv->scale);
        cairo_rectangle (cr, x, y, width, height);
        cairo_fill (cr);
        break;

    case KEY_SURFACE_LARGE:
        cairo_scale (cr, priv->scale, priv->scale);
        cairo_set_source_surface (cr, large_surface, x, y);
        cairo_rectangle (cr, x, y, width, height);
        cairo_fill (cr);

        cairo_move_to (cr, x, y);
        gdk_cairo_set_source_color (cr, &style->fg[state]);

        cairo_scale (cr,
                     key_surface_scale[KEY_SURFACE_LARGE],
                     key_surface_scale[KEY_SURFACE_LARGE]);
        eek_draw_key_label (cr, key, priv->fonts);
        break;
    }
    cairo_destroy (cr);
}

static void
on_key_pressed (EekKey *key, gpointer user_data)
{
    EekGtkKeyboard *keyboard = user_data;
    EekGtkKeyboardPrivate *priv = EEK_GTK_KEYBOARD_GET_PRIVATE(keyboard);

    redraw_key (EEK_GTK_KEYBOARD(keyboard), key, KEY_SURFACE_LARGE);
    priv->key = key;
}

static void
on_key_released (EekKey *key, gpointer user_data)
{
    EekGtkKeyboard *keyboard = user_data;
    EekGtkKeyboardPrivate *priv = EEK_GTK_KEYBOARD_GET_PRIVATE(keyboard);

    if (priv->key) {
        redraw_key (EEK_GTK_KEYBOARD(keyboard), priv->key, KEY_SURFACE_NORMAL);
        priv->key = NULL;
    }
    redraw_key (EEK_GTK_KEYBOARD(keyboard), key, KEY_SURFACE_NORMAL);
}

static void
press_key (EekGtkKeyboard *keyboard, EekKey *key)
{
    EekGtkKeyboardPrivate *priv = EEK_GTK_KEYBOARD_GET_PRIVATE(keyboard);
    if (priv->key != key)
        g_signal_emit_by_name (key, "pressed", keyboard);
}

static void
release_key (EekGtkKeyboard *keyboard)
{
    EekGtkKeyboardPrivate *priv = EEK_GTK_KEYBOARD_GET_PRIVATE(keyboard);

    if (priv->key)
        g_signal_emit_by_name (priv->key, "released", keyboard);
}

static gboolean
on_key_event (GtkWidget   *widget,
              GdkEventKey *event,
              gpointer     user_data)
{
    EekGtkKeyboard *keyboard = user_data;
    EekKey *key;

    key = eek_keyboard_find_key_by_keycode (EEK_KEYBOARD(keyboard),
                                            event->hardware_keycode);
    if (!key)
        return FALSE;
    switch (event->type) {
    case GDK_KEY_PRESS:
        press_key (keyboard, key);
        return TRUE;
    case GDK_KEY_RELEASE:
        release_key (keyboard);
        return TRUE;
    default:
        return FALSE;
    }
}

static gboolean
on_button_event (GtkWidget      *widget,
                 GdkEventButton *event,
                 gpointer        user_data)
{
    EekGtkKeyboard *keyboard = EEK_GTK_KEYBOARD(user_data);
    EekGtkKeyboardPrivate *priv = EEK_GTK_KEYBOARD_GET_PRIVATE(keyboard);
    EekKey *key;
    gdouble x, y;

    x = (gdouble)event->x / priv->scale;
    y = (gdouble)event->y / priv->scale;
    key = eek_keyboard_find_key_by_position (EEK_KEYBOARD(keyboard), x, y);
    if (key)
        switch (event->type) {
        case GDK_BUTTON_PRESS:
            press_key (EEK_GTK_KEYBOARD(keyboard), key);
            return TRUE;
        case GDK_BUTTON_RELEASE:
            release_key (EEK_GTK_KEYBOARD(keyboard));
            return TRUE;
        default:
            return FALSE;
        }
    return FALSE;
}

static void
on_size_allocate (GtkWidget     *widget,
                  GtkAllocation *allocation,
                  gpointer       user_data)
{
    EekGtkKeyboard *keyboard = user_data;
    EekGtkKeyboardPrivate *priv =
        EEK_GTK_KEYBOARD_GET_PRIVATE(keyboard);
    EekBounds bounds;

    if (priv->keyboard_surface) {
        cairo_surface_destroy (priv->keyboard_surface);
        priv->keyboard_surface = NULL;
    }

    eek_element_get_bounds (EEK_ELEMENT(keyboard), &bounds);
    priv->scale = allocation->width > allocation->height ?
        allocation->width / bounds.width :
        allocation->height / bounds.height;
}

GtkWidget *
eek_gtk_keyboard_get_widget (EekGtkKeyboard *keyboard)
{
    EekGtkKeyboardPrivate *priv =
        EEK_GTK_KEYBOARD_GET_PRIVATE(keyboard);

    if (!priv->widget) {
        priv->widget = gtk_drawing_area_new ();
        g_object_ref_sink (priv->widget);

        gtk_widget_set_double_buffered (priv->widget, FALSE);
        gtk_widget_set_events (priv->widget,
                               GDK_EXPOSURE_MASK |
                               GDK_KEY_PRESS_MASK |
                               GDK_KEY_RELEASE_MASK |
                               GDK_BUTTON_PRESS_MASK |
                               GDK_BUTTON_RELEASE_MASK);
        g_signal_connect (priv->widget, "draw",
                          G_CALLBACK (on_draw), keyboard);
        g_signal_connect (priv->widget, "size-allocate",
                          G_CALLBACK (on_size_allocate), keyboard);
        g_signal_connect (priv->widget, "key-press-event",
                          G_CALLBACK (on_key_event), keyboard);
        g_signal_connect (priv->widget, "key-release-event",
                          G_CALLBACK (on_key_event), keyboard);
        g_signal_connect (priv->widget, "button-press-event",
                          G_CALLBACK (on_button_event), keyboard);
        g_signal_connect (priv->widget, "button-release-event",
                          G_CALLBACK (on_button_event), keyboard);
        eek_keyboard_realize (EEK_KEYBOARD(keyboard));
    }
    return priv->widget;
}
