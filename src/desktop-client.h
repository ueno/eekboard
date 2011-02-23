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
#ifndef EEKBOARD_DESKTOP_CLIENT_H
#define EEKBOARD_DESKTOP_CLIENT_H 1

#include <gio/gio.h>

G_BEGIN_DECLS

#define EEKBOARD_TYPE_DESKTOP_CLIENT (eekboard_desktop_client_get_type())
#define EEKBOARD_DESKTOP_CLIENT(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), EEKBOARD_TYPE_DESKTOP_CLIENT, EekboardDesktopClient))
#define EEKBOARD_DESKTOP_CLIENT_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), EEKBOARD_TYPE_DESKTOP_CLIENT, EekboardDesktopClientClass))
#define EEKBOARD_IS_DESKTOP_CLIENT(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EEKBOARD_TYPE_DESKTOP_CLIENT))
#define EEKBOARD_IS_DESKTOP_CLIENT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), EEKBOARD_TYPE_DESKTOP_CLIENT))
#define EEKBOARD_DESKTOP_CLIENT_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), EEKBOARD_TYPE_DESKTOP_CLIENT, EekboardDesktopClientClass))

typedef struct _EekboardDesktopClient EekboardDesktopClient;

EekboardDesktopClient * eekboard_desktop_client_new
                       (GDBusConnection      *connection);

gboolean               eekboard_desktop_client_enable_xkl
                       (EekboardDesktopClient *client);
void                   eekboard_desktop_client_disable_xkl
                       (EekboardDesktopClient *client);

gboolean               eekboard_desktop_client_enable_cspi_focus
                       (EekboardDesktopClient *client);
void                   eekboard_desktop_client_disable_cspi_focus
                       (EekboardDesktopClient *client);

gboolean               eekboard_desktop_client_enable_cspi_keystroke
                       (EekboardDesktopClient *client);
void                   eekboard_desktop_client_disable_cspi_keystroke
                       (EekboardDesktopClient *client);

gboolean               eekboard_desktop_client_enable_fakekey
                       (EekboardDesktopClient *client);
void                   eekboard_desktop_client_disable_fakekey
                       (EekboardDesktopClient *client);

G_END_DECLS
#endif  /* EEKBOARD_DESKTOP_CLIENT_H */
