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

#define __EEKBOARD_CLIENT_H_INSIDE__ 1

#include <gio/gio.h>
#include "eekboard/eekboard-context.h"

G_BEGIN_DECLS

#define EEKBOARD_TYPE_CLIENT (eekboard_client_get_type())
#define EEKBOARD_CLIENT(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), EEKBOARD_TYPE_CLIENT, EekboardClient))
#define EEKBOARD_CLIENT_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), EEKBOARD_TYPE_CLIENT, EekboardClientClass))
#define EEKBOARD_IS_CLIENT(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EEKBOARD_TYPE_CLIENT))
#define EEKBOARD_IS_CLIENT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), EEKBOARD_TYPE_CLIENT))
#define EEKBOARD_CLIENT_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), EEKBOARD_TYPE_CLIENT, EekboardClientClass))

typedef struct _EekboardClient EekboardClient;
typedef struct _EekboardClientClass EekboardClientClass;
typedef struct _EekboardClientPrivate EekboardClientPrivate;

struct _EekboardClient {
    /*< private >*/
    GDBusProxy parent;

    EekboardClientPrivate *priv;
};

struct _EekboardClientClass {
    /*< private >*/
    GDBusProxyClass parent_class;

    /* signals */
    void (* destroyed) (EekboardClient *self);

    /*< private >*/
    /* padding */
    gpointer pdummy[23];
};

GType            eekboard_client_get_type        (void) G_GNUC_CONST;

EekboardClient  *eekboard_client_new             (GDBusConnection *connection,
                                                  GCancellable    *cancellable);
EekboardContext *eekboard_client_create_context  (EekboardClient  *eekboard,
                                                  const gchar     *client_name,
                                                  GCancellable    *cancellable);
void             eekboard_client_push_context    (EekboardClient  *eekboard,
                                                  EekboardContext *context,
                                                  GCancellable    *cancellable);
void             eekboard_client_pop_context     (EekboardClient  *eekboard,
                                                  GCancellable    *cancellable);
void             eekboard_client_destroy_context (EekboardClient  *eekboard,
                                                  EekboardContext *context,
                                                  GCancellable    *cancellable);

G_END_DECLS
#endif  /* EEKBOARD_CLIENT_H */
