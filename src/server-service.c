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

#include "server-service.h"
#include "server-context-service.h"

typedef struct _ServerServiceClass ServerServiceClass;

struct _ServerService {
    EekboardService parent;
};

struct _ServerServiceClass {
    EekboardServiceClass parent_class;
};

G_DEFINE_TYPE (ServerService, server_service, EEKBOARD_TYPE_SERVICE);

static EekboardContextService *
server_service_real_create_context (EekboardService *self,
                                    const gchar     *client_name,
                                    const gchar     *object_path)
{
    GDBusConnection *connection;
    ServerContextService *context;

    g_object_get (G_OBJECT(self), "connection", &connection, NULL);
    context = server_context_service_new (client_name, object_path, connection);
    g_object_unref (connection);

    return EEKBOARD_CONTEXT_SERVICE(context);
}

static void
server_service_class_init (ServerServiceClass *klass)
{
    EekboardServiceClass *service_class = EEKBOARD_SERVICE_CLASS(klass);
    service_class->create_context = server_service_real_create_context;
}

static void
server_service_init (ServerService *self)
{
}

ServerService *server_service_new (const gchar     *object_path,
                                   GDBusConnection *connection)
{
    return g_object_new (SERVER_TYPE_SERVICE,
                         "object-path", object_path,
                         "connection", connection,
                         NULL);
}
