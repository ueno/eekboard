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

#include "server-server.h"
#include "server-context.h"

#define I_(string) g_intern_static_string (string)

enum {
    PROP_0,
    PROP_OBJECT_PATH,
    PROP_CONNECTION,
    PROP_LAST
};

enum {
    DESTROYED,
    LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0, };

static const gchar introspection_xml[] =
    "<node>"
    "  <interface name='org.fedorahosted.Eekboard.Server'>"
    "    <method name='CreateContext'>"
    "      <arg direction='in' type='s' name='client_name'/>"
    "      <arg direction='out' type='s' name='object_path'/>"
    "    </method>"
    "    <method name='PushContext'>"
    "      <arg direction='in' type='s' name='object_path'/>"
    "    </method>"
    "    <method name='PopContext'/>"
    "    <method name='DestroyContext'>"
    "      <arg direction='in' type='s' name='object_path'/>"
    "    </method>"
    "    <method name='Destroy'/>"
    /* signals */
    "  </interface>"
    "</node>";

typedef struct _ServerServerClass ServerServerClass;

struct _ServerServer {
    GObject parent;
    GDBusConnection *connection;
    GDBusNodeInfo *introspection_data;
    guint registration_id;
    char *object_path;

    GHashTable *context_hash;
    GSList *context_stack;
};

struct _ServerServerClass {
    GObjectClass parent_class;
};

G_DEFINE_TYPE (ServerServer, server_server, G_TYPE_OBJECT);

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

static void
server_server_set_property (GObject      *object,
                            guint         prop_id,
                            const GValue *value,
                            GParamSpec   *pspec)
{
    ServerServer *server = SERVER_SERVER(object);
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
server_server_dispose (GObject *object)
{
    ServerServer *server = SERVER_SERVER(object);
    GSList *head;

    if (server->context_hash) {
        g_hash_table_destroy (server->context_hash);
        server->context_hash = NULL;
    }

    for (head = server->context_stack; head; head = server->context_stack) {
        g_object_unref (head->data);
        server->context_stack = g_slist_next (head);
        g_slist_free1 (head);
    }

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

    G_OBJECT_CLASS (server_server_parent_class)->dispose (object);
}

static void
server_server_finalize (GObject *object)
{
    ServerServer *server = SERVER_SERVER(object);

    g_free (server->object_path);

    G_OBJECT_CLASS (server_server_parent_class)->dispose (object);
}

static void
server_server_constructed (GObject *object)
{
    ServerServer *server = SERVER_SERVER (object);
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
server_server_class_init (ServerServerClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    GParamSpec *pspec;

    gobject_class->constructed = server_server_constructed;
    gobject_class->set_property = server_server_set_property;
    gobject_class->dispose = server_server_dispose;
    gobject_class->finalize = server_server_finalize;

    signals[DESTROYED] =
        g_signal_new (I_("destroyed"),
                      G_TYPE_FROM_CLASS(gobject_class),
                      G_SIGNAL_RUN_LAST,
                      0,
                      NULL,
                      NULL,
                      g_cclosure_marshal_VOID__VOID,
                      G_TYPE_NONE,
                      0);

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
server_server_init (ServerServer *server)
{
    GError *error;

    server->connection = NULL;
    error = NULL;
    server->introspection_data =
        g_dbus_node_info_new_for_xml (introspection_xml, &error);
    g_assert (server->introspection_data != NULL);
    server->registration_id = 0;

    server->context_hash =
        g_hash_table_new_full (g_str_hash,
                               g_str_equal,
                               (GDestroyNotify)g_free,
                               (GDestroyNotify)g_object_unref);
    server->context_stack = NULL;
}

static void
remove_context_from_stack (ServerServer *server, ServerContext *context)
{
    GSList *head;

    head = g_slist_find (server->context_stack, context);
    if (head) {
        server->context_stack = g_slist_remove_link (server->context_stack,
                                                     head);
        g_object_unref (head->data);
        g_slist_free1 (head);
    }
    if (server->context_stack)
        server_context_set_enabled (server->context_stack->data, TRUE);
}

static void
server_name_vanished_callback (GDBusConnection *connection,
                               const gchar     *name,
                               gpointer         user_data)
{
    ServerServer *server = user_data;
    GSList *head;
    GHashTableIter iter;
    gpointer k, v;

    g_hash_table_iter_init (&iter, server->context_hash);
    while (g_hash_table_iter_next (&iter, &k, &v)) {
        const gchar *client_connection =
            server_context_get_client_connection (v);
        if (g_strcmp0 (client_connection, name) == 0)
            g_hash_table_iter_remove (&iter);
    }

    for (head = server->context_stack; head; ) {
        const gchar *client_connection =
            server_context_get_client_connection (head->data);
        GSList *next = g_slist_next (head);

        if (g_strcmp0 (client_connection, name) == 0) {
            server->context_stack = g_slist_remove_link (server->context_stack,
                                                         head);
            g_object_unref (head->data);
            g_slist_free1 (head);
        }

        head = next;
    }

    if (server->context_stack)
        server_context_set_enabled (server->context_stack->data, TRUE);
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
    ServerServer *server = user_data;

    if (g_strcmp0 (method_name, "CreateContext") == 0) {
        const gchar *client_name;
        gchar *object_path;
        static gint context_id = 0;
        ServerContext *context;

        g_variant_get (parameters, "(&s)", &client_name);
        object_path = g_strdup_printf (SERVER_CONTEXT_PATH, context_id++);
        context = server_context_new (object_path, server->connection);
        server_context_set_client_connection (context, sender);
        server_context_set_client_name (context, client_name);
        g_hash_table_insert (server->context_hash,
                             object_path,
                             context);

        /* the vanished callback is called when clients are disconnected */
        g_bus_watch_name_on_connection (server->connection,
                                        sender,
                                        G_BUS_NAME_WATCHER_FLAGS_NONE,
                                        NULL,
                                        server_name_vanished_callback,
                                        server,
                                        NULL);
        g_dbus_method_invocation_return_value (invocation,
                                               g_variant_new ("(s)",
                                                              object_path));
        return;
    }

    if (g_strcmp0 (method_name, "PushContext") == 0) {
        const gchar *object_path;
        ServerContext *context;

        g_variant_get (parameters, "(&s)", &object_path);
        context = g_hash_table_lookup (server->context_hash, object_path);
        if (!context) {
            g_dbus_method_invocation_return_error (invocation,
                                                   G_IO_ERROR,
                                                   G_IO_ERROR_FAILED_HANDLED,
                                                   "context not found");
            return;
        }
        if (server->context_stack)
            server_context_set_enabled (server->context_stack->data, FALSE);
        server->context_stack = g_slist_prepend (server->context_stack,
                                                 context);
        g_object_ref (context);
        server_context_set_enabled (context, TRUE);

        g_dbus_method_invocation_return_value (invocation, NULL);
        return;
    }

    if (g_strcmp0 (method_name, "PopContext") == 0) {
        if (server->context_stack) {
            ServerContext *context = server->context_stack->data;

            if (g_strcmp0 (server_context_get_client_connection (context),
                           sender) != 0) {
                g_dbus_method_invocation_return_error
                    (invocation,
                     G_IO_ERROR,
                     G_IO_ERROR_FAILED_HANDLED,
                     "the current context not owned by %s",
                     sender);
                return;
            }
                
            server_context_set_enabled (context, FALSE);
            server->context_stack = g_slist_next (server->context_stack);
            g_object_unref (context);
            if (server->context_stack)
                server_context_set_enabled (server->context_stack->data, TRUE);
        }

        g_dbus_method_invocation_return_value (invocation, NULL);
        return;
    }

    if (g_strcmp0 (method_name, "DestroyContext") == 0) {
        const gchar *object_path;
        ServerContext *context;

        g_variant_get (parameters, "(&s)", &object_path);
        context = g_hash_table_lookup (server->context_hash, object_path);
        if (!context) {
            g_dbus_method_invocation_return_error (invocation,
                                                   G_IO_ERROR,
                                                   G_IO_ERROR_FAILED_HANDLED,
                                                   "context not found");
            return;
        }
        remove_context_from_stack (server, context);
        g_hash_table_remove (server->context_hash, object_path);
        g_dbus_method_invocation_return_value (invocation, NULL);
        return;
    }

    if (g_strcmp0 (method_name, "Destroy") == 0) {
        g_signal_emit_by_name (server, "destroyed", NULL);
        g_dbus_method_invocation_return_value (invocation, NULL);
        return;
    }

    g_return_if_reached ();
}

ServerServer *
server_server_new (const gchar     *object_path,
                   GDBusConnection *connection)
{
    return g_object_new (SERVER_TYPE_SERVER,
                         "object-path", object_path,
                         "connection", connection,
                         NULL);
}
