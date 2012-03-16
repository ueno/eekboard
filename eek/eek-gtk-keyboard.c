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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif  /* HAVE_CONFIG_H */

#ifdef HAVE_LIBCANBERRA
#include <canberra-gtk.h>
#endif

#include <string.h>

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
    gulong key_pressed_handler;
    gulong key_released_handler;
    gulong key_locked_handler;
    gulong key_unlocked_handler;
    gulong key_cancelled_handler;
    gulong symbol_index_changed_handler;
    EekTheme *theme;
};

static EekColor * color_from_gdk_color    (GdkColor    *gdk_color);
static void       on_key_pressed          (EekKeyboard *keyboard,
                                           EekKey      *key,
                                           gpointer     user_data);
static void       on_key_released         (EekKeyboard *keyboard,
                                           EekKey      *key,
                                           gpointer     user_data);
static void       on_key_locked          (EekKeyboard *keyboard,
                                           EekKey      *key,
                                           gpointer     user_data);
static void       on_key_unlocked         (EekKeyboard *keyboard,
                                           EekKey      *key,
                                           gpointer     user_data);
static void       on_key_cancelled        (EekKeyboard *keyboard,
                                           EekKey      *key,
                                           gpointer     user_data);
static void       on_symbol_index_changed (EekKeyboard *keyboard,
                                           gint         group,
                                           gint         level,
                                           gpointer     user_data);
static void       render_pressed_key      (GtkWidget   *widget,
                                           EekKey      *key);
static void       render_locked_key       (GtkWidget   *widget,
                                           EekKey      *key);
static void       render_released_key     (GtkWidget   *widget,
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
                           GDK_BUTTON_RELEASE_MASK |
                           GDK_BUTTON_MOTION_MASK);

    GTK_WIDGET_CLASS (eek_gtk_keyboard_parent_class)->realize (self);
}

static gboolean
eek_gtk_keyboard_real_draw (GtkWidget *self,
                            cairo_t   *cr)
{
    EekGtkKeyboardPrivate *priv = EEK_GTK_KEYBOARD_GET_PRIVATE(self);
    GtkAllocation allocation;
    EekColor background;
    GList *head;

    gtk_widget_get_allocation (self, &allocation);

    if (!priv->renderer) {
        GtkStyle *style;
        GtkStateType state;
        PangoContext *pcontext;
        EekColor *color;

        pcontext = gtk_widget_get_pango_context (self);
        priv->renderer = eek_gtk_renderer_new (priv->keyboard, pcontext, self);
        if (priv->theme)
            eek_renderer_set_theme (priv->renderer, priv->theme);

        eek_renderer_set_allocation_size (priv->renderer,
                                          allocation.width,
                                          allocation.height);

        style = gtk_widget_get_style (self);
        state = gtk_widget_get_state (self);

        color = color_from_gdk_color (&style->text[state]);
        eek_renderer_set_default_foreground_color (priv->renderer, color);
        eek_color_free (color);

        color = color_from_gdk_color (&style->base[state]);
        eek_renderer_set_default_background_color (priv->renderer, color);
        eek_color_free (color);
    }

    /* blank background */
    eek_renderer_get_background_color (priv->renderer,
                                       EEK_ELEMENT(priv->keyboard),
                                       &background);
    cairo_set_source_rgba (cr,
                           background.red,
                           background.green,
                           background.blue,
                           background.alpha);
    cairo_paint (cr);

    eek_renderer_render_keyboard (priv->renderer, cr);

    /* redraw pressed key */
    head = eek_keyboard_get_pressed_keys (priv->keyboard);
    for (; head; head = g_list_next (head)) {
        render_pressed_key (self, head->data);
    }

    /* redraw locked key */
    head = eek_keyboard_get_locked_keys (priv->keyboard);
    for (; head; head = g_list_next (head)) {
        render_locked_key (self, ((EekModifierKey *)head->data)->key);
    }

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
    if (key)
        g_signal_emit_by_name (key, "pressed", priv->keyboard);
    return TRUE;
}

static gboolean
eek_gtk_keyboard_real_button_release_event (GtkWidget      *self,
                                            GdkEventButton *event)
{
    EekGtkKeyboardPrivate *priv = EEK_GTK_KEYBOARD_GET_PRIVATE(self);
    GList *head = eek_keyboard_get_pressed_keys (priv->keyboard);

    /* Make a copy of HEAD before sending "released" signal on
       elements, so that the default handler of
       EekKeyboard::key-released signal can remove elements from its
       internal copy */
    head = g_list_copy (head);
    for (; head; head = g_list_next (head))
        g_signal_emit_by_name (head->data, "released", priv->keyboard);
    g_list_free (head);

    return TRUE;
}

static gboolean
eek_gtk_keyboard_real_motion_notify_event (GtkWidget      *self,
                                           GdkEventMotion *event)
{
    EekGtkKeyboardPrivate *priv = EEK_GTK_KEYBOARD_GET_PRIVATE(self);
    EekKey *key;

    key = eek_renderer_find_key_by_position (priv->renderer,
                                             (gdouble)event->x,
                                             (gdouble)event->y);
    if (key) {
        GList *head = eek_keyboard_get_pressed_keys (priv->keyboard);
        gboolean found = FALSE;

        /* Make a copy of HEAD before sending "cancelled" signal on
           elements, so that the default handler of
           EekKeyboard::key-cancelled signal can remove elements from its
           internal copy */
        head = g_list_copy (head);
        for (; head; head = g_list_next (head)) {
            if (head->data == key)
                found = TRUE;
            else
                g_signal_emit_by_name (head->data, "cancelled", priv->keyboard);
        }
        g_list_free (head);

        if (!found)
            g_signal_emit_by_name (key, "pressed", priv->keyboard);
    }
    return TRUE;
}

static void
eek_gtk_keyboard_real_unmap (GtkWidget *self)
{
    EekGtkKeyboardPrivate *priv = EEK_GTK_KEYBOARD_GET_PRIVATE(self);

    if (priv->keyboard) {
        GList *head = eek_keyboard_get_pressed_keys (priv->keyboard);

        /* Make a copy of HEAD before sending "released" signal on
           elements, so that the default handler of
           EekKeyboard::key-released signal can remove elements from its
           internal copy */
        head = g_list_copy (head);
        for (; head; head = g_list_next (head))
            g_signal_emit_by_name (head->data, "released", priv->keyboard);
        g_list_free (head);
    }

    GTK_WIDGET_CLASS (eek_gtk_keyboard_parent_class)->unmap (self);
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
    priv->key_locked_handler =
        g_signal_connect (priv->keyboard, "key-locked",
                          G_CALLBACK(on_key_locked), self);
    priv->key_unlocked_handler =
        g_signal_connect (priv->keyboard, "key-unlocked",
                          G_CALLBACK(on_key_unlocked), self);
    priv->key_cancelled_handler =
        g_signal_connect (priv->keyboard, "key-cancelled",
                          G_CALLBACK(on_key_cancelled), self);
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
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
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
                                           priv->key_locked_handler))
            g_signal_handler_disconnect (priv->keyboard,
                                         priv->key_locked_handler);
        if (g_signal_handler_is_connected (priv->keyboard,
                                           priv->key_unlocked_handler))
            g_signal_handler_disconnect (priv->keyboard,
                                         priv->key_unlocked_handler);
        if (g_signal_handler_is_connected (priv->keyboard,
                                           priv->key_cancelled_handler))
            g_signal_handler_disconnect (priv->keyboard,
                                         priv->key_cancelled_handler);
        if (g_signal_handler_is_connected (priv->keyboard,
                                           priv->symbol_index_changed_handler))
            g_signal_handler_disconnect (priv->keyboard,
                                         priv->symbol_index_changed_handler);
            
        GList *head;

        head = eek_keyboard_get_pressed_keys (priv->keyboard);
        for (; head; head = g_list_next (head)) {
            g_signal_emit_by_name (head->data, "released", priv->keyboard);
        }

        g_object_unref (priv->keyboard);
        priv->keyboard = NULL;
    }

    if (priv->theme) {
        g_object_unref (priv->theme);
        priv->theme = NULL;
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
    widget_class->unmap = eek_gtk_keyboard_real_unmap;
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
    widget_class->motion_notify_event =
        eek_gtk_keyboard_real_motion_notify_event;

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
    self->priv = EEK_GTK_KEYBOARD_GET_PRIVATE(self);
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
    return eek_color_new (gdk_color->red / (gdouble)0xFFFF,
                          gdk_color->green / (gdouble)0xFFFF,
                          gdk_color->blue / (gdouble)0xFFFF,
                          1.0);
}

static void
magnify_bounds (GtkWidget *self,
                EekBounds *bounds,
                EekBounds *large_bounds,
                gdouble    scale)
{
    GtkAllocation allocation;
    gdouble x, y;

    g_assert (scale >= 1.0);

    gtk_widget_get_allocation (self, &allocation);

    large_bounds->width = bounds->width * scale;
    large_bounds->height = bounds->height * scale;

    x = bounds->x - (large_bounds->width - bounds->width) / 2;
    y = bounds->y - large_bounds->height;

    large_bounds->x = CLAMP(x, 0, allocation.width - large_bounds->width);
    large_bounds->y = CLAMP(y, 0, allocation.height - large_bounds->height);
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
    magnify_bounds (widget, &bounds, &large_bounds, 1.5);

    cairo_save (cr);
    cairo_translate (cr, bounds.x, bounds.y);
    eek_renderer_render_key (priv->renderer, cr, key, 1.0, TRUE);
    cairo_restore (cr);

    cairo_save (cr);
    cairo_translate (cr, large_bounds.x, large_bounds.y);
    eek_renderer_render_key (priv->renderer, cr, key, 1.5, TRUE);
    cairo_restore (cr);

    cairo_destroy (cr);
}

static void
render_locked_key (GtkWidget *widget,
                   EekKey    *key)
{
    EekGtkKeyboardPrivate *priv = EEK_GTK_KEYBOARD_GET_PRIVATE(widget);
    EekBounds bounds;
    cairo_t *cr;

    cr = gdk_cairo_create (GDK_DRAWABLE (gtk_widget_get_window (widget)));

    eek_renderer_get_key_bounds (priv->renderer, key, &bounds, TRUE);
    cairo_translate (cr, bounds.x, bounds.y);
    eek_renderer_render_key (priv->renderer, cr, key, 1.0, TRUE);

    cairo_destroy (cr);
}

static void
render_released_key (GtkWidget *widget,
                     EekKey    *key)
{
    EekGtkKeyboardPrivate *priv = EEK_GTK_KEYBOARD_GET_PRIVATE(widget);
    EekBounds bounds, large_bounds;
    cairo_t *cr;

    cr = gdk_cairo_create (GDK_DRAWABLE (gtk_widget_get_window (widget)));

    eek_renderer_get_key_bounds (priv->renderer, key, &bounds, TRUE);
    magnify_bounds (widget, &bounds, &large_bounds, 2.0);
    cairo_rectangle (cr,
                     large_bounds.x,
                     large_bounds.y,
                     large_bounds.width,
                     large_bounds.height);
    cairo_rectangle (cr,
                     bounds.x,
                     bounds.y,
                     bounds.width,
                     bounds.height);
    cairo_clip (cr);

    eek_renderer_render_keyboard (priv->renderer, cr);
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

#if HAVE_LIBCANBERRA
    ca_gtk_play_for_widget (widget, 0,
                            CA_PROP_EVENT_ID, "button-pressed",
                            CA_PROP_EVENT_DESCRIPTION, "virtual key pressed",
                            CA_PROP_APPLICATION_ID, "org.fedorahosted.Eekboard",
                            NULL);
#endif
}

static void
on_key_released (EekKeyboard *keyboard,
                 EekKey      *key,
                 gpointer     user_data)
{
    GtkWidget *widget = user_data;
    EekGtkKeyboardPrivate *priv = EEK_GTK_KEYBOARD_GET_PRIVATE(widget);

    /* renderer may have not been set yet if the widget is a popup */
    if (!priv->renderer)
        return;

    render_released_key (widget, key);

#if HAVE_LIBCANBERRA
    ca_gtk_play_for_widget (widget, 0,
                            CA_PROP_EVENT_ID, "button-released",
                            CA_PROP_EVENT_DESCRIPTION, "virtual key pressed",
                            CA_PROP_APPLICATION_ID, "org.fedorahosted.Eekboard",
                            NULL);
#endif
}

static void
on_key_cancelled (EekKeyboard *keyboard,
                 EekKey      *key,
                 gpointer     user_data)
{
    GtkWidget *widget = user_data;
    EekGtkKeyboardPrivate *priv = EEK_GTK_KEYBOARD_GET_PRIVATE(widget);

    /* renderer may have not been set yet if the widget is a popup */
    if (!priv->renderer)
        return;

    render_released_key (widget, key);
}

static void
on_key_locked (EekKeyboard *keyboard,
               EekKey      *key,
               gpointer     user_data)
{
    GtkWidget *widget = user_data;
    EekGtkKeyboardPrivate *priv = EEK_GTK_KEYBOARD_GET_PRIVATE(widget);

    /* renderer may have not been set yet if the widget is a popup */
    if (!priv->renderer)
        return;

    render_locked_key (widget, key);
}

static void
on_key_unlocked (EekKeyboard *keyboard,
                 EekKey      *key,
                 gpointer     user_data)
{
    GtkWidget *widget = user_data;
    EekGtkKeyboardPrivate *priv = EEK_GTK_KEYBOARD_GET_PRIVATE(widget);

    /* renderer may have not been set yet if the widget is a popup */
    if (!priv->renderer)
        return;

    render_released_key (widget, key);
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

void
eek_gtk_keyboard_set_theme (EekGtkKeyboard *keyboard,
                            EekTheme       *theme)
{
    EekGtkKeyboardPrivate *priv;

    g_return_if_fail (EEK_IS_GTK_KEYBOARD(keyboard));
    g_return_if_fail (EEK_IS_THEME(theme));

    priv = EEK_GTK_KEYBOARD_GET_PRIVATE(keyboard);
    priv->theme = g_object_ref (theme);
}
