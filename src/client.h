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
#ifndef EEKBOARD_CLIENT_H
#define EEKBOARD_CLIENT_H 1

#include <gio/gio.h>

G_BEGIN_DECLS

#define EEKBOARD_TYPE_CLIENT (eekboard_client_get_type())
#define EEKBOARD_CLIENT(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), EEKBOARD_TYPE_CLIENT, EekboardClient))
#define EEKBOARD_CLIENT_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), EEKBOARD_TYPE_CLIENT, EekboardClientClass))
#define EEKBOARD_IS_CLIENT(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EEKBOARD_TYPE_CLIENT))
#define EEKBOARD_IS_CLIENT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), EEKBOARD_TYPE_CLIENT))
#define EEKBOARD_CLIENT_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), EEKBOARD_TYPE_CLIENT, EekboardClientClass))

typedef struct _EekboardClient EekboardClient;

EekboardClient * eekboard_client_new             (GDBusConnection *connection);

gboolean         eekboard_client_load_keyboard_from_file
                                                 (EekboardClient  *client,
                                                  const gchar     *file);
gboolean         eekboard_client_set_xkl_config  (EekboardClient  *client,
                                                  const gchar     *model,
                                                  const gchar     *layouts,
                                                  const gchar     *options);

gboolean         eekboard_client_enable_xkl      (EekboardClient  *client);
void             eekboard_client_disable_xkl     (EekboardClient  *client);

gboolean         eekboard_client_enable_cspi_focus
                                                 (EekboardClient  *client);
void             eekboard_client_disable_cspi_focus
                                                 (EekboardClient  *client);

gboolean         eekboard_client_enable_cspi_keystroke
                                                 (EekboardClient  *client);
void             eekboard_client_disable_cspi_keystroke
                                                 (EekboardClient  *client);

gboolean         eekboard_client_enable_fakekey  (EekboardClient  *client);
void             eekboard_client_disable_fakekey (EekboardClient  *client);

G_END_DECLS
#endif  /* EEKBOARD_CLIENT_H */
