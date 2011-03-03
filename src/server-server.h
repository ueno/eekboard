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
#ifndef SERVER_KEYBOARD_H
#define SERVER_KEYBOARD_H 1

#include <gio/gio.h>

G_BEGIN_DECLS

#define SERVER_SERVER_PATH "/com/redhat/Eekboard/Server"
#define SERVER_SERVER_INTERFACE "com.redhat.Eekboard.Server"

#define SERVER_TYPE_SERVER (server_server_get_type())
#define SERVER_SERVER(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), SERVER_TYPE_SERVER, ServerServer))
#define SERVER_SERVER_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), SERVER_TYPE_SERVER, ServerServerClass))
#define SERVER_IS_SERVER(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SERVER_TYPE_SERVER))
#define SERVER_IS_SERVER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SERVER_TYPE_SERVER))
#define SERVER_SERVER_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), SERVER_TYPE_SERVER, ServerServerClass))

typedef struct _ServerServer ServerServer;

ServerServer *server_server_new (const gchar     *object_path,
                                 GDBusConnection *connection);

G_END_DECLS
#endif  /* SERVER_SERVER_H */
