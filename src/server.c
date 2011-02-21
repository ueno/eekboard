/* 
 * Copyright (C) 2010-2011 Daiki Ueno <ueno@unixuser.org>
 * Copyright (C) 2010-2011 Red Hat, Inc.
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <gtk/gtk.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif  /* HAVE_CONFIG_H */

#include "eek/eek.h"

#if HAVE_CLUTTER_GTK
#include <clutter-gtk/clutter-gtk.h>
#include "eek/eek-clutter.h"
#else  /* HAVE_CLUTTER_GTK */
#include "eek/eek-gtk.h"
#endif  /* !HAVE_CLUTTER_GTK */

#include "server.h"

#define CSW 640
#define CSH 480

enum {
    PROP_0,
    PROP_OBJECT_PATH,
    PROP_CONNECTION,
    PROP_LAST
};

static const gchar introspection_xml[] =
    "<node>"
    "  <interface name='com.redhat.Eekboard.Keyboard'>"
    "    <method name='SetDescription'>"
    "      <arg type='v' name='description'/>"
    "    </method>"
    "    <method name='SetGroup'>"
    "      <arg type='i' name='group'/>"
    "    </method>"
    "    <method name='Show'/>"
    "    <method name='Hide'/>"
    "    <method name='PressKey'>"
    "      <arg type='u' name='keycode'/>"
    "    </method>"
    "    <method name='ReleaseKey'>"
    "      <arg type='u' name='keycode'/>"
    "    </method>"
    "    <signal name='KeyPressed'>"
    "      <arg type='u' name='keycode'/>"
    "    </signal>"
    "    <signal name='KeyReleased'>"
    "      <arg type='u' name='keycode'/>"
    "    </signal>"
    "    <signal name='VisibilityChanged'>"
    "      <arg type='b' name='visible'/>"
    "    </signal>"
    "  </interface>"
    "</node>";

typedef struct _EekboardServerClass EekboardServerClass;

struct _EekboardServer {
    GObject parent;
    GDBusConnection *connection;
    GDBusNodeInfo *introspection_data;
    guint registration_id;
    char *object_path;

    GtkWidget *window;
    GtkWidget *widget;
    EekKeyboard *keyboard;

    gulong key_pressed_handler;
    gulong key_released_handler;
};

struct _EekboardServerClass {
    GObjectClass parent_class;
};

G_DEFINE_TYPE (EekboardServer, eekboard_server, G_TYPE_OBJECT);

static void handle_method_call (GDBusConnection       *connection,
                                const gchar           *sender,
                                const gchar           *object_path,
                                const gchar           *interface_name,
                                const gchar           *method_name,
                                GVariant              *parameters,
                                GDBusMethodInvocation *invocation,
                                gpointer               user_data);

static const GDBusInterfaceVTable interface_vtable =
{
  handle_method_call,
  NULL,
  NULL
};

#if HAVE_CLUTTER_GTK
static void
on_allocation_changed (ClutterActor          *stage,
                       ClutterActorBox       *box,
                       ClutterAllocationFlags flags,
                       gpointer               user_data)
{
    ClutterActor *actor = user_data;
    clutter_actor_set_size (actor,
                            box->x2 - box->x1,
                            box->y2 - box->y1);
}
#endif

static void
on_destroy (GtkWidget *widget, gpointer user_data)
{
    EekboardServer *server = user_data;

    g_assert (widget == server->window);
    server->window = NULL;
    server->widget = NULL;
}

static void
on_notify_visible (GObject *object, GParamSpec *spec, gpointer user_data)
{
    EekboardServer *server = user_data;
    gboolean visible;
    GError *error;

    g_object_get (object, "visible", &visible, NULL);

    error = NULL;
    g_dbus_connection_emit_signal (server->connection,
                                   "com.redhat.Eekboard.Keyboard",
                                   "/com/redhat/Eekboard/Keyboard",
                                   "com.redhat.Eekboard.Keyboard",
                                   "VisibilityChanged",
                                   g_variant_new ("(b)", visible),
                                   &error);
    g_assert_no_error (error);
}

static void
update_widget (EekboardServer *server)
{
    GdkScreen *screen;
    GdkWindow *root;
    gint monitor;
    GdkRectangle rect;
    EekBounds bounds;
#if HAVE_CLUTTER_GTK
    ClutterActor *stage, *actor;
    ClutterColor stage_color = { 0xff, 0xff, 0xff, 0xff };
#endif

    if (server->widget)
        gtk_widget_destroy (server->widget);

    if (server->window)
        gtk_widget_destroy (server->window);

    eek_element_get_bounds (EEK_ELEMENT(server->keyboard), &bounds);
#if HAVE_CLUTTER_GTK
    server->widget = gtk_clutter_embed_new ();
    stage = gtk_clutter_embed_get_stage (GTK_CLUTTER_EMBED(server->widget));
    actor = eek_clutter_keyboard_new (server->keyboard);
    clutter_container_add_actor (CLUTTER_CONTAINER(stage), actor);

    clutter_stage_set_color (CLUTTER_STAGE(stage), &stage_color);
    clutter_stage_set_user_resizable (CLUTTER_STAGE(stage), TRUE);
    clutter_stage_set_minimum_size (CLUTTER_STAGE(stage),
                                    bounds.width / 3,
                                    bounds.height / 3);
    g_signal_connect (stage,
                      "allocation-changed",
                      G_CALLBACK(on_allocation_changed),
                      actor);
#else
    server->widget = eek_gtk_keyboard_new (server->keyboard);
#endif
    gtk_widget_set_size_request (server->widget, bounds.width, bounds.height);

    server->window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    g_signal_connect (server->window, "destroy",
                      G_CALLBACK(on_destroy), server);
    g_signal_connect (server->window, "notify::visible",
                      G_CALLBACK(on_notify_visible), server);
    gtk_container_add (GTK_CONTAINER(server->window), server->widget);

    gtk_widget_set_can_focus (server->window, FALSE);
    g_object_set (G_OBJECT(server->window), "accept_focus", FALSE, NULL);
    gtk_window_set_title (GTK_WINDOW(server->window), "Keyboard");
    gtk_window_set_keep_above (GTK_WINDOW(server->window), TRUE);

    screen = gdk_screen_get_default ();
    root = gtk_widget_get_root_window (server->window);
    monitor = gdk_screen_get_monitor_at_window (screen, root);
    gdk_screen_get_monitor_geometry (screen, monitor, &rect);
    gtk_window_move (GTK_WINDOW(server->window),
                     MAX(rect.width - 20 - bounds.width, 0),
                     MAX(rect.height - 40 - bounds.height, 0));
}

static void
eekboard_server_set_property (GObject      *object,
                              guint         prop_id,
                              const GValue *value,
                              GParamSpec   *pspec)
{
    EekboardServer *server = EEKBOARD_SERVER(object);
    GDBusConnection *connection;

    switch (prop_id) {
    case PROP_OBJECT_PATH:
        if (server->object_path)
            g_free (server->object_path);
        server->object_path = g_strdup (g_value_get_string (value));
        break;
    case PROP_CONNECTION:
        connection = g_value_get_object (value);
        if (server->connection)
            g_object_unref (server->connection);
        server->connection = g_object_ref (connection);
        break;
    default:
        g_object_set_property (object,
                               g_param_spec_get_name (pspec),
                               value);
        break;
    }
}

static void
eekboard_server_dispose (GObject *object)
{
    EekboardServer *server = EEKBOARD_SERVER(object);

    if (server->connection) {
        if (server->registration_id > 0) {
            g_dbus_connection_unregister_object (server->connection,
                                                 server->registration_id);
            server->registration_id = 0;
        }

        g_object_unref (server->connection);
        server->connection = NULL;
    }

    if (server->introspection_data) {
        g_dbus_node_info_unref (server->introspection_data);
        server->introspection_data = NULL;
    }

    G_OBJECT_CLASS (eekboard_server_parent_class)->dispose (object);
}

static void
eekboard_server_constructed (GObject *object)
{
    EekboardServer *server = EEKBOARD_SERVER (object);
    if (server->connection && server->object_path) {
        GError *error = NULL;

        server->registration_id = g_dbus_connection_register_object
            (server->connection,
             server->object_path,
             server->introspection_data->interfaces[0],
             &interface_vtable,
             server,
             NULL,
             &error);
    }
}

static void
eekboard_server_class_init (EekboardServerClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    GParamSpec *pspec;

    gobject_class->constructed = eekboard_server_constructed;
    gobject_class->set_property = eekboard_server_set_property;
    gobject_class->dispose = eekboard_server_dispose;

    pspec = g_param_spec_string ("object-path",
                                 "Object-path",
                                 "Object-path",
                                 NULL,
                                 G_PARAM_CONSTRUCT_ONLY | G_PARAM_WRITABLE);
    g_object_class_install_property (gobject_class,
                                     PROP_OBJECT_PATH,
                                     pspec);

    pspec = g_param_spec_object ("connection",
                                 "Connection",
                                 "Connection",
                                 G_TYPE_DBUS_CONNECTION,
                                 G_PARAM_CONSTRUCT_ONLY | G_PARAM_WRITABLE);
    g_object_class_install_property (gobject_class,
                                     PROP_CONNECTION,
                                     pspec);
}

static void
eekboard_server_init (EekboardServer *server)
{
    GError *error;

    error = NULL;
    server->introspection_data =
        g_dbus_node_info_new_for_xml (introspection_xml, &error);
    g_assert (server->introspection_data != NULL);
    server->registration_id = 0;
    server->object_path = NULL;
    server->keyboard = NULL;
    server->widget = NULL;
    server->window = NULL;
    server->key_pressed_handler = 0;
    server->key_released_handler = 0;
}

static void
on_key_pressed (EekKeyboard *keyboard,
                EekKey      *key,
                gpointer     user_data)
{
    EekboardServer *server = user_data;
    GError *error;

    error = NULL;
    g_dbus_connection_emit_signal (server->connection,
                                   "com.redhat.Eekboard.Keyboard",
                                   "/com/redhat/Eekboard/Keyboard",
                                   "com.redhat.Eekboard.Keyboard",
                                   "KeyPressed",
                                   g_variant_new ("(u)",
                                                  eek_key_get_keycode (key)),
                                   &error);
    g_assert_no_error (error);
}

static void
on_key_released (EekKeyboard *keyboard,
                EekKey      *key,
                gpointer     user_data)
{
    EekboardServer *server = user_data;
    GError *error;

    error = NULL;
    g_dbus_connection_emit_signal (server->connection,
                                   "com.redhat.Eekboard.Keyboard",
                                   "/com/redhat/Eekboard/Keyboard",
                                   "com.redhat.Eekboard.Keyboard",
                                   "KeyReleased",
                                   g_variant_new ("(u)",
                                                  eek_key_get_keycode (key)),
                                   &error);
    g_assert_no_error (error);
}

static void
disconnect_keyboard_signals (EekboardServer *server)
{
    if (g_signal_handler_is_connected (server->keyboard,
                                       server->key_pressed_handler))
        g_signal_handler_disconnect (server->keyboard,
                                     server->key_pressed_handler);
    if (g_signal_handler_is_connected (server->keyboard,
                                       server->key_released_handler))
        g_signal_handler_disconnect (server->keyboard,
                                     server->key_released_handler);
}

static void
handle_method_call (GDBusConnection       *connection,
                    const gchar           *sender,
                    const gchar           *object_path,
                    const gchar           *interface_name,
                    const gchar           *method_name,
                    GVariant              *parameters,
                    GDBusMethodInvocation *invocation,
                    gpointer               user_data)
{
    EekboardServer *server = user_data;

    // g_debug ("%s", method_name);
    if (g_strcmp0 (method_name, "SetDescription") == 0) {
        EekSerializable *serializable;
        GVariant *variant;

        g_variant_get (parameters, "(v)", &variant);
        serializable = eek_serializable_deserialize (variant);
        if (!EEK_IS_KEYBOARD(serializable)) {
            g_dbus_method_invocation_return_error (invocation,
                                                   G_IO_ERROR,
                                                   G_IO_ERROR_FAILED_HANDLED,
                                                   "not a keyboard");
            return;
        }
        
        server->keyboard = EEK_KEYBOARD(serializable);
        disconnect_keyboard_signals (server);
        server->key_pressed_handler =
            g_signal_connect (server->keyboard, "key-pressed",
                              G_CALLBACK(on_key_pressed),
                              server);
        server->key_released_handler =
            g_signal_connect (server->keyboard, "key-released",
                              G_CALLBACK(on_key_released),
                              server);
        eek_keyboard_set_modifier_behavior (server->keyboard,
                                            EEK_MODIFIER_BEHAVIOR_LATCH);
        
        if (server->window) {
            gboolean was_visible = gtk_widget_get_visible (server->window);
            update_widget (server);
            if (was_visible)
                gtk_widget_show_all (server->window);
        }

        g_dbus_method_invocation_return_value (invocation, NULL);
        return;
    }

    if (g_strcmp0 (method_name, "SetGroup") == 0) {
        gint group;

        if (!server->keyboard) {
            g_dbus_method_invocation_return_error (invocation,
                                                   G_IO_ERROR,
                                                   G_IO_ERROR_FAILED_HANDLED,
                                                   "keyboard is not set");
            return;
        }

        g_variant_get (parameters, "(i)", &group);
        eek_keyboard_set_group (server->keyboard, group);

        if (server->window) {
            gboolean was_visible = gtk_widget_get_visible (server->window);
            update_widget (server);
            if (was_visible)
                gtk_widget_show_all (server->window);
        }

        g_dbus_method_invocation_return_value (invocation, NULL);
        return;
    }

    if (g_strcmp0 (method_name, "Show") == 0) {
        if (!server->keyboard) {
            g_dbus_method_invocation_return_error (invocation,
                                                   G_IO_ERROR,
                                                   G_IO_ERROR_FAILED_HANDLED,
                                                   "keyboard is not set");
            return;
        }

        if (!server->window)
            update_widget (server);
        g_assert (server->window);
        gtk_widget_show_all (server->window);
        g_dbus_method_invocation_return_value (invocation, NULL);
        g_object_notify (G_OBJECT(server->window), "visible");
        return;
    }

    if (g_strcmp0 (method_name, "Hide") == 0) {
        if (server->window)
            gtk_widget_hide (server->window);
        g_dbus_method_invocation_return_value (invocation, NULL);
        g_object_notify (G_OBJECT(server->window), "visible");
        return;
    }

    if (g_strcmp0 (method_name, "PressKey") == 0 ||
        g_strcmp0 (method_name, "ReleaseKey") == 0) {
        EekKey *key;
        guint keycode;

        if (!server->keyboard) {
            g_dbus_method_invocation_return_error (invocation,
                                                   G_IO_ERROR,
                                                   G_IO_ERROR_FAILED_HANDLED,
                                                   "keyboard is not set");
            return;
        }

        g_variant_get (parameters, "(u)", &keycode);
        key = eek_keyboard_find_key_by_keycode (server->keyboard, keycode);

        if (!key) {
            g_dbus_method_invocation_return_error (invocation,
                                                   G_IO_ERROR,
                                                   G_IO_ERROR_FAILED_HANDLED,
                                                   "key for %u is not found",
                                                   keycode);
            return;
        }

        if (g_strcmp0 (method_name, "PressKey") == 0) {
            g_signal_handler_block (server->keyboard,
                                    server->key_pressed_handler);
            g_signal_emit_by_name (key, "pressed");
            g_signal_handler_unblock (server->keyboard,
                                      server->key_pressed_handler);
        } else {
            g_signal_handler_block (server->keyboard,
                                    server->key_released_handler);
            g_signal_emit_by_name (key, "released");
            g_signal_handler_unblock (server->keyboard,
                                      server->key_released_handler);
        }

        g_dbus_method_invocation_return_value (invocation, NULL);
        return;
    }

    g_return_if_reached ();
}

EekboardServer *
eekboard_server_new (const gchar     *object_path,
                     GDBusConnection *connection)
{
    return g_object_new (EEKBOARD_TYPE_SERVER,
                         "object-path", object_path,
                         "connection", connection,
                         NULL);
}
