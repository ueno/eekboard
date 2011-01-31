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
#ifndef EEKBOARD_PROXY_H
#define EEKBOARD_PROXY_H 1

#include <gio/gio.h>
#include "eek/eek.h"

G_BEGIN_DECLS

#define EEKBOARD_TYPE_PROXY (eekboard_proxy_get_type())
#define EEKBOARD_PROXY(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), EEKBOARD_TYPE_PROXY, EekboardProxy))
#define EEKBOARD_PROXY_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), EEKBOARD_TYPE_PROXY, EekboardProxyClass))
#define EEKBOARD_IS_PROXY(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EEKBOARD_TYPE_PROXY))
#define EEKBOARD_IS_PROXY_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), EEKBOARD_TYPE_PROXY))
#define EEKBOARD_PROXY_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), EEKBOARD_TYPE_PROXY, EekboardProxyClass))

typedef struct _EekboardProxy EekboardProxy;
typedef struct _EekboardProxyClass EekboardProxyClass;

EekboardProxy *eekboard_proxy_new          (const gchar     *path,
                                            GDBusConnection *connection,
                                            GCancellable    *cancellable,
                                            GError         **error);
void           eekboard_proxy_set_keyboard (EekboardProxy   *proxy,
                                            EekKeyboard     *keyboard);
void           eekboard_proxy_set_group    (EekboardProxy   *proxy,
                                            gint             group);
void           eekboard_proxy_show         (EekboardProxy   *proxy);
void           eekboard_proxy_hide         (EekboardProxy   *proxy);

G_END_DECLS
#endif  /* EEKBOARD_PROXY_H */
