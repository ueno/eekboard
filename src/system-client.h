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
#ifndef EEKBOARD_SYSTEM_CLIENT_H
#define EEKBOARD_SYSTEM_CLIENT_H 1

#include <gio/gio.h>

G_BEGIN_DECLS

#define EEKBOARD_TYPE_SYSTEM_CLIENT (eekboard_system_client_get_type())
#define EEKBOARD_SYSTEM_CLIENT(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), EEKBOARD_TYPE_SYSTEM_CLIENT, EekboardSystemClient))
#define EEKBOARD_SYSTEM_CLIENT_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), EEKBOARD_TYPE_SYSTEM_CLIENT, EekboardSystemClientClass))
#define EEKBOARD_IS_SYSTEM_CLIENT(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EEKBOARD_TYPE_SYSTEM_CLIENT))
#define EEKBOARD_IS_SYSTEM_CLIENT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), EEKBOARD_TYPE_SYSTEM_CLIENT))
#define EEKBOARD_SYSTEM_CLIENT_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), EEKBOARD_TYPE_SYSTEM_CLIENT, EekboardSystemClientClass))

typedef struct _EekboardSystemClient EekboardSystemClient;

EekboardSystemClient * eekboard_system_client_new
                       (GDBusConnection      *connection);

gboolean               eekboard_system_client_enable_xkl
                       (EekboardSystemClient *client);
void                   eekboard_system_client_disable_xkl
                       (EekboardSystemClient *client);

gboolean               eekboard_system_client_enable_cspi
                       (EekboardSystemClient *client);
void                   eekboard_system_client_disable_cspi
                       (EekboardSystemClient *client);

gboolean               eekboard_system_client_enable_fakekey
                       (EekboardSystemClient *client);
void                   eekboard_system_client_disable_fakekey
                       (EekboardSystemClient *client);

G_END_DECLS
#endif  /* EEKBOARD_SYSTEM_CLIENT_H */
