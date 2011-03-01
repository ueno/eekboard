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

/**
 * SECTION:eek-gtk-keyboard
 * @short_description: a #GtkWidget displaying #EekKeyboard
 */
#include <string.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif  /* HAVE_CONFIG_H */

#include "eek-gtk-keyboard.h"
#include "eek-gtk-renderer.h"
#include "eek-keyboard.h"
#include "eek-section.h"
#include "eek-key.h"
#include "eek-symbol.h"

enum {
    PROP_0,
    PROP_KEYBOARD,
    PROP_LAST
};

G_DEFINE_TYPE (EekGtkKeyboard, eek_gtk_keyboard, GTK_TYPE_DRAWING_AREA);

#define EEK_GTK_KEYBOARD_GET_PRIVATE(obj)                                  \
    (G_TYPE_INSTANCE_GET_PRIVATE ((obj), EEK_TYPE_GTK_KEYBOARD, EekGtkKeyboardPrivate))

/* since 2.91.5 GDK_DRAWABLE was removed and gdk_cairo_create takes
   GdkWindow as the argument */
#ifndef GDK_DRAWABLE
#define GDK_DRAWABLE(x) (x)
#endif

struct _EekGtkKeyboardPrivate
{
    EekRenderer *renderer;
    EekKeyboard *keyboard;
    EekKey *dragged_key;
    gulong key_pressed_handler;
    gulong key_released_handler;
    gulong symbol_index_changed_handler;
};

static EekColor * color_from_gdk_color    (GdkColor    *gdk_color);
static void       on_key_pressed          (EekKeyboard *keyboard,
                                           EekKey      *key,
                                           gpointer     user_data);
static void       on_key_released         (EekKeyboard *keyboard,
                                           EekKey      *key,
                                           gpointer     user_data);
static void       on_symbol_index_changed (EekKeyboard *keyboard,
                                           gint         group,
                                           gint         level,
                                           gpointer     user_data);
static void       render_pressed_key      (GtkWidget   *widget,
                                           EekKey      *key);

static void
eek_gtk_keyboard_real_realize (GtkWidget      *self)
{
    gtk_widget_set_double_buffered (self, FALSE);
    gtk_widget_set_events (self,
                           GDK_EXPOSURE_MASK |
                           GDK_KEY_PRESS_MASK |
                           GDK_KEY_RELEASE_MASK |
                           GDK_BUTTON_PRESS_MASK |
                           GDK_BUTTON_RELEASE_MASK);

    GTK_WIDGET_CLASS (eek_gtk_keyboard_parent_class)->realize (self);
}

static gboolean
eek_gtk_keyboard_real_draw (GtkWidget *self,
                            cairo_t   *cr)
{
    EekGtkKeyboardPrivate *priv = EEK_GTK_KEYBOARD_GET_PRIVATE(self);
    GtkAllocation allocation;

    gtk_widget_get_allocation (self, &allocation);

    if (!priv->renderer) {
        GtkStyle *style;
        GtkStateType state;
        PangoContext *pcontext;

        pcontext = gtk_widget_get_pango_context (self);
        priv->renderer = eek_gtk_renderer_new (priv->keyboard, pcontext, self);

        eek_renderer_set_allocation_size (priv->renderer,
                                          allocation.width,
                                          allocation.height);

        style = gtk_widget_get_style (self);
        state = gtk_widget_get_state (self);

        eek_renderer_set_foreground
            (priv->renderer,
             color_from_gdk_color (&style->fg[state]));
        eek_renderer_set_background
            (priv->renderer,
             color_from_gdk_color (&style->bg[state]));
    }

    eek_renderer_render_keyboard (priv->renderer, cr);

    /* redraw dragged key */
    if (priv->dragged_key)
        render_pressed_key (self, priv->dragged_key);

    return FALSE;
}

#if !GTK_CHECK_VERSION (2, 91, 2)
static gboolean
eek_gtk_keyboard_real_expose_event (GtkWidget      *self,
                                    GdkEventExpose *event)
{
    gboolean retval;
    cairo_t *cr;

    cr = gdk_cairo_create (GDK_DRAWABLE (gtk_widget_get_window (self)));
    retval = eek_gtk_keyboard_real_draw (self, cr);
    cairo_destroy (cr);

    return retval;
}
#endif  /* !GTK_CHECK_VERSION (2, 91, 2) */

static void
eek_gtk_keyboard_real_size_allocate (GtkWidget     *self,
                                     GtkAllocation *allocation)
{
    EekGtkKeyboardPrivate *priv = EEK_GTK_KEYBOARD_GET_PRIVATE(self);

    if (priv->renderer)
        eek_renderer_set_allocation_size (priv->renderer,
                                          allocation->width,
                                          allocation->height);

    GTK_WIDGET_CLASS (eek_gtk_keyboard_parent_class)->
        size_allocate (self, allocation);
}

static gboolean
eek_gtk_keyboard_real_button_press_event (GtkWidget      *self,
                                          GdkEventButton *event)
{
    EekGtkKeyboardPrivate *priv = EEK_GTK_KEYBOARD_GET_PRIVATE(self);
    EekKey *key;

    key = eek_renderer_find_key_by_position (priv->renderer,
                                             (gdouble)event->x,
                                             (gdouble)event->y);

    if (priv->dragged_key && priv->dragged_key != key)
        g_signal_emit_by_name (priv->dragged_key, "released", priv->keyboard);
    if (key && !eek_key_is_pressed (key)) {
        priv->dragged_key = key;
        g_signal_emit_by_name (key, "pressed", priv->keyboard);
    }

    return TRUE;
}

static gboolean
eek_gtk_keyboard_real_button_release_event (GtkWidget      *self,
                                            GdkEventButton *event)
{
    EekGtkKeyboardPrivate *priv = EEK_GTK_KEYBOARD_GET_PRIVATE(self);

    if (priv->dragged_key) {
        g_signal_emit_by_name (priv->dragged_key, "released", priv->keyboard);
        priv->dragged_key = NULL;
    }

    return TRUE;
}

static void
eek_gtk_keyboard_set_keyboard (EekGtkKeyboard *self,
                               EekKeyboard    *keyboard)
{
    EekGtkKeyboardPrivate *priv = EEK_GTK_KEYBOARD_GET_PRIVATE(self);
    priv->keyboard = g_object_ref (keyboard);

    priv->key_pressed_handler =
        g_signal_connect (priv->keyboard, "key-pressed",
                          G_CALLBACK(on_key_pressed), self);
    priv->key_released_handler =
        g_signal_connect (priv->keyboard, "key-released",
                          G_CALLBACK(on_key_released), self);
    priv->symbol_index_changed_handler =
        g_signal_connect (priv->keyboard, "symbol-index-changed",
                          G_CALLBACK(on_symbol_index_changed), self);
}

static void
eek_gtk_keyboard_set_property (GObject      *object,
                               guint         prop_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
    EekKeyboard *keyboard;

    switch (prop_id) {
    case PROP_KEYBOARD:
        keyboard = g_value_get_object (value);
        eek_gtk_keyboard_set_keyboard (EEK_GTK_KEYBOARD(object), keyboard);
        break;
    default:
        g_object_set_property (object,
                               g_param_spec_get_name (pspec),
                               value);
        break;
    }
}

static void
eek_gtk_keyboard_dispose (GObject *object)
{
    EekGtkKeyboardPrivate *priv = EEK_GTK_KEYBOARD_GET_PRIVATE(object);

    if (priv->renderer) {
        g_object_unref (priv->renderer);
        priv->renderer = NULL;
    }

    if (priv->keyboard) {
        if (g_signal_handler_is_connected (priv->keyboard,
                                           priv->key_pressed_handler))
            g_signal_handler_disconnect (priv->keyboard,
                                         priv->key_pressed_handler);
        if (g_signal_handler_is_connected (priv->keyboard,
                                           priv->key_released_handler))
            g_signal_handler_disconnect (priv->keyboard,
                                         priv->key_released_handler);
        if (g_signal_handler_is_connected (priv->keyboard,
                                           priv->symbol_index_changed_handler))
            g_signal_handler_disconnect (priv->keyboard,
                                         priv->symbol_index_changed_handler);
            
        if (priv->dragged_key) {
            g_signal_emit_by_name (priv->dragged_key,
                                   "released",
                                   priv->keyboard);
            priv->dragged_key = NULL;
        }

        g_object_unref (priv->keyboard);
        priv->keyboard = NULL;
    }
        
    G_OBJECT_CLASS (eek_gtk_keyboard_parent_class)->dispose (object);
}

static void
eek_gtk_keyboard_class_init (EekGtkKeyboardClass *klass)
{
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    GParamSpec *pspec;

    g_type_class_add_private (gobject_class,
                              sizeof (EekGtkKeyboardPrivate));

    widget_class->realize = eek_gtk_keyboard_real_realize;
#if GTK_CHECK_VERSION (2, 91, 2)
    widget_class->draw = eek_gtk_keyboard_real_draw;
#else  /* GTK_CHECK_VERSION (2, 91, 2) */
    widget_class->expose_event = eek_gtk_keyboard_real_expose_event;
#endif  /* !GTK_CHECK_VERSION (2, 91, 2) */
    widget_class->size_allocate = eek_gtk_keyboard_real_size_allocate;
    widget_class->button_press_event =
        eek_gtk_keyboard_real_button_press_event;
    widget_class->button_release_event =
        eek_gtk_keyboard_real_button_release_event;

    gobject_class->set_property = eek_gtk_keyboard_set_property;
    gobject_class->dispose = eek_gtk_keyboard_dispose;

    pspec = g_param_spec_object ("keyboard",
                                 "Keyboard",
                                 "Keyboard",
                                 EEK_TYPE_KEYBOARD,
                                 G_PARAM_CONSTRUCT_ONLY | G_PARAM_WRITABLE);
    g_object_class_install_property (gobject_class,
                                     PROP_KEYBOARD,
                                     pspec);
}

static void
eek_gtk_keyboard_init (EekGtkKeyboard *self)
{
    EekGtkKeyboardPrivate *priv;

    priv = self->priv = EEK_GTK_KEYBOARD_GET_PRIVATE(self);
    priv->renderer = NULL;
    priv->keyboard = NULL;
    priv->dragged_key = NULL;
}

/**
 * eek_gtk_keyboard_new:
 * @keyboard: an #EekKeyboard
 *
 * Create a new #GtkWidget displaying @keyboard.
 * Returns: a #GtkWidget
 */
GtkWidget *
eek_gtk_keyboard_new (EekKeyboard *keyboard)
{
    return g_object_new (EEK_TYPE_GTK_KEYBOARD, "keyboard", keyboard, NULL);
}

static EekColor *
color_from_gdk_color (GdkColor *gdk_color)
{
    EekColor *color;

    color = g_slice_new (EekColor);
    color->red = gdk_color->red / (gdouble)0xFFFF;
    color->green = gdk_color->green / (gdouble)0xFFFF;
    color->blue = gdk_color->blue / (gdouble)0xFFFF;
    color->alpha = 1.0;
    return color;
}

static void
magnify_bounds (EekBounds *bounds, EekBounds *large_bounds, gdouble scale)
{
    g_assert (scale >= 1.0);

    large_bounds->width = bounds->width * scale;
    large_bounds->height = bounds->height * scale;

    large_bounds->x = bounds->x - (large_bounds->width - bounds->width) / 2;
    large_bounds->y = bounds->y - (large_bounds->height - bounds->height) / 2;
}

static void
render_pressed_key (GtkWidget *widget,
                    EekKey    *key)
{
    EekGtkKeyboardPrivate *priv = EEK_GTK_KEYBOARD_GET_PRIVATE(widget);
    EekBounds bounds, large_bounds;
    cairo_t *cr;

    cr = gdk_cairo_create (GDK_DRAWABLE (gtk_widget_get_window (widget)));

    eek_renderer_get_key_bounds (priv->renderer, key, &bounds, TRUE);
    magnify_bounds (&bounds, &large_bounds, 1.5);

    cairo_translate (cr, large_bounds.x, large_bounds.y);
    eek_renderer_render_key (priv->renderer, cr, key, 1.5, TRUE);
    cairo_destroy (cr);
}

static void
on_key_pressed (EekKeyboard *keyboard,
                EekKey      *key,
                gpointer     user_data)
{
    GtkWidget *widget = user_data;
    EekGtkKeyboardPrivate *priv = EEK_GTK_KEYBOARD_GET_PRIVATE(widget);

    /* renderer may have not been set yet if the widget is a popup */
    if (!priv->renderer)
        return;

    render_pressed_key (widget, key);
}

static void
on_key_released (EekKeyboard *keyboard,
                 EekKey      *key,
                 gpointer     user_data)
{
    GtkWidget *widget = user_data;
    EekGtkKeyboardPrivate *priv = EEK_GTK_KEYBOARD_GET_PRIVATE(widget);
    cairo_t *cr;
    EekBounds bounds, large_bounds;

    /* renderer may have not been set yet if the widget is a popup */
    if (!priv->renderer)
        return;

    cr = gdk_cairo_create (GDK_DRAWABLE (gtk_widget_get_window (widget)));

    eek_renderer_get_key_bounds (priv->renderer, key, &bounds, TRUE);
    magnify_bounds (&bounds, &large_bounds, 2.0);
    cairo_rectangle (cr,
                     large_bounds.x,
                     large_bounds.y,
                     large_bounds.width,
                     large_bounds.height);
    cairo_clip (cr);
    eek_renderer_render_keyboard (priv->renderer, cr);
    cairo_destroy (cr);
}

static void
on_symbol_index_changed (EekKeyboard *keyboard,
                         gint         group,
                         gint         level,
                         gpointer     user_data)
{
    GtkWidget *widget = user_data;

    gtk_widget_queue_draw (widget);
}
