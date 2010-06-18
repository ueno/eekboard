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

#define SCALE 1.5

struct _EekGtkKeyboardPrivate
{
    GtkWidget *widget;

    /* pixmap of entire keyboard (for expose event) */
    GdkPixmap *pixmap;

    /* mapping from outline pointer to pixmap */
    GHashTable *outline_pixmaps;

    /* mapping from outline pointer to large pixmap */
    GHashTable *outline_large_pixmaps;

    PangoFontDescription *fonts[EEK_KEYSYM_CATEGORY_LAST];

    gdouble scale;
};

static void prepare_keyboard_pixmap (EekGtkKeyboard *keyboard);

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
        GtkStateType state;
        GtkAllocation allocation;

        prepare_keyboard_pixmap (keyboard);
        state = gtk_widget_get_state (GTK_WIDGET (priv->widget));
        gtk_widget_get_allocation (GTK_WIDGET (priv->widget), &allocation);
        gdk_draw_drawable (gtk_widget_get_window (priv->widget),
                           gtk_widget_get_style (priv->widget)->fg_gc[state],
                           priv->pixmap,
                           0, 0,
                           0, 0,
                           allocation.width, allocation.height);
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

    g_hash_table_unref (priv->outline_pixmaps);
    g_hash_table_unref (priv->outline_large_pixmaps);

    for (i = 0; i < EEK_KEYSYM_CATEGORY_LAST; i++)
        pango_font_description_free (priv->fonts[i]);
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
eek_gtk_keyboard_init (EekGtkKeyboard *self)
{
    EekGtkKeyboardPrivate *priv;

    priv = self->priv = EEK_GTK_KEYBOARD_GET_PRIVATE(self);
    priv->widget = NULL;
    priv->pixmap = NULL;
    priv->outline_pixmaps =
        g_hash_table_new_full (g_direct_hash,
                               g_direct_equal,
                               NULL,
                               g_object_unref);
    priv->outline_large_pixmaps =
        g_hash_table_new_full (g_direct_hash,
                               g_direct_equal,
                               NULL,
                               g_object_unref);
    memset (priv->fonts, 0, sizeof priv->fonts);
    priv->scale = 1.0;
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
    GdkPixmap *pixmap;

    eek_element_get_bounds (element, &bounds);

    outline = eek_key_get_outline (key);
    pixmap = g_hash_table_lookup (priv->outline_pixmaps, outline);
    if (!pixmap) {
        cairo_t *cr;

        pixmap =
            gdk_pixmap_new (gtk_widget_get_window (GTK_WIDGET (priv->widget)),
                            bounds.width, bounds.height, -1);
        cr = gdk_cairo_create (GDK_DRAWABLE (pixmap));
        gdk_cairo_set_source_color (cr, context->bg);
        cairo_rectangle (cr, 0, 0, bounds.width, bounds.height);
        gdk_cairo_set_source_color (cr, context->fg);
        eek_draw_outline (cr, outline);
        cairo_destroy (cr);
        g_hash_table_insert (priv->outline_pixmaps, outline, pixmap);
    }

    cairo_save (context->cr);
    cairo_translate (context->cr, bounds.x, bounds.y);

    gdk_cairo_set_source_pixmap (context->cr, pixmap, 0, 0);
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

    eek_element_get_bounds (element, &bounds);
    cairo_save (context->cr);
    cairo_translate (context->cr,
                     bounds.x,
                     bounds.y);
    eek_container_foreach_child (EEK_CONTAINER(element),
                                 prepare_keyboard_pixmap_key_callback,
                                 context);
    cairo_restore (context->cr);
}

static void
drawing_context_init (DrawingContext *context, EekGtkKeyboard *keyboard)
{
    EekGtkKeyboardPrivate *priv = keyboard->priv;
    GtkStateType state;

    state = gtk_widget_get_state (GTK_WIDGET (priv->widget));
    context->keyboard = keyboard;
    context->fg = &gtk_widget_get_style (GTK_WIDGET (priv->widget))->fg[state];
    context->bg = &gtk_widget_get_style (GTK_WIDGET (priv->widget))->bg[state];
}

static void
prepare_keyboard_pixmap (EekGtkKeyboard *keyboard)
{
    EekGtkKeyboardPrivate *priv = keyboard->priv;
    GtkAllocation allocation;
    GtkStateType state;
    DrawingContext context;

    gtk_widget_get_allocation (GTK_WIDGET (priv->widget), &allocation);
    priv->pixmap =
        gdk_pixmap_new (gtk_widget_get_window (GTK_WIDGET (priv->widget)),
                        allocation.width, allocation.height, -1);

    /* blank background */
    state = gtk_widget_get_state (GTK_WIDGET (priv->widget));
    gdk_draw_rectangle
        (priv->pixmap,
         gtk_widget_get_style (GTK_WIDGET(priv->widget))->base_gc[state],
         TRUE,
         0, 0, allocation.width, allocation.height);

    /* draw sections on the canvas */
    drawing_context_init (&context, keyboard);
    context.cr = gdk_cairo_create (GDK_DRAWABLE (priv->pixmap));
    cairo_scale (context.cr, priv->scale, priv->scale);
    eek_container_foreach_child (EEK_CONTAINER(keyboard),
                                 prepare_keyboard_pixmap_section_callback,
                                 &context);
    cairo_destroy (context.cr);
}

static gboolean
on_expose_event (GtkWidget      *widget,
                 GdkEventExpose *event,
                 gpointer        user_data)
{
    EekGtkKeyboard *keyboard = user_data;
    EekGtkKeyboardPrivate *priv =
        EEK_GTK_KEYBOARD_GET_PRIVATE(keyboard);
    GtkStateType state = gtk_widget_get_state (widget);

    if (!priv->pixmap) {
        /* compute font sizes which fit in each key shape */
        PangoFontDescription *base_font;
        PangoContext *context;
        PangoLayout *layout;

        context = gtk_widget_get_pango_context (priv->widget);
        layout = pango_layout_new (context);
        base_font = gtk_widget_get_style (priv->widget)->font_desc;
        pango_layout_set_font_description (layout, base_font);
        eek_get_fonts (EEK_KEYBOARD(keyboard), layout, priv->fonts);

        prepare_keyboard_pixmap (keyboard);
    }
    g_return_val_if_fail (priv->pixmap, FALSE);

    gdk_draw_drawable (gtk_widget_get_window (widget),
                       gtk_widget_get_style (widget)->fg_gc[state],
                       priv->pixmap,
                       event->area.x, event->area.y,
                       event->area.x, event->area.y,
                       event->area.width, event->area.height);
    return TRUE;
}

static void
key_enlarge (EekGtkKeyboard *keyboard, EekKey *key)
{
    EekGtkKeyboardPrivate *priv =
        EEK_GTK_KEYBOARD_GET_PRIVATE(keyboard);
    EekBounds bounds;
    EekOutline *outline;
    gdouble ax, ay;
    GdkPixmap *pixmap, *texture;
    DrawingContext context;
    GtkStateType state;
    cairo_t *cr;

    drawing_context_init (&context, keyboard);

    eek_element_get_bounds (EEK_ELEMENT(key), &bounds);
    eek_element_get_absolute_position (EEK_ELEMENT(key), &ax, &ay);

    outline = eek_key_get_outline (key);
    texture = g_hash_table_lookup (priv->outline_large_pixmaps, outline);
    if (!texture) {
        texture =
            gdk_pixmap_new (gtk_widget_get_window (GTK_WIDGET (priv->widget)),
                            bounds.width * SCALE, bounds.height * SCALE, -1);
        cr = gdk_cairo_create (GDK_DRAWABLE (texture));
        cairo_scale (cr, SCALE, SCALE);
        gdk_cairo_set_source_color (cr, context.bg);
        cairo_rectangle (cr, 0, 0, bounds.width, bounds.height);
        gdk_cairo_set_source_color (cr, context.fg);
        eek_draw_outline (cr, outline);
        cairo_destroy (cr);
        g_hash_table_insert (priv->outline_large_pixmaps, outline, texture);
    }

    pixmap =
        gdk_pixmap_new (gtk_widget_get_window (GTK_WIDGET (priv->widget)),
                        bounds.width * SCALE * priv->scale,
                        bounds.height * SCALE * priv->scale, -1);
    cr = gdk_cairo_create (GDK_DRAWABLE (pixmap));
    cairo_scale (cr, priv->scale, priv->scale);
    gdk_cairo_set_source_pixmap (cr, texture, 0, 0);
    cairo_rectangle (cr, 0, 0, bounds.width * SCALE, bounds.height * SCALE);
    cairo_fill (cr);

    cairo_move_to (cr, 0, 0);
    cairo_scale (cr, SCALE, SCALE);
    gdk_cairo_set_source_color (cr, context.fg);
    eek_draw_key_label (cr, key, priv->fonts);
    cairo_destroy (cr);

    state = gtk_widget_get_state (GTK_WIDGET (priv->widget));
    gdk_draw_drawable (gtk_widget_get_window (priv->widget),
                       gtk_widget_get_style (priv->widget)->fg_gc[state],
                       pixmap,
                       0, 0,
                       (ax - (bounds.width * SCALE - bounds.width) / 2) * priv->scale,
                       (ay - (bounds.height * SCALE - bounds.height) / 2) * priv->scale,
                       bounds.width * SCALE * priv->scale,
                       bounds.height * SCALE * priv->scale);
    g_object_unref (pixmap);
}

static void
key_shrink (EekGtkKeyboard *keyboard, EekKey *key)
{
    EekGtkKeyboardPrivate *priv =
        EEK_GTK_KEYBOARD_GET_PRIVATE(keyboard);
    EekBounds bounds;
    gdouble ax, ay;
    GtkStateType state;

    g_return_if_fail (priv->pixmap);
    eek_element_get_bounds (EEK_ELEMENT(key), &bounds);
    eek_element_get_absolute_position (EEK_ELEMENT(key), &ax, &ay);
    state = gtk_widget_get_state (GTK_WIDGET (priv->widget));

    ax -= (bounds.width * SCALE - bounds.width) / 2;
    ay -= (bounds.height * SCALE - bounds.height) / 2;

    gdk_draw_drawable (gtk_widget_get_window (priv->widget),
                       gtk_widget_get_style (priv->widget)->fg_gc[state],
                       priv->pixmap,
                       ax * priv->scale, ay * priv->scale,
                       ax * priv->scale, ay * priv->scale,
                       bounds.width * SCALE * priv->scale,
                       bounds.height * SCALE * priv->scale);
}

static gboolean
on_button_event (GtkWidget      *widget,
                     GdkEventButton *event,
                     gpointer        user_data)
{
    EekElement *keyboard = user_data, *section, *key;
    EekGtkKeyboardPrivate *priv = EEK_GTK_KEYBOARD_GET_PRIVATE(keyboard);
    EekBounds bounds;
    gdouble x, y;

    x = (gdouble)event->x / priv->scale;
    y = (gdouble)event->y / priv->scale;
    section = eek_container_find_by_position (EEK_CONTAINER(keyboard), x, y);
    if (section) {
        eek_element_get_bounds (keyboard, &bounds);
        x -= bounds.x;
        y -= bounds.y;
        key = eek_container_find_by_position (EEK_CONTAINER(section),
                                              x,
                                              y);
        if (key)
            switch (event->type) {
            case GDK_BUTTON_PRESS:
                key_enlarge (EEK_GTK_KEYBOARD(keyboard), EEK_KEY(key));
                g_signal_emit_by_name (keyboard, "key-pressed", key);
                return TRUE;
            case GDK_BUTTON_RELEASE:
                key_shrink (EEK_GTK_KEYBOARD(keyboard), EEK_KEY(key));
                g_signal_emit_by_name (keyboard, "key-released", key);
                return TRUE;
            default:
                return FALSE;
            }
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

    if (priv->pixmap) {
        g_object_unref (priv->pixmap);
        priv->pixmap = NULL;
    }
    g_hash_table_unref (priv->outline_pixmaps);
    g_hash_table_unref (priv->outline_large_pixmaps);
    priv->outline_pixmaps =
        g_hash_table_new_full (g_direct_hash,
                               g_direct_equal,
                               NULL,
                               g_object_unref);
    priv->outline_large_pixmaps =
        g_hash_table_new_full (g_direct_hash,
                               g_direct_equal,
                               NULL,
                               g_object_unref);

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
                               GDK_BUTTON_PRESS_MASK |
                               GDK_BUTTON_RELEASE_MASK);
        g_signal_connect (priv->widget, "expose_event",
                          G_CALLBACK (on_expose_event), keyboard);
        g_signal_connect (priv->widget, "size-allocate",
                          G_CALLBACK (on_size_allocate), keyboard);
        g_signal_connect (priv->widget, "button-press-event",
                          G_CALLBACK (on_button_event), keyboard);
        g_signal_connect (priv->widget, "button-release-event",
                          G_CALLBACK (on_button_event), keyboard);
        eek_keyboard_realize (EEK_KEYBOARD(keyboard));
    }
    return priv->widget;
}
