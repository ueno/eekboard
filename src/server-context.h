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
#ifndef SERVER_CONTEXT_H
#define SERVER_CONTEXT_H 1

#include <gio/gio.h>

G_BEGIN_DECLS

#define SERVER_CONTEXT_PATH "/com/redhat/Eekboard/Context_%d"
#define SERVER_CONTEXT_INTERFACE "com.redhat.Eekboard.Context"

#define SERVER_TYPE_CONTEXT (server_context_get_type())
#define SERVER_CONTEXT(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), SERVER_TYPE_CONTEXT, ServerContext))
#define SERVER_CONTEXT_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), SERVER_TYPE_CONTEXT, ServerContextClass))
#define SERVER_IS_CONTEXT(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SERVER_TYPE_CONTEXT))
#define SERVER_IS_CONTEXT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SERVER_TYPE_CONTEXT))
#define SERVER_CONTEXT_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), SERVER_TYPE_CONTEXT, ServerContextClass))

typedef struct _ServerContext ServerContext;

ServerContext *server_context_new         (const gchar     *object_path,
                                           GDBusConnection *connection);
void           server_context_set_enabled (ServerContext   *context,
                                           gboolean         enabled);
void           server_context_set_client_connection
                                          (ServerContext   *context,
                                           const gchar     *client_connection);
const gchar   *server_context_get_client_connection
                                          (ServerContext   *context);
void           server_context_set_client_name
                                          (ServerContext   *context,
                                           const gchar     *client_name);

G_END_DECLS
#endif  /* SERVER_CONTEXT_H */
