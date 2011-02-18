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
#ifndef EEKBOARD_KEYBOARD_H
#define EEKBOARD_KEYBOARD_H 1

#include <gio/gio.h>
#include "eek/eek.h"

G_BEGIN_DECLS

#define EEKBOARD_TYPE_KEYBOARD (eekboard_keyboard_get_type())
#define EEKBOARD_KEYBOARD(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), EEKBOARD_TYPE_KEYBOARD, EekboardKeyboard))
#define EEKBOARD_KEYBOARD_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), EEKBOARD_TYPE_KEYBOARD, EekboardKeyboardClass))
#define EEKBOARD_IS_KEYBOARD(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EEKBOARD_TYPE_KEYBOARD))
#define EEKBOARD_IS_KEYBOARD_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), EEKBOARD_TYPE_KEYBOARD))
#define EEKBOARD_KEYBOARD_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), EEKBOARD_TYPE_KEYBOARD, EekboardKeyboardClass))

typedef struct _EekboardKeyboard EekboardKeyboard;
typedef struct _EekboardKeyboardClass EekboardKeyboardClass;
typedef struct _EekboardKeyboardPrivate EekboardKeyboardPrivate;

struct _EekboardKeyboard {
    GDBusProxy parent;

    EekboardKeyboardPrivate *priv;
};

struct _EekboardKeyboardClass {
    GDBusProxyClass parent_class;

    void (*key_pressed) (EekboardKeyboard *keyboard, guint keycode);
    void (*key_released) (EekboardKeyboard *keyboard, guint keycode);
};

GType             eekboard_keyboard_get_type    (void) G_GNUC_CONST;
EekboardKeyboard *eekboard_keyboard_new         (const gchar      *path,
                                                 GDBusConnection  *connection,
                                                 GCancellable     *cancellable,
                                                 GError          **error);
void              eekboard_keyboard_set_description
                                                (EekboardKeyboard *keyboard,
                                                 EekKeyboard      *description);
void              eekboard_keyboard_set_group   (EekboardKeyboard *keyboard,
                                                 gint              group);
void              eekboard_keyboard_show        (EekboardKeyboard *keyboard);
void              eekboard_keyboard_hide        (EekboardKeyboard *keyboard);
void              eekboard_keyboard_press_key   (EekboardKeyboard *keyboard,
                                                 guint             keycode);
void              eekboard_keyboard_release_key (EekboardKeyboard *keyboard,
                                                 guint             keycode);

G_END_DECLS
#endif  /* EEKBOARD_KEYBOARD_H */
