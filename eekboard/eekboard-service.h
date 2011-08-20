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
#ifndef EEKBOARD_SERVICE_H
#define EEKBOARD_SERVICE_H 1

#define __EEKBOARD_SERVICE_H_INSIDE__ 1

#include "eekboard/eekboard-context-service.h"

G_BEGIN_DECLS

#define EEKBOARD_SERVICE_PATH "/org/fedorahosted/Eekboard"
#define EEKBOARD_SERVICE_INTERFACE "org.fedorahosted.Eekboard"

#define EEKBOARD_TYPE_SERVICE (eekboard_service_get_type())
#define EEKBOARD_SERVICE(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), EEKBOARD_TYPE_SERVICE, EekboardService))
#define EEKBOARD_SERVICE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), EEKBOARD_TYPE_SERVICE, EekboardServiceClass))
#define EEKBOARD_IS_SERVICE(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EEKBOARD_TYPE_SERVICE))
#define EEKBOARD_IS_SERVICE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), EEKBOARD_TYPE_SERVICE))
#define EEKBOARD_SERVICE_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), EEKBOARD_TYPE_SERVICE, EekboardServiceClass))

typedef struct _EekboardService EekboardService;
typedef struct _EekboardServiceClass EekboardServiceClass;
typedef struct _EekboardServicePrivate EekboardServicePrivate;

struct _EekboardService {
    GObject parent;

    EekboardServicePrivate *priv;
};

struct _EekboardServiceClass {
    /*< private >*/
    GObjectClass parent_class;

    /*< public >*/
    EekboardContextService *(*create_context) (EekboardService *self,
                                               const gchar     *client_name,
                                               const gchar     *object_path);

    /*< private >*/
    /* padding */
    gpointer pdummy[24];
};

GType             eekboard_service_get_type (void) G_GNUC_CONST;
EekboardService * eekboard_service_new      (const gchar     *object_path,
                                             GDBusConnection *connection);

G_END_DECLS
#endif  /* EEKBOARD_SERVICE_H */
