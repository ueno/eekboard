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
#ifndef EEKBOARD_EEKBOARD_H
#define EEKBOARD_EEKBOARD_H 1

#include <gio/gio.h>
#include "eekboard/eekboard-context.h"

G_BEGIN_DECLS

#define EEKBOARD_TYPE_EEKBOARD (eekboard_eekboard_get_type())
#define EEKBOARD_EEKBOARD(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), EEKBOARD_TYPE_EEKBOARD, EekboardEekboard))
#define EEKBOARD_EEKBOARD_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), EEKBOARD_TYPE_EEKBOARD, EekboardEekboardClass))
#define EEKBOARD_IS_EEKBOARD(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EEKBOARD_TYPE_EEKBOARD))
#define EEKBOARD_IS_EEKBOARD_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), EEKBOARD_TYPE_EEKBOARD))
#define EEKBOARD_EEKBOARD_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), EEKBOARD_TYPE_EEKBOARD, EekboardEekboardClass))

typedef struct _EekboardEekboard EekboardEekboard;
typedef struct _EekboardEekboardClass EekboardEekboardClass;
typedef struct _EekboardEekboardPrivate EekboardEekboardPrivate;

struct _EekboardEekboard {
    /*< private >*/
    GDBusProxy parent;

    EekboardEekboardPrivate *priv;
};

struct _EekboardEekboardClass {
    /*< private >*/
    GDBusProxyClass parent_class;

    /* signals */
    void (* destroyed) (EekboardEekboard *self);

    /*< private >*/
    /* padding */
    gpointer pdummy[23];
};

GType             eekboard_eekboard_get_type    (void) G_GNUC_CONST;

EekboardEekboard *eekboard_eekboard_new         (GDBusConnection  *connection,
                                                 GCancellable     *cancellable);
EekboardContext  *eekboard_eekboard_create_context
                                                (EekboardEekboard *eekboard,
                                                 const gchar      *client_name,
                                                 GCancellable     *cancellable);
void              eekboard_eekboard_push_context
                                                (EekboardEekboard *eekboard,
                                                 EekboardContext  *context,
                                                 GCancellable     *cancellable);
void              eekboard_eekboard_pop_context (EekboardEekboard *eekboard,
                                                 GCancellable     *cancellable);
void              eekboard_eekboard_destroy_context
                                                (EekboardEekboard *eekboard,
                                                 EekboardContext  *context,
                                                 GCancellable     *cancellable);

G_END_DECLS
#endif  /* EEKBOARD_EEKBOARD_H */
