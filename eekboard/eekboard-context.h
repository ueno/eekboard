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
#ifndef EEKBOARD_CONTEXT_H
#define EEKBOARD_CONTEXT_H 1

#include <gio/gio.h>
#include "eek/eek.h"

G_BEGIN_DECLS

#define EEKBOARD_TYPE_CONTEXT (eekboard_context_get_type())
#define EEKBOARD_CONTEXT(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), EEKBOARD_TYPE_CONTEXT, EekboardContext))
#define EEKBOARD_CONTEXT_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), EEKBOARD_TYPE_CONTEXT, EekboardContextClass))
#define EEKBOARD_IS_CONTEXT(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EEKBOARD_TYPE_CONTEXT))
#define EEKBOARD_IS_CONTEXT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), EEKBOARD_TYPE_CONTEXT))
#define EEKBOARD_CONTEXT_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), EEKBOARD_TYPE_CONTEXT, EekboardContextClass))

typedef struct _EekboardContext EekboardContext;
typedef struct _EekboardContextClass EekboardContextClass;
typedef struct _EekboardContextPrivate EekboardContextPrivate;

struct _EekboardContext {
    GDBusProxy parent;

    EekboardContextPrivate *priv;
};

struct _EekboardContextClass {
    GDBusProxyClass parent_class;

    void (*enabled)      (EekboardContext *self);
    void (*disabled)     (EekboardContext *self);
    void (*key_pressed)  (EekboardContext *self,
                          guint            keycode);
    void (*key_released) (EekboardContext *self,
                          guint            keycode);
};

GType            eekboard_context_get_type      (void) G_GNUC_CONST;

EekboardContext *eekboard_context_new           (GDBusConnection *connection,
                                                 const gchar     *object_path,
                                                 GCancellable    *cancellable);
void             eekboard_context_set_keyboard  (EekboardContext *context,
                                                 EekKeyboard     *keyboard,
                                                 GCancellable    *cancellable);
void             eekboard_context_show_keyboard (EekboardContext *context,
                                                 GCancellable    *cancellable);
void             eekboard_context_hide_keyboard (EekboardContext *context,
                                                 GCancellable    *cancellable);
void             eekboard_context_set_group     (EekboardContext *context,
                                                 gint             group,
                                                 GCancellable    *cancellable);
void             eekboard_context_press_key     (EekboardContext *context,
                                                 guint            keycode,
                                                 GCancellable    *cancellable);
void             eekboard_context_release_key   (EekboardContext *context,
                                                 guint            keycode,
                                                 GCancellable    *cancellable);
gboolean         eekboard_context_is_keyboard_visible
                                                (EekboardContext *context);
void             eekboard_context_set_enabled   (EekboardContext *context,
                                                 gboolean         enabled);
gboolean         eekboard_context_is_enabled    (EekboardContext *context);

G_END_DECLS
#endif  /* EEKBOARD_CONTEXT_H */
