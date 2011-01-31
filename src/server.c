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
#include "eek/eek-gtk.h"
#include "server.h"

#define CSW 640
#define CSH 480

enum {
    PROP_0,
    PROP_CONNECTION,
    PROP_LAST
};

static const gchar introspection_xml[] =
    "<node>"
    "  <interface name='com.redhat.eekboard.Keyboard'>"
    "    <method name='SetKeyboard'>"
    "      <arg type='v' name='keyboard'/>"
    "    </method>"
    "    <method name='SetGroup'>"
    "      <arg type='i' name='group'/>"
    "    </method>"
    "    <method name='Show'/>"
    "    <method name='Hide'/>"
    "    <method name='PressKey'>"
    "      <arg type='s' name='key_id' direction='in'/>"
    "    </method>"
    "    <method name='ReleaseKey'>"
    "      <arg type='s' name='key_id' direction='in'/>"
    "    </method>"
    "    <signal name='KeyPressed'>"
    "      <arg type='u' name='keycode'/>"
    "    </signal>"
    "    <signal name='KeyReleased'>"
    "      <arg type='u' name='keycode'/>"
    "    </signal>"
    "  </interface>"
    "</node>";

typedef struct _EekboardServerClass EekboardServerClass;

struct _EekboardServer {
    GObject parent;
    GDBusConnection *connection;
    guint owner_id;
    GDBusNodeInfo *introspection_data;

    GtkWidget *window;
    GtkWidget *widget;
    EekKeyboard *keyboard;
};

struct _EekboardServerClass {
    GObjectClass parent_class;
};

G_DEFINE_TYPE (EekboardServer, eekboard_server, G_TYPE_OBJECT);

static void
update_widget (EekboardServer *server)
{
    GdkScreen *screen;
    GdkWindow *root;
    gint monitor;
    GdkRectangle rect;
    EekBounds bounds;

    if (server->widget)
        gtk_widget_destroy (server->widget);

    if (server->window)
        gtk_widget_destroy (server->window);

    server->widget = eek_gtk_keyboard_new (server->keyboard);

    eek_element_get_bounds (EEK_ELEMENT(server->keyboard), &bounds);
    gtk_widget_set_size_request (server->widget, bounds.width, bounds.height);

    server->window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_container_add (GTK_CONTAINER(server->window), server->widget);

    gtk_widget_set_can_focus (server->window, FALSE);
    g_object_set (G_OBJECT(server->window), "accept_focus", FALSE, NULL);
    gtk_window_set_title (GTK_WINDOW(server->window), "Keyboard");

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
        g_object_unref (server->connection);
        server->connection = NULL;
    }
    G_OBJECT_CLASS (eekboard_server_parent_class)->dispose (object);
}

static void
eekboard_server_class_init (EekboardServerClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    GParamSpec *pspec;

    gobject_class->set_property = eekboard_server_set_property;
    gobject_class->dispose = eekboard_server_dispose;

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
    server->owner_id = 0;
    server->keyboard = NULL;
    server->widget = NULL;
    server->window = NULL;
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
                                   "com.redhat.eekboard.Keyboard",
                                   "/com/redhat/eekboard/Keyboard",
                                   "com.redhat.eekboard.Keyboard",
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
                                   "com.redhat.eekboard.Keyboard",
                                   "/com/redhat/eekboard/Keyboard",
                                   "com.redhat.eekboard.Keyboard",
                                   "KeyReleased",
                                   g_variant_new ("(u)",
                                                  eek_key_get_keycode (key)),
                                   &error);
    g_assert_no_error (error);
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

    g_debug ("%s", method_name);
    if (g_strcmp0 (method_name, "SetKeyboard") == 0) {
        GVariant *variant;
        gchar *data;
        GInputStream *input;
        EekLayout *layout;

        g_variant_get (parameters, "(v)", &variant);
        g_variant_get (variant, "(&s)", &data);
        input = g_memory_input_stream_new_from_data (data, -1, NULL);
        g_variant_unref (variant);

        layout = eek_xml_layout_new (input);
        if (!layout) {
            g_dbus_method_invocation_return_error (invocation,
                                                   G_IO_ERROR,
                                                   G_IO_ERROR_FAILED_HANDLED,
                                                   "can't create layout");
            return;
        }

        server->keyboard = eek_keyboard_new (layout, CSW, CSH);
        g_signal_connect (server->keyboard, "key-pressed",
                          G_CALLBACK(on_key_pressed),
                          server);
        g_signal_connect (server->keyboard, "key-released",
                          G_CALLBACK(on_key_released),
                          server);
        eek_keyboard_set_modifier_behavior (server->keyboard,
                                            EEK_MODIFIER_BEHAVIOR_LATCH);
        
        update_widget (server);
        g_dbus_method_invocation_return_value (invocation, NULL);
    } else if (g_strcmp0 (method_name, "SetGroup") == 0) {
        gint group;

        if (!server->keyboard)
            g_dbus_method_invocation_return_error (invocation,
                                                   G_IO_ERROR,
                                                   G_IO_ERROR_FAILED_HANDLED,
                                                   "keyboard is not set");
        g_variant_get (parameters, "(i)", &group);
        eek_keyboard_set_group (server->keyboard, group);
        g_dbus_method_invocation_return_value (invocation, NULL);
    } else if (g_strcmp0 (method_name, "Show") == 0) {
        if (!server->keyboard)
            g_dbus_method_invocation_return_error (invocation,
                                                   G_IO_ERROR,
                                                   G_IO_ERROR_FAILED_HANDLED,
                                                   "keyboard is not set");
        if (server->window)
            gtk_widget_show_all (server->window);
        g_dbus_method_invocation_return_value (invocation, NULL);
    } else if (g_strcmp0 (method_name, "Hide") == 0) {
        if (server->window)
            gtk_widget_hide (server->window);
        g_dbus_method_invocation_return_value (invocation, NULL);
    }
}

static const GDBusInterfaceVTable interface_vtable =
{
  handle_method_call,
  NULL,
  NULL
};

static void
on_name_acquired (GDBusConnection *connection,
                  const gchar     *name,
                  gpointer         user_data)
{
    //g_debug ("name acquired %s", name);
}

static void
on_name_lost (GDBusConnection *connection,
              const gchar     *name,
              gpointer         user_data)
{
    //g_debug ("name lost %s", name);
}

EekboardServer *
eekboard_server_new (GDBusConnection *connection)
{
    return g_object_new (EEKBOARD_TYPE_SERVER, "connection", connection, NULL);
}

gboolean
eekboard_server_start (EekboardServer *server)
{
    guint registration_id;
    GError *error;

    error = NULL;
    registration_id = g_dbus_connection_register_object
        (server->connection,
         "/com/redhat/eekboard/Keyboard",
         server->introspection_data->interfaces[0],
         &interface_vtable,
         server,
         NULL,
         &error);
    if (error)
        g_printerr ("%s\n", error->message);
    g_assert (registration_id > 0);

    server->owner_id =
        g_bus_own_name_on_connection (server->connection,
                                      "com.redhat.eekboard.Keyboard",
                                      G_BUS_NAME_OWNER_FLAGS_NONE,
                                      on_name_acquired,
                                      on_name_lost,
                                      NULL,
                                      NULL);
    return server->owner_id > 0;
}

void
eekboard_server_stop (EekboardServer *server)
{
    if (server->owner_id > 0)
        g_bus_unown_name (server->owner_id);
    if (server->introspection_data)
        g_dbus_node_info_unref (server->introspection_data);
}
