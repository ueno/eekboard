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
#ifndef EEKBOARD_DEVICE_H
#define EEKBOARD_DEVICE_H 1

#include <gio/gio.h>
#include "eek/eek.h"

G_BEGIN_DECLS

#define EEKBOARD_TYPE_DEVICE (eekboard_device_get_type())
#define EEKBOARD_DEVICE(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), EEKBOARD_TYPE_DEVICE, EekboardDevice))
#define EEKBOARD_DEVICE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), EEKBOARD_TYPE_DEVICE, EekboardDeviceClass))
#define EEKBOARD_IS_DEVICE(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EEKBOARD_TYPE_DEVICE))
#define EEKBOARD_IS_DEVICE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), EEKBOARD_TYPE_DEVICE))
#define EEKBOARD_DEVICE_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), EEKBOARD_TYPE_DEVICE, EekboardDeviceClass))

typedef struct _EekboardDevice EekboardDevice;
typedef struct _EekboardDeviceClass EekboardDeviceClass;

EekboardDevice *eekboard_device_new          (const gchar     *path,
                                              GDBusConnection *connection,
                                              GCancellable    *cancellable,
                                              GError         **error);
void            eekboard_device_set_keyboard (EekboardDevice  *device,
                                              EekKeyboard     *keyboard);
void            eekboard_device_set_group    (EekboardDevice  *device,
                                              gint             group);
void            eekboard_device_show         (EekboardDevice  *device);
void            eekboard_device_hide         (EekboardDevice  *device);
void            eekboard_device_press_key    (EekboardDevice  *device,
                                              guint            keycode);
void            eekboard_device_release_key  (EekboardDevice  *device,
                                              guint            keycode);

G_END_DECLS
#endif  /* EEKBOARD_DEVICE_H */
