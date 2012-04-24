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
    gboolean visible;
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
    "    <method name='ShowKeyboard'/>"
    "    <method name='HideKeyboard'/>"
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
    GDBusConnection *connection;

    switch (prop_id) {
    case PROP_OBJECT_PATH:
        if (service->priv->object_path)
            g_free (service->priv->object_path);
        service->priv->object_path = g_value_dup_string (value);
        break;
    case PROP_CONNECTION:
        connection = g_value_get_object (value);
        if (service->priv->connection)
            g_object_unref (service->priv->connection);
        service->priv->connection = g_object_ref (connection);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
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

    switch (prop_id) {
    case PROP_OBJECT_PATH:
        g_value_set_string (value, service->priv->object_path);
        break;
    case PROP_CONNECTION:
        g_value_set_object (value, service->priv->connection);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void
eekboard_service_dispose (GObject *object)
{
    EekboardService *service = EEKBOARD_SERVICE(object);
    GSList *head;

    if (service->priv->context_hash) {
        g_hash_table_destroy (service->priv->context_hash);
        service->priv->context_hash = NULL;
    }

    for (head = service->priv->context_stack; head; head = service->priv->context_stack) {
        g_object_unref (head->data);
        service->priv->context_stack = g_slist_next (head);
        g_slist_free1 (head);
    }

    if (service->priv->connection) {
        if (service->priv->registration_id > 0) {
            g_dbus_connection_unregister_object (service->priv->connection,
                                                 service->priv->registration_id);
            service->priv->registration_id = 0;
        }

        g_object_unref (service->priv->connection);
        service->priv->connection = NULL;
    }

    if (service->priv->introspection_data) {
        g_dbus_node_info_unref (service->priv->introspection_data);
        service->priv->introspection_data = NULL;
    }

    G_OBJECT_CLASS (eekboard_service_parent_class)->dispose (object);
}

static void
eekboard_service_finalize (GObject *object)
{
    EekboardService *service = EEKBOARD_SERVICE(object);

    g_free (service->priv->object_path);

    G_OBJECT_CLASS (eekboard_service_parent_class)->finalize (object);
}

static void
eekboard_service_constructed (GObject *object)
{
    EekboardService *service = EEKBOARD_SERVICE(object);
    if (service->priv->connection && service->priv->object_path) {
        GError *error = NULL;

        service->priv->registration_id = g_dbus_connection_register_object
            (service->priv->connection,
             service->priv->object_path,
             service->priv->introspection_data->interfaces[0],
             &interface_vtable,
             object,
             NULL,
             &error);

        if (service->priv->registration_id == 0) {
            g_warning ("failed to register context object: %s",
                       error->message);
            g_error_free (error);
        }
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

    /**
     * EekboardService:object-path:
     *
     * D-Bus object path.
     */
    pspec = g_param_spec_string ("object-path",
                                 "Object-path",
                                 "Object-path",
                                 NULL,
                                 G_PARAM_CONSTRUCT | G_PARAM_READWRITE);
    g_object_class_install_property (gobject_class,
                                     PROP_OBJECT_PATH,
                                     pspec);

    /**
     * EekboardService:connection:
     *
     * D-Bus connection.
     */
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
eekboard_service_init (EekboardService *self)
{
    GError *error;

    self->priv = EEKBOARD_SERVICE_GET_PRIVATE(self);

    error = NULL;
    self->priv->introspection_data =
        g_dbus_node_info_new_for_xml (introspection_xml, &error);
    if (self->priv->introspection_data == NULL) {
        g_warning ("failed to parse D-Bus XML: %s", error->message);
        g_error_free (error);
        g_assert_not_reached ();
    }

    self->priv->context_hash =
        g_hash_table_new_full (g_str_hash,
                               g_str_equal,
                               (GDestroyNotify)g_free,
                               (GDestroyNotify)g_object_unref);
}

static void
remove_context_from_stack (EekboardService        *service,
                           EekboardContextService *context)
{
    GSList *head;

    head = g_slist_find (service->priv->context_stack, context);
    if (head) {
        service->priv->context_stack = g_slist_remove_link (service->priv->context_stack, head);
        g_object_unref (head->data);
        g_slist_free1 (head);
    }
    if (service->priv->context_stack)
        eekboard_context_service_enable (service->priv->context_stack->data);
}

static void
service_name_vanished_callback (GDBusConnection *connection,
                                const gchar     *name,
                                gpointer         user_data)
{
    EekboardService *service = user_data;
    GSList *head;
    GHashTableIter iter;
    gpointer k, v;

    g_hash_table_iter_init (&iter, service->priv->context_hash);
    while (g_hash_table_iter_next (&iter, &k, &v)) {
        const gchar *owner = g_object_get_data (G_OBJECT(v), "owner");
        if (g_strcmp0 (owner, name) == 0)
            g_hash_table_iter_remove (&iter);
    }

    for (head = service->priv->context_stack; head; ) {
        const gchar *owner = g_object_get_data (G_OBJECT(head->data), "owner");
        GSList *next = g_slist_next (head);

        if (g_strcmp0 (owner, name) == 0) {
            service->priv->context_stack =
                g_slist_remove_link (service->priv->context_stack, head);
            g_object_unref (head->data);
            g_slist_free1 (head);
        }

        head = next;
    }

    if (service->priv->context_stack)
        eekboard_context_service_enable (service->priv->context_stack->data);
}

static void
context_destroyed_cb (EekboardContextService *context, EekboardService *service)
{
    gchar *object_path = NULL;

    remove_context_from_stack (service, context);

    g_object_get (G_OBJECT(context), "object-path", &object_path, NULL);
    g_hash_table_remove (service->priv->context_hash, object_path);
    g_free (object_path);
}

static void
on_notify_visible (GObject *object, GParamSpec *spec, gpointer user_data)
{
    EekboardService *service = user_data;

    g_object_get (object, "visible", &service->priv->visible, NULL);
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
        g_hash_table_insert (service->priv->context_hash,
                             object_path,
                             context);

        /* the vanished callback is called when clients are disconnected */
        g_bus_watch_name_on_connection (service->priv->connection,
                                        sender,
                                        G_BUS_NAME_WATCHER_FLAGS_NONE,
                                        NULL,
                                        service_name_vanished_callback,
                                        service,
                                        NULL);

        g_signal_connect (G_OBJECT(context), "destroyed",
                          G_CALLBACK(context_destroyed_cb), service);

        g_dbus_method_invocation_return_value (invocation,
                                               g_variant_new ("(s)",
                                                              object_path));
        return;
    }

    if (g_strcmp0 (method_name, "PushContext") == 0) {
        const gchar *object_path;
        EekboardContextService *context;

        g_variant_get (parameters, "(&s)", &object_path);
        context = g_hash_table_lookup (service->priv->context_hash, object_path);
        if (!context) {
            g_dbus_method_invocation_return_error (invocation,
                                                   G_IO_ERROR,
                                                   G_IO_ERROR_FAILED_HANDLED,
                                                   "context not found");
            return;
        }
        if (service->priv->context_stack)
            eekboard_context_service_disable (service->priv->context_stack->data);
        service->priv->context_stack = g_slist_prepend (service->priv->context_stack,
                                               g_object_ref (context));
        eekboard_context_service_enable (context);
        g_signal_connect (context, "notify::visible",
                          G_CALLBACK(on_notify_visible), service);
        if (service->priv->visible)
            eekboard_context_service_show_keyboard (context);

        g_dbus_method_invocation_return_value (invocation, NULL);
        return;
    }

    if (g_strcmp0 (method_name, "PopContext") == 0) {
        if (service->priv->context_stack) {
            EekboardContextService *context = service->priv->context_stack->data;
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
                
            g_signal_handlers_disconnect_by_func (context,
                                                  G_CALLBACK(on_notify_visible),
                                                  service);
            eekboard_context_service_disable (context);
            service->priv->context_stack = g_slist_next (service->priv->context_stack);
            if (service->priv->context_stack)
                eekboard_context_service_enable (service->priv->context_stack->data);
        }

        g_dbus_method_invocation_return_value (invocation, NULL);
        return;
    }

    if (g_strcmp0 (method_name, "ShowKeyboard") == 0) {
        if (service->priv->context_stack) {
            eekboard_context_service_show_keyboard (service->priv->context_stack->data);
        } else {
            service->priv->visible = TRUE;
        }
        return;
    }

    if (g_strcmp0 (method_name, "HideKeyboard") == 0) {
        if (service->priv->context_stack) {
            eekboard_context_service_hide_keyboard (service->priv->context_stack->data);
        } else {
            service->priv->visible = FALSE;
        }
        return;
    }

    if (g_strcmp0 (method_name, "Destroy") == 0) {
        g_signal_emit (service, signals[DESTROYED], 0);
        g_dbus_method_invocation_return_value (invocation, NULL);
        return;
    }

    g_return_if_reached ();
}

/**
 * eekboard_service_new:
 * @connection: a #GDBusConnection
 * @object_path: object path
 *
 * Create an empty server for testing purpose.
 */
EekboardService *
eekboard_service_new (GDBusConnection *connection,
                      const gchar     *object_path)
{
    return g_object_new (EEKBOARD_TYPE_SERVICE,
                         "object-path", object_path,
                         "connection", connection,
                         NULL);
}
