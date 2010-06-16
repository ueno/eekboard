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
 * @short_description: #EekKeyboard embedding a #GtkActor
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


struct _EekGtkKeyboardPrivate
{
    GtkWidget *widget;          /* GtkDrawingArea */
    GdkPixmap *pixmap;
    GdkColor *dark_color;
    cairo_t *cr;
    PangoLayout *layout;
    PangoFontDescription *fonts[EEK_KEYSYM_CATEGORY_LAST];
};

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

    for (i = 0; i < EEK_KEYSYM_CATEGORY_LAST; i++)
        pango_font_description_free (priv->fonts[i]);
}

static void
eek_gtk_keyboard_class_init (EekGtkKeyboardClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    g_type_class_add_private (gobject_class,
                              sizeof (EekGtkKeyboardPrivate));

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
    memset (priv->fonts, 0, sizeof priv->fonts);
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

static void
draw_key (EekElement *element, gpointer user_data)
{
    EekKeyboard *keyboard = user_data;
    EekGtkKeyboardPrivate *priv = EEK_GTK_KEYBOARD_GET_PRIVATE(keyboard);
    EekKey *key = EEK_KEY(element);
    EekOutline *outline;
    EekBounds bounds;
    guint keysym;

    cairo_save (priv->cr);
    eek_element_get_bounds (EEK_ELEMENT(key), &bounds);
    cairo_translate (priv->cr, bounds.x, bounds.y);
    outline = eek_key_get_outline (key);
    eek_draw_outline (priv->cr, outline);

    gdk_cairo_set_source_color (priv->cr, priv->dark_color);

    keysym = eek_key_get_keysym (key);
    if (keysym != EEK_INVALID_KEYSYM) {
        const gchar *label = eek_keysym_to_string (keysym);
        PangoRectangle logical_rect = { 0, };
        EekKeysymCategory category = eek_keysym_get_category (keysym);

        if (category != EEK_KEYSYM_CATEGORY_UNKNOWN && label) {
            pango_layout_set_font_description (priv->layout,
                                               priv->fonts[category]);
            pango_layout_set_text (priv->layout, label, -1);
            pango_layout_get_extents (priv->layout, NULL, &logical_rect);

            cairo_move_to
                (priv->cr,
                 (bounds.width - logical_rect.width / PANGO_SCALE) / 2,
                 (bounds.height - logical_rect.height / PANGO_SCALE) / 2);

            pango_layout_set_width (priv->layout, PANGO_SCALE * bounds.width);
            pango_layout_set_ellipsize (priv->layout, PANGO_ELLIPSIZE_END);
            pango_cairo_show_layout (priv->cr, priv->layout);
        }
    }
    cairo_restore (priv->cr);
}

static void
draw_section (EekElement *element, gpointer user_data)
{
    EekKeyboard *keyboard = user_data;
    EekGtkKeyboardPrivate *priv = EEK_GTK_KEYBOARD_GET_PRIVATE(keyboard);
    EekBounds bounds;

    gdk_cairo_set_source_color (priv->cr, priv->dark_color);
    eek_element_get_bounds (element, &bounds);

    cairo_save (priv->cr);
    cairo_translate (priv->cr, bounds.x, bounds.y);

#if 0
    cairo_rectangle (priv->cr, 0, 0, bounds.width, bounds.height);
    cairo_stroke (priv->cr);
#endif

    eek_container_foreach_child (EEK_CONTAINER(element), draw_key,
                                 keyboard);
    cairo_restore (priv->cr);
}

static gboolean
on_gtk_expose_event (GtkWidget      *widget,
                     GdkEventExpose *event,
                     gpointer        user_data)
{
    EekGtkKeyboard *keyboard = user_data;
    EekGtkKeyboardPrivate *priv =
        EEK_GTK_KEYBOARD_GET_PRIVATE(keyboard);
    GtkStateType state = gtk_widget_get_state (GTK_WIDGET (widget));

    if (!priv->pixmap) {
        GtkStateType state = gtk_widget_get_state (GTK_WIDGET (priv->widget));
        GtkAllocation allocation;
	PangoContext *context;
        PangoFontDescription *default_font_desc;
        
        context = gtk_widget_get_pango_context (GTK_WIDGET (priv->widget));
	priv->layout = pango_layout_new (context);

        /* compute font sizes */
        default_font_desc =
            gtk_widget_get_style (GTK_WIDGET(priv->widget))->font_desc;
        pango_layout_set_font_description (priv->layout, default_font_desc);
        eek_get_fonts (EEK_KEYBOARD(keyboard), priv->layout, priv->fonts);

        /* create priv->pixmap */
	gtk_widget_set_double_buffered (GTK_WIDGET (priv->widget), FALSE);
        gtk_widget_get_allocation (GTK_WIDGET (priv->widget), &allocation);
        priv->pixmap =
            gdk_pixmap_new (gtk_widget_get_window (GTK_WIDGET (priv->widget)),
                            allocation.width, allocation.height, -1);

        /* blank background */
        gdk_draw_rectangle
            (priv->pixmap,
             gtk_widget_get_style (GTK_WIDGET(priv->widget))->base_gc[state],
             TRUE,
             0, 0, allocation.width, allocation.height);

        /* draw sections on the canvas */
        priv->cr = gdk_cairo_create (GDK_DRAWABLE (priv->pixmap));
        priv->dark_color =
            &gtk_widget_get_style (GTK_WIDGET (priv->widget))->dark[state];

        eek_container_foreach_child (EEK_CONTAINER(keyboard), draw_section,
                                     keyboard);

        cairo_destroy (priv->cr);
        priv->cr = NULL;
        priv->dark_color = NULL;
    }

    gdk_draw_drawable (gtk_widget_get_window (widget),
                       gtk_widget_get_style (widget)->fg_gc[state],
                       priv->pixmap,
                       event->area.x, event->area.y,
                       event->area.x, event->area.y,
                       event->area.width, event->area.height);
    return TRUE;
}

GtkWidget *
eek_gtk_keyboard_get_widget (EekGtkKeyboard *keyboard)
{
    EekGtkKeyboardPrivate *priv =
        EEK_GTK_KEYBOARD_GET_PRIVATE(keyboard);

    if (!priv->widget) {
        priv->widget = gtk_drawing_area_new ();
        g_object_ref_sink (priv->widget);
        g_signal_connect (priv->widget, "expose_event",
                          G_CALLBACK (on_gtk_expose_event), keyboard);
        eek_keyboard_realize (EEK_KEYBOARD(keyboard));
    }
    return priv->widget;
}
