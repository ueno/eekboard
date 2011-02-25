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
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif  /* HAVE_CONFIG_H */

#include <gtk/gtk.h>
#include <glib/gi18n.h>

#include "eek/eek.h"

#if HAVE_CLUTTER_GTK
#include <clutter-gtk/clutter-gtk.h>
#include "eek/eek-clutter.h"
#else  /* HAVE_CLUTTER_GTK */
#include "eek/eek-gtk.h"
#endif  /* !HAVE_CLUTTER_GTK */

#include "server-context.h"

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
    "  <interface name='com.redhat.Eekboard.Context'>"
    "    <method name='SetKeyboard'>"
    "      <arg type='v' name='keyboard'/>"
    "    </method>"
    "    <method name='ShowKeyboard'/>"
    "    <method name='HideKeyboard'/>"
    "    <method name='SetGroup'>"
    "      <arg type='i' name='group'/>"
    "    </method>"
    "    <method name='PressKey'>"
    "      <arg type='u' name='keycode'/>"
    "    </method>"
    "    <method name='ReleaseKey'>"
    "      <arg type='u' name='keycode'/>"
    "    </method>"
    /* signals */
    "    <signal name='Enabled'/>"
    "    <signal name='Disabled'/>"
    "    <signal name='KeyPressed'>"
    "      <arg type='u' name='keycode'/>"
    "    </signal>"
    "    <signal name='KeyReleased'>"
    "      <arg type='u' name='keycode'/>"
    "    </signal>"
    "    <signal name='KeyboardVisibilityChanged'>"
    "      <arg type='b' name='visible'/>"
    "    </signal>"
    "  </interface>"
    "</node>";

typedef struct _ServerContextClass ServerContextClass;

struct _ServerContext {
    GObject parent;
    GDBusConnection *connection;
    GDBusNodeInfo *introspection_data;
    guint registration_id;
    char *object_path;
    char *client_connection;

    gboolean enabled;
    gboolean last_keyboard_visible;

    GtkWidget *window;
    GtkWidget *widget;
    EekKeyboard *keyboard;

    gulong key_pressed_handler;
    gulong key_released_handler;
    gulong notify_visible_handler;
};

struct _ServerContextClass {
    GObjectClass parent_class;
};

G_DEFINE_TYPE (ServerContext, server_context, G_TYPE_OBJECT);

static void disconnect_keyboard_signals (ServerContext         *context);
static void handle_method_call          (GDBusConnection       *connection,
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
    ServerContext *context = user_data;

    g_assert (widget == context->window);
    context->window = NULL;
    context->widget = NULL;
}

static void
on_notify_visible (GObject *object, GParamSpec *spec, gpointer user_data)
{
    ServerContext *context = user_data;
    gboolean visible;
    GError *error;

    g_object_get (object, "visible", &visible, NULL);

    if (context->connection && context->enabled) {
        error = NULL;
        g_dbus_connection_emit_signal (context->connection,
                                       NULL,
                                       context->object_path,
                                       SERVER_CONTEXT_INTERFACE,
                                       "KeyboardVisibilityChanged",
                                       g_variant_new ("(b)", visible),
                                       &error);
        g_assert_no_error (error);
    }
}

static void
update_widget (ServerContext *context)
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

    if (context->widget)
        gtk_widget_destroy (context->widget);

    eek_element_get_bounds (EEK_ELEMENT(context->keyboard), &bounds);
#if HAVE_CLUTTER_GTK
    context->widget = gtk_clutter_embed_new ();
    stage = gtk_clutter_embed_get_stage (GTK_CLUTTER_EMBED(context->widget));
    actor = eek_clutter_keyboard_new (context->keyboard);
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
    context->widget = eek_gtk_keyboard_new (context->keyboard);
#endif
    gtk_widget_set_size_request (context->widget, bounds.width, bounds.height);

    if (!context->window) {
        context->window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
        g_signal_connect (context->window, "destroy",
                          G_CALLBACK(on_destroy), context);
        context->notify_visible_handler =
            g_signal_connect (context->window, "notify::visible",
                              G_CALLBACK(on_notify_visible), context);

        gtk_widget_set_can_focus (context->window, FALSE);
        g_object_set (G_OBJECT(context->window), "accept_focus", FALSE, NULL);
        gtk_window_set_title (GTK_WINDOW(context->window), _("Keyboard"));
        gtk_window_set_icon_name (GTK_WINDOW(context->window), "eekboard");
        gtk_window_set_keep_above (GTK_WINDOW(context->window), TRUE);

        screen = gdk_screen_get_default ();
        root = gtk_widget_get_root_window (context->window);
        monitor = gdk_screen_get_monitor_at_window (screen, root);
        gdk_screen_get_monitor_geometry (screen, monitor, &rect);
        gtk_window_move (GTK_WINDOW(context->window),
                         MAX(rect.width - 20 - bounds.width, 0),
                         MAX(rect.height - 40 - bounds.height, 0));
    }
    gtk_container_add (GTK_CONTAINER(context->window), context->widget);
}

static void
server_context_set_property (GObject      *object,
                             guint         prop_id,
                             const GValue *value,
                             GParamSpec   *pspec)
{
    ServerContext *context = SERVER_CONTEXT(object);
    GDBusConnection *connection;

    switch (prop_id) {
    case PROP_OBJECT_PATH:
        if (context->object_path)
            g_free (context->object_path);
        context->object_path = g_strdup (g_value_get_string (value));
        break;
    case PROP_CONNECTION:
        connection = g_value_get_object (value);
        if (context->connection)
            g_object_unref (context->connection);
        context->connection = g_object_ref (connection);
        break;
    default:
        g_object_set_property (object,
                               g_param_spec_get_name (pspec),
                               value);
        break;
    }
}

static void
server_context_dispose (GObject *object)
{
    ServerContext *context = SERVER_CONTEXT(object);

    if (context->keyboard) {
        disconnect_keyboard_signals (context);
        g_object_unref (context->keyboard);
        context->keyboard = NULL;
    }

    if (context->window) {
        gtk_widget_destroy (context->window);
        context->window = NULL;
    }

    if (context->connection) {
        if (context->registration_id > 0) {
            g_dbus_connection_unregister_object (context->connection,
                                                 context->registration_id);
            context->registration_id = 0;
        }

        g_object_unref (context->connection);
        context->connection = NULL;
    }

    if (context->introspection_data) {
        g_dbus_node_info_unref (context->introspection_data);
        context->introspection_data = NULL;
    }

    G_OBJECT_CLASS (server_context_parent_class)->dispose (object);
}

static void
server_context_finalize (GObject *object)
{
    ServerContext *context = SERVER_CONTEXT(object);

    g_free (context->client_connection);

    G_OBJECT_CLASS (server_context_parent_class)->finalize (object);
}

static void
server_context_constructed (GObject *object)
{
    ServerContext *context = SERVER_CONTEXT (object);
    if (context->connection && context->object_path) {
        GError *error = NULL;

        context->registration_id = g_dbus_connection_register_object
            (context->connection,
             context->object_path,
             context->introspection_data->interfaces[0],
             &interface_vtable,
             context,
             NULL,
             &error);
    }
}

static void
server_context_class_init (ServerContextClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    GParamSpec *pspec;

    gobject_class->constructed = server_context_constructed;
    gobject_class->set_property = server_context_set_property;
    gobject_class->dispose = server_context_dispose;
    gobject_class->finalize = server_context_finalize;

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
server_context_init (ServerContext *context)
{
    GError *error;

    context->connection = NULL;
    error = NULL;
    context->introspection_data =
        g_dbus_node_info_new_for_xml (introspection_xml, &error);
    g_assert (context->introspection_data != NULL);
    context->registration_id = 0;
    context->object_path = NULL;

    context->enabled = FALSE;
    context->last_keyboard_visible = FALSE;

    context->keyboard = NULL;
    context->widget = NULL;
    context->window = NULL;
    context->key_pressed_handler = 0;
    context->key_released_handler = 0;
}

static void
on_key_pressed (EekKeyboard *keyboard,
                EekKey      *key,
                gpointer     user_data)
{
    ServerContext *context = user_data;

    if (context->connection && context->enabled) {
        guint keycode = eek_key_get_keycode (key);
        GError *error;

        error = NULL;
        g_dbus_connection_emit_signal (context->connection,
                                       NULL,
                                       context->object_path,
                                       SERVER_CONTEXT_INTERFACE,
                                       "KeyPressed",
                                       g_variant_new ("(u)", keycode),
                                       &error);
        g_assert_no_error (error);
    }
}

static void
on_key_released (EekKeyboard *keyboard,
                 EekKey      *key,
                 gpointer     user_data)
{
    ServerContext *context = user_data;

    if (context->connection && context->enabled) {
        guint keycode = eek_key_get_keycode (key);
        GError *error;

        error = NULL;
        g_dbus_connection_emit_signal (context->connection,
                                       NULL,
                                       context->object_path,
                                       SERVER_CONTEXT_INTERFACE,
                                       "KeyReleased",
                                       g_variant_new ("(u)", keycode),
                                       &error);
        g_assert_no_error (error);
    }
}

static void
disconnect_keyboard_signals (ServerContext *context)
{
    if (g_signal_handler_is_connected (context->keyboard,
                                       context->key_pressed_handler))
        g_signal_handler_disconnect (context->keyboard,
                                     context->key_pressed_handler);
    if (g_signal_handler_is_connected (context->keyboard,
                                       context->key_released_handler))
        g_signal_handler_disconnect (context->keyboard,
                                     context->key_released_handler);
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
    ServerContext *context = user_data;

    if (g_strcmp0 (method_name, "SetKeyboard") == 0) {
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
        
        context->keyboard = EEK_KEYBOARD(serializable);
        disconnect_keyboard_signals (context);
        context->key_pressed_handler =
            g_signal_connect (context->keyboard, "key-pressed",
                              G_CALLBACK(on_key_pressed),
                              context);
        context->key_released_handler =
            g_signal_connect (context->keyboard, "key-released",
                              G_CALLBACK(on_key_released),
                              context);
        eek_keyboard_set_modifier_behavior (context->keyboard,
                                            EEK_MODIFIER_BEHAVIOR_LATCH);
        
        if (context->window) {
            gboolean was_visible = gtk_widget_get_visible (context->window);
            /* avoid to send KeyboardVisibilityChanged */
            g_signal_handler_block (context->window,
                                    context->notify_visible_handler);
            update_widget (context);
            if (was_visible)
                gtk_widget_show_all (context->window);
            g_signal_handler_unblock (context->window,
                                      context->notify_visible_handler);
        }

        g_dbus_method_invocation_return_value (invocation, NULL);
        return;
    }

    if (g_strcmp0 (method_name, "SetGroup") == 0) {
        gint group;

        if (!context->keyboard) {
            g_dbus_method_invocation_return_error (invocation,
                                                   G_IO_ERROR,
                                                   G_IO_ERROR_FAILED_HANDLED,
                                                   "keyboard is not set");
            return;
        }

        g_variant_get (parameters, "(i)", &group);
        eek_keyboard_set_group (context->keyboard, group);

        if (context->window) {
            gboolean was_visible = gtk_widget_get_visible (context->window);

            /* avoid to send KeyboardVisibilityChanged */
            g_signal_handler_block (context->window,
                                    context->notify_visible_handler);
            update_widget (context);
            if (was_visible)
                gtk_widget_show_all (context->window);
            g_signal_handler_unblock (context->window,
                                      context->notify_visible_handler);
        }

        g_dbus_method_invocation_return_value (invocation, NULL);
        return;
    }

    if (g_strcmp0 (method_name, "ShowKeyboard") == 0) {
        if (!context->keyboard) {
            g_dbus_method_invocation_return_error (invocation,
                                                   G_IO_ERROR,
                                                   G_IO_ERROR_FAILED_HANDLED,
                                                   "keyboard is not set");
            return;
        }

        if (!context->window)
            update_widget (context);
        g_assert (context->window);
        gtk_widget_show_all (context->window);
        g_dbus_method_invocation_return_value (invocation, NULL);
        return;
    }

    if (g_strcmp0 (method_name, "HideKeyboard") == 0) {
        if (context->window)
            gtk_widget_hide (context->window);
        g_dbus_method_invocation_return_value (invocation, NULL);
        return;
    }

    if (g_strcmp0 (method_name, "PressKey") == 0 ||
        g_strcmp0 (method_name, "ReleaseKey") == 0) {
        EekKey *key;
        guint keycode;

        if (!context->keyboard) {
            g_dbus_method_invocation_return_error (invocation,
                                                   G_IO_ERROR,
                                                   G_IO_ERROR_FAILED_HANDLED,
                                                   "keyboard is not set");
            return;
        }

        g_variant_get (parameters, "(u)", &keycode);
        key = eek_keyboard_find_key_by_keycode (context->keyboard, keycode);

        if (!key) {
            g_dbus_method_invocation_return_error (invocation,
                                                   G_IO_ERROR,
                                                   G_IO_ERROR_FAILED_HANDLED,
                                                   "key for %u is not found",
                                                   keycode);
            return;
        }

        if (g_strcmp0 (method_name, "PressKey") == 0) {
            g_signal_handler_block (context->keyboard,
                                    context->key_pressed_handler);
            g_signal_emit_by_name (key, "pressed");
            g_signal_handler_unblock (context->keyboard,
                                      context->key_pressed_handler);
        } else {
            g_signal_handler_block (context->keyboard,
                                    context->key_released_handler);
            g_signal_emit_by_name (key, "released");
            g_signal_handler_unblock (context->keyboard,
                                      context->key_released_handler);
        }

        g_dbus_method_invocation_return_value (invocation, NULL);
        return;
    }

    g_return_if_reached ();
}

ServerContext *
server_context_new (const gchar     *object_path,
                    GDBusConnection *connection)
{
    return g_object_new (SERVER_TYPE_CONTEXT,
                         "object-path", object_path,
                         "connection", connection,
                         NULL);
}

void
server_context_set_enabled (ServerContext *context, gboolean enabled)
{
    GError *error;

    g_return_if_fail (SERVER_IS_CONTEXT(context));
    g_return_if_fail (context->connection);

    if (context->enabled == enabled)
        return;

    context->enabled = enabled;
    if (enabled) {
        error = NULL;
        g_dbus_connection_emit_signal (context->connection,
                                       NULL,
                                       context->object_path,
                                       SERVER_CONTEXT_INTERFACE,
                                       "Enabled",
                                       NULL,
                                       &error);
        g_assert_no_error (error);
        if (context->last_keyboard_visible && context->window)
            gtk_widget_show_all (context->window);
    } else {
        error = NULL;
        g_dbus_connection_emit_signal (context->connection,
                                       NULL,
                                       context->object_path,
                                       SERVER_CONTEXT_INTERFACE,
                                       "Disabled",
                                       NULL,
                                       &error);
        g_assert_no_error (error);
        if (context->window) {
            context->last_keyboard_visible =
                gtk_widget_get_visible (context->window);
            gtk_widget_hide (context->window);
        }
    }
}

void
server_context_set_client_connection (ServerContext *context,
                                      const gchar   *client_connection)
{
    context->client_connection = g_strdup (client_connection);
}

const gchar *
server_context_get_client_connection (ServerContext *context)
{
    return context->client_connection;
}
