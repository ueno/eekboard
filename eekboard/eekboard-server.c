/* 
 * Copyright (C) 2011 Daiki Ueno <ueno@unixuser.org>
 * Copyright (C) 2011 Red Hat, Inc.
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
 * SECTION:eekboard-server
 * @short_description: D-Bus proxy of eekboard-server
 *
 * The #EekboardServer class provides a client side access to eekboard-server.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif  /* HAVE_CONFIG_H */

#include "eekboard/eekboard-server.h"

G_DEFINE_TYPE (EekboardServer, eekboard_server, G_TYPE_DBUS_PROXY);

#define EEKBOARD_SERVER_GET_PRIVATE(obj)                               \
    (G_TYPE_INSTANCE_GET_PRIVATE ((obj), EEKBOARD_TYPE_SERVER, EekboardServerPrivate))

struct _EekboardServerPrivate
{
    GHashTable *context_hash;
};

static void
eekboard_server_dispose (GObject *object)
{
    EekboardServerPrivate *priv = EEKBOARD_SERVER_GET_PRIVATE(object);

    if (priv->context_hash) {
        g_hash_table_destroy (priv->context_hash);
        priv->context_hash = NULL;
    }

    G_OBJECT_CLASS (eekboard_server_parent_class)->dispose (object);
}

static void
eekboard_server_class_init (EekboardServerClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    g_type_class_add_private (gobject_class,
                              sizeof (EekboardServerPrivate));

    gobject_class->dispose = eekboard_server_dispose;
}

static void
eekboard_server_init (EekboardServer *self)
{
    EekboardServerPrivate *priv;

    priv = self->priv = EEKBOARD_SERVER_GET_PRIVATE(self);
    priv->context_hash =
        g_hash_table_new_full (g_str_hash,
                               g_str_equal,
                               (GDestroyNotify)g_free,
                               (GDestroyNotify)g_object_unref);
}

/**
 * eekboard_server_new:
 * @connection: a #GDBusConnection
 * @cancellable: a #GCancellable
 *
 * Create a D-Bus proxy of eekboard-server.
 */
EekboardServer *
eekboard_server_new (GDBusConnection *connection,
                     GCancellable    *cancellable)
{
    GInitable *initable;
    GError *error;

    g_assert (G_IS_DBUS_CONNECTION(connection));

    error = NULL;
    initable =
        g_initable_new (EEKBOARD_TYPE_SERVER,
                        cancellable,
                        &error,
                        "g-connection", connection,
                        "g-name", "com.redhat.Eekboard.Server",
                        "g-interface-name", "com.redhat.Eekboard.Server",
                        "g-object-path", "/com/redhat/Eekboard/Server",
                        NULL);
    if (initable != NULL)
        return EEKBOARD_SERVER (initable);
    return NULL;
}

/**
 * eekboard_server_create_context:
 * @server: an #EekboardServer
 * @client_name: name of the client
 * @cancellable: a #GCancellable
 *
 * Create a new input context.
 */
EekboardContext *
eekboard_server_create_context (EekboardServer *server,
                                const gchar    *client_name,
                                GCancellable   *cancellable)
{
    GVariant *variant;
    const gchar *object_path;
    EekboardContext *context;
    EekboardServerPrivate *priv;
    GError *error;
    GDBusConnection *connection;

    g_assert (EEKBOARD_IS_SERVER(server));
    g_assert (client_name);

    error = NULL;
    variant = g_dbus_proxy_call_sync (G_DBUS_PROXY(server),
                                      "CreateContext",
                                      g_variant_new ("(s)", client_name),
                                      G_DBUS_CALL_FLAGS_NONE,
                                      -1,
                                      cancellable,
                                      &error);
    if (!variant)
        return NULL;

    g_variant_get (variant, "(&s)", &object_path);
    connection = g_dbus_proxy_get_connection (G_DBUS_PROXY(server));
    context = eekboard_context_new (connection, object_path, cancellable);
    if (!context) {
        g_variant_unref (variant);
        return NULL;
    }

    priv = EEKBOARD_SERVER_GET_PRIVATE(server);
    g_hash_table_insert (priv->context_hash,
                         g_strdup (object_path),
                         g_object_ref (context));
    return context;
}

static void
server_async_ready_callback (GObject      *source_object,
                             GAsyncResult *res,
                             gpointer      user_data)
{
    GError *error = NULL;
    GVariant *result;

    result = g_dbus_proxy_call_finish (G_DBUS_PROXY(source_object),
                                       res,
                                       &error);
    if (result)
        g_variant_unref (result);
}

/**
 * eekboard_server_push_context:
 * @server: an #EekboardServer
 * @context: an #EekboardContext
 * @cancellable: a #GCancellable
 *
 * Enable the input context @context and disable the others.
 */
void
eekboard_server_push_context (EekboardServer  *server,
                              EekboardContext *context,
                              GCancellable    *cancellable)
{
    EekboardServerPrivate *priv;
    const gchar *object_path;

    g_return_if_fail (EEKBOARD_IS_SERVER(server));
    g_return_if_fail (EEKBOARD_IS_CONTEXT(context));

    object_path = g_dbus_proxy_get_object_path (G_DBUS_PROXY(context));

    priv = EEKBOARD_SERVER_GET_PRIVATE(server);
    context = g_hash_table_lookup (priv->context_hash, object_path);
    if (!context)
        return;

    eekboard_context_set_enabled (context, TRUE);
    g_dbus_proxy_call (G_DBUS_PROXY(server),
                       "PushContext",
                       g_variant_new ("(s)", object_path),
                       G_DBUS_CALL_FLAGS_NONE,
                       -1,
                       cancellable,
                       server_async_ready_callback,
                       NULL);
}

/**
 * eekboard_server_pop_context:
 * @server: an #EekboardServer
 * @cancellable: a #GCancellable
 *
 * Disable the current input context and enable the previous one.
 */
void
eekboard_server_pop_context (EekboardServer  *server,
                             GCancellable    *cancellable)
{
    g_return_if_fail (EEKBOARD_IS_SERVER(server));

    g_dbus_proxy_call (G_DBUS_PROXY(server),
                       "PopContext",
                       NULL,
                       G_DBUS_CALL_FLAGS_NONE,
                       -1,
                       cancellable,
                       server_async_ready_callback,
                       NULL);
}

/**
 * eekboard_server_destroy_context:
 * @server: an #EekboardServer
 * @context: an #EekboardContext
 * @cancellable: a #GCancellable
 *
 * Remove @context from @server.
 */
void
eekboard_server_destroy_context (EekboardServer  *server,
                                 EekboardContext *context,
                                 GCancellable    *cancellable)
{
    EekboardServerPrivate *priv;
    const gchar *object_path;

    g_return_if_fail (EEKBOARD_IS_SERVER(server));
    g_return_if_fail (EEKBOARD_IS_CONTEXT(context));

    priv = EEKBOARD_SERVER_GET_PRIVATE(server);

    object_path = g_dbus_proxy_get_object_path (G_DBUS_PROXY(context));
    g_hash_table_remove (priv->context_hash, object_path);

    g_dbus_proxy_call (G_DBUS_PROXY(server),
                       "DestroyContext",
                       g_variant_new ("(s)", object_path),
                       G_DBUS_CALL_FLAGS_NONE,
                       -1,
                       cancellable,
                       server_async_ready_callback,
                       NULL);
}
