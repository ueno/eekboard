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

/**
 * SECTION:eekboard-service
 * @short_description: base server implementation of eekboard service
 *
 * The #EekboardService class provides a base server side
 * implementation of eekboard service.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif  /* HAVE_CONFIG_H */

#include "eekboard/eekboard-service.h"

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

#define EEKBOARD_SERVICE_GET_PRIVATE(obj)                               \
    (G_TYPE_INSTANCE_GET_PRIVATE ((obj), EEKBOARD_TYPE_SERVICE, EekboardServicePrivate))

struct _EekboardServicePrivate {
    GDBusConnection *connection;
    GDBusNodeInfo *introspection_data;
    guint registration_id;
    char *object_path;

    GHashTable *context_hash;
    GSList *context_stack;
};

G_DEFINE_TYPE (EekboardService, eekboard_service, G_TYPE_OBJECT);

static const gchar introspection_xml[] =
    "<node>"
    "  <interface name='org.fedorahosted.Eekboard'>"
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
eekboard_service_set_property (GObject      *object,
                               guint         prop_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
    EekboardService *service = EEKBOARD_SERVICE(object);
    EekboardServicePrivate *priv = EEKBOARD_SERVICE_GET_PRIVATE(service);
    GDBusConnection *connection;

    switch (prop_id) {
    case PROP_OBJECT_PATH:
        if (priv->object_path)
            g_free (priv->object_path);
        priv->object_path = g_strdup (g_value_get_string (value));
        break;
    case PROP_CONNECTION:
        connection = g_value_get_object (value);
        if (priv->connection)
            g_object_unref (priv->connection);
        priv->connection = g_object_ref (connection);
        break;
    default:
        g_object_set_property (object,
                               g_param_spec_get_name (pspec),
                               value);
        break;
    }
}

static void
eekboard_service_get_property (GObject    *object,
                               guint       prop_id,
                               GValue     *value,
                               GParamSpec *pspec)
{
    EekboardService *service = EEKBOARD_SERVICE(object);
    EekboardServicePrivate *priv = EEKBOARD_SERVICE_GET_PRIVATE(service);

    switch (prop_id) {
    case PROP_OBJECT_PATH:
        g_value_set_string (value, priv->object_path);
        break;
    case PROP_CONNECTION:
        g_value_set_object (value, priv->connection);
        break;
    default:
        g_object_set_property (object,
                               g_param_spec_get_name (pspec),
                               value);
        break;
    }
}

static void
eekboard_service_dispose (GObject *object)
{
    EekboardService *service = EEKBOARD_SERVICE(object);
    EekboardServicePrivate *priv = EEKBOARD_SERVICE_GET_PRIVATE(service);
    GSList *head;

    if (priv->context_hash) {
        g_hash_table_destroy (priv->context_hash);
        priv->context_hash = NULL;
    }

    for (head = priv->context_stack; head; head = priv->context_stack) {
        g_object_unref (head->data);
        priv->context_stack = g_slist_next (head);
        g_slist_free1 (head);
    }

    if (priv->connection) {
        if (priv->registration_id > 0) {
            g_dbus_connection_unregister_object (priv->connection,
                                                 priv->registration_id);
            priv->registration_id = 0;
        }

        g_object_unref (priv->connection);
        priv->connection = NULL;
    }

    if (priv->introspection_data) {
        g_dbus_node_info_unref (priv->introspection_data);
        priv->introspection_data = NULL;
    }

    G_OBJECT_CLASS (eekboard_service_parent_class)->dispose (object);
}

static void
eekboard_service_finalize (GObject *object)
{
    EekboardServicePrivate *priv = EEKBOARD_SERVICE_GET_PRIVATE(object);

    g_free (priv->object_path);

    G_OBJECT_CLASS (eekboard_service_parent_class)->finalize (object);
}

static void
eekboard_service_constructed (GObject *object)
{
    EekboardServicePrivate *priv = EEKBOARD_SERVICE_GET_PRIVATE(object);
    if (priv->connection && priv->object_path) {
        GError *error = NULL;

        priv->registration_id = g_dbus_connection_register_object
            (priv->connection,
             priv->object_path,
             priv->introspection_data->interfaces[0],
             &interface_vtable,
             object,
             NULL,
             &error);
    }
}

static void
eekboard_service_class_init (EekboardServiceClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    GParamSpec *pspec;

    g_type_class_add_private (gobject_class,
                              sizeof (EekboardServicePrivate));

    klass->create_context = NULL;

    gobject_class->constructed = eekboard_service_constructed;
    gobject_class->set_property = eekboard_service_set_property;
    gobject_class->get_property = eekboard_service_get_property;
    gobject_class->dispose = eekboard_service_dispose;
    gobject_class->finalize = eekboard_service_finalize;

    /**
     * EekboardService::destroyed:
     * @service: an #EekboardService
     *
     * The ::destroyed signal is emitted when the service is vanished.
     */
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
                                 G_PARAM_CONSTRUCT | G_PARAM_READWRITE);
    g_object_class_install_property (gobject_class,
                                     PROP_OBJECT_PATH,
                                     pspec);

    pspec = g_param_spec_object ("connection",
                                 "Connection",
                                 "Connection",
                                 G_TYPE_DBUS_CONNECTION,
                                 G_PARAM_CONSTRUCT | G_PARAM_READWRITE);
    g_object_class_install_property (gobject_class,
                                     PROP_CONNECTION,
                                     pspec);
}

static void
eekboard_service_init (EekboardService *service)
{
    EekboardServicePrivate *priv;
    GError *error;

    priv = service->priv = EEKBOARD_SERVICE_GET_PRIVATE(service);

    error = NULL;
    priv->introspection_data =
        g_dbus_node_info_new_for_xml (introspection_xml, &error);
    g_assert (priv->introspection_data != NULL);

    priv->context_hash =
        g_hash_table_new_full (g_str_hash,
                               g_str_equal,
                               (GDestroyNotify)g_free,
                               (GDestroyNotify)g_object_unref);
}

static void
remove_context_from_stack (EekboardService        *service,
                           EekboardContextService *context)
{
    EekboardServicePrivate *priv = EEKBOARD_SERVICE_GET_PRIVATE(service);
    GSList *head;

    head = g_slist_find (priv->context_stack, context);
    if (head) {
        priv->context_stack = g_slist_remove_link (priv->context_stack, head);
        g_object_unref (head->data);
        g_slist_free1 (head);
    }
    if (priv->context_stack)
        eekboard_context_service_enable (priv->context_stack->data);
}

static void
service_name_vanished_callback (GDBusConnection *connection,
                                const gchar     *name,
                                gpointer         user_data)
{
    EekboardService *service = user_data;
    EekboardServicePrivate *priv = EEKBOARD_SERVICE_GET_PRIVATE(service);
    GSList *head;
    GHashTableIter iter;
    gpointer k, v;

    g_hash_table_iter_init (&iter, priv->context_hash);
    while (g_hash_table_iter_next (&iter, &k, &v)) {
        const gchar *owner = g_object_get_data (G_OBJECT(v), "owner");
        if (g_strcmp0 (owner, name) == 0)
            g_hash_table_iter_remove (&iter);
    }

    for (head = priv->context_stack; head; ) {
        const gchar *owner = g_object_get_data (G_OBJECT(head->data), "owner");
        GSList *next = g_slist_next (head);

        if (g_strcmp0 (owner, name) == 0) {
            priv->context_stack =
                g_slist_remove_link (priv->context_stack, head);
            g_object_unref (head->data);
            g_slist_free1 (head);
        }

        head = next;
    }

    if (priv->context_stack)
        eekboard_context_service_enable (priv->context_stack->data);
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
    EekboardService *service = user_data;
    EekboardServicePrivate *priv = EEKBOARD_SERVICE_GET_PRIVATE(service);
    EekboardServiceClass *klass = EEKBOARD_SERVICE_GET_CLASS(service);

    if (g_strcmp0 (method_name, "CreateContext") == 0) {
        const gchar *client_name;
        gchar *object_path;
        static gint context_id = 0;
        EekboardContextService *context;

        g_variant_get (parameters, "(&s)", &client_name);
        object_path = g_strdup_printf (EEKBOARD_CONTEXT_SERVICE_PATH, context_id++);
        g_assert (klass->create_context);
        context = klass->create_context (service, client_name, object_path);
        g_object_set_data_full (G_OBJECT(context),
                                "owner", g_strdup (sender),
                                (GDestroyNotify)g_free);
        g_hash_table_insert (priv->context_hash,
                             object_path,
                             context);

        /* the vanished callback is called when clients are disconnected */
        g_bus_watch_name_on_connection (priv->connection,
                                        sender,
                                        G_BUS_NAME_WATCHER_FLAGS_NONE,
                                        NULL,
                                        service_name_vanished_callback,
                                        service,
                                        NULL);
        g_dbus_method_invocation_return_value (invocation,
                                               g_variant_new ("(s)",
                                                              object_path));
        return;
    }

    if (g_strcmp0 (method_name, "PushContext") == 0) {
        const gchar *object_path;
        EekboardContextService *context;

        g_variant_get (parameters, "(&s)", &object_path);
        context = g_hash_table_lookup (priv->context_hash, object_path);
        if (!context) {
            g_dbus_method_invocation_return_error (invocation,
                                                   G_IO_ERROR,
                                                   G_IO_ERROR_FAILED_HANDLED,
                                                   "context not found");
            return;
        }
        if (priv->context_stack)
            eekboard_context_service_disable (priv->context_stack->data);
        priv->context_stack = g_slist_prepend (priv->context_stack,
                                               g_object_ref (context));
        eekboard_context_service_enable (context);

        g_dbus_method_invocation_return_value (invocation, NULL);
        return;
    }

    if (g_strcmp0 (method_name, "PopContext") == 0) {
        if (priv->context_stack) {
            EekboardContextService *context = priv->context_stack->data;
            gchar *object_path;
            const gchar *owner;

            g_object_get (G_OBJECT(context), "object-path", &object_path, NULL);
            owner = g_object_get_data (G_OBJECT(context), "owner");
            if (g_strcmp0 (owner, sender) != 0) {
                g_dbus_method_invocation_return_error
                    (invocation,
                     G_IO_ERROR,
                     G_IO_ERROR_FAILED_HANDLED,
                     "context at %s not owned by %s",
                     object_path, sender);
                return;
            }
            g_free (object_path);
                
            eekboard_context_service_disable (context);
            priv->context_stack = g_slist_next (priv->context_stack);
            if (priv->context_stack)
                eekboard_context_service_enable (priv->context_stack->data);
        }

        g_dbus_method_invocation_return_value (invocation, NULL);
        return;
    }

    if (g_strcmp0 (method_name, "DestroyContext") == 0) {
        EekboardContextService *context;
        const gchar *object_path;
        const gchar *owner;

        g_variant_get (parameters, "(&s)", &object_path);
        context = g_hash_table_lookup (priv->context_hash, object_path);
        if (!context) {
            g_dbus_method_invocation_return_error (invocation,
                                                   G_IO_ERROR,
                                                   G_IO_ERROR_FAILED_HANDLED,
                                                   "context not found");
            return;
        }

        owner = g_object_get_data (G_OBJECT(context), "owner");
        if (g_strcmp0 (owner, sender) != 0) {
            g_dbus_method_invocation_return_error
                (invocation,
                 G_IO_ERROR,
                 G_IO_ERROR_FAILED_HANDLED,
                 "the context at %s not owned by %s",
                 object_path, sender);
            return;
        }

        remove_context_from_stack (service, context);
        g_hash_table_remove (priv->context_hash, object_path);
        g_dbus_method_invocation_return_value (invocation, NULL);
        return;
    }

    if (g_strcmp0 (method_name, "Destroy") == 0) {
        g_signal_emit_by_name (service, "destroyed", NULL);
        g_dbus_method_invocation_return_value (invocation, NULL);
        return;
    }

    g_return_if_reached ();
}

EekboardService *
eekboard_service_new (const gchar     *object_path,
                      GDBusConnection *connection)
{
    return g_object_new (EEKBOARD_TYPE_SERVICE,
                         "object-path", object_path,
                         "connection", connection,
                         NULL);
}
