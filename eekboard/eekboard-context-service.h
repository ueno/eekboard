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
#if !defined(__EEKBOARD_SERVICE_H_INSIDE__) && !defined(EEKBOARD_COMPILATION)
#error "Only <eekboard/eekboard-service.h> can be included directly."
#endif

#ifndef EEKBOARD_CONTEXT_SERVICE_H
#define EEKBOARD_CONTEXT_SERVICE_H 1

#include <eek/eek.h>

G_BEGIN_DECLS

#define EEKBOARD_CONTEXT_SERVICE_PATH "/org/fedorahosted/Eekboard/Context_%d"
#define EEKBOARD_CONTEXT_SERVICE_INTERFACE "org.fedorahosted.Eekboard.Context"

#define EEKBOARD_TYPE_CONTEXT_SERVICE (eekboard_context_service_get_type())
#define EEKBOARD_CONTEXT_SERVICE(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), EEKBOARD_TYPE_CONTEXT_SERVICE, EekboardContextService))
#define EEKBOARD_CONTEXT_SERVICE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), EEKBOARD_TYPE_CONTEXT_SERVICE, EekboardContextServiceClass))
#define EEKBOARD_IS_CONTEXT_SERVICE(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EEKBOARD_TYPE_CONTEXT_SERVICE))
#define EEKBOARD_IS_CONTEXT_SERVICE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), EEKBOARD_TYPE_CONTEXT_SERVICE))
#define EEKBOARD_CONTEXT_SERVICE_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), EEKBOARD_TYPE_CONTEXT_SERVICE, EekboardContextServiceClass))

typedef struct _EekboardContextService EekboardContextService;
typedef struct _EekboardContextServiceClass EekboardContextServiceClass;
typedef struct _EekboardContextServicePrivate EekboardContextServicePrivate;

struct _EekboardContextService {
    GObject parent;

    EekboardContextServicePrivate *priv;
};

struct _EekboardContextServiceClass {
    /*< private >*/
    GObjectClass parent_class;

    EekKeyboard *(*create_keyboard)    (EekboardContextService *self,
                                        const gchar            *keyboard_type);
    void         (*show_keyboard)      (EekboardContextService *self);
    void         (*hide_keyboard)      (EekboardContextService *self);

    /* signals */
    void         (*enabled)            (EekboardContextService *self);
    void         (*disabled)           (EekboardContextService *self);

    /*< private >*/
    /* padding */
    gpointer pdummy[24];
};

GType              eekboard_context_service_get_type
                   (void) G_GNUC_CONST;
void               eekboard_context_service_enable
                   (EekboardContextService *context);
void               eekboard_context_service_disable
                   (EekboardContextService *context);
const EekKeyboard *eekboard_context_service_get_keyboard
                   (EekboardContextService *context);
gboolean           eekboard_context_service_get_fullscreen
                   (EekboardContextService *context);
const gchar *      eekboard_context_service_get_client_name
                   (EekboardContextService *context);

G_END_DECLS
#endif  /* EEKBOARD_CONTEXT_SERVICE_H */
