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
#ifndef SERVER_CONTEXT_SERVICE_H
#define SERVER_CONTEXT_SERVICE_H 1

G_BEGIN_DECLS

#include "eekboard/eekboard-service.h"

#define SERVER_TYPE_CONTEXT_SERVICE (server_context_service_get_type())
#define SERVER_CONTEXT_SERVICE(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), SERVER_TYPE_CONTEXT_SERVICE, ServerContextService))
#define SERVER_CONTEXT_SERVICE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), SERVER_TYPE_CONTEXT_SERVICE, ServerContextServiceClass))
#define SERVER_IS_CONTEXT_SERVICE(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SERVER_TYPE_CONTEXT_SERVICE))
#define SERVER_IS_CONTEXT_SERVICE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SERVER_TYPE_CONTEXT_SERVICE))
#define SERVER_CONTEXT_SERVICE_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), SERVER_TYPE_CONTEXT_SERVICE, ServerContextServiceClass))

typedef struct _ServerContextService ServerContextService;

ServerContextService *server_context_service_new (const gchar     *client_name,
                                                  const gchar     *object_path,
                                                  GDBusConnection *connection);

G_END_DECLS
#endif  /* SERVER_CONTEXT_SERVICE_H */

