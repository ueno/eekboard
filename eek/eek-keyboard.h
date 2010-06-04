/* 
 * Copyright (C) 2010 Daiki Ueno <ueno@unixuser.org>
 * Copyright (C) 2010 Red Hat, Inc.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA
 */
#ifndef EEK_KEYBOARD_H
#define EEK_KEYBOARD_H 1

#include "eek-section.h"
#include "eek-layout.h"

G_BEGIN_DECLS

#define EEK_TYPE_KEYBOARD (eek_keyboard_get_type())
#define EEK_KEYBOARD(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), EEK_TYPE_KEYBOARD, EekKeyboard))
#define EEK_IS_KEYBOARD(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EEK_TYPE_KEYBOARD))
#define EEK_KEYBOARD_GET_IFACE(obj) (G_TYPE_INSTANCE_GET_INTERFACE ((obj), EEK_TYPE_KEYBOARD, EekKeyboardIface))

typedef struct _EekKeyboardIface EekKeyboardIface;

struct _EekKeyboardIface
{
    /*< private >*/
    GTypeInterface g_iface;

    /*< public >*/
    void        (* set_bounds)      (EekKeyboard *self,
                                     EekBounds   *bounds);
    void        (* get_bounds)      (EekKeyboard *self,
                                     EekBounds   *bounds);
    EekSection *(* create_section)  (EekKeyboard *self,
                                     const gchar *name,
                                     gint         angle,
                                     EekBounds   *bounds);
                                    
    void        (* foreach_section) (EekKeyboard *self,
                                     GFunc        func,
                                     gpointer     user_data);

    void        (* set_layout)      (EekKeyboard *self,
                                     EekLayout   *layout);
};

GType       eek_keyboard_get_type        (void) G_GNUC_CONST;

void        eek_keyboard_set_bounds      (EekKeyboard *keyboard,
                                          EekBounds   *bounds);
void        eek_keyboard_get_bounds      (EekKeyboard *keyboard,
                                          EekBounds   *bounds);

EekSection *eek_keyboard_create_section  (EekKeyboard *keyboard,
                                          const gchar *name,
                                          gint         angle,
                                          EekBounds   *bounds);

void        eek_keyboard_foreach_section (EekKeyboard *keyboard,
                                          GFunc        func,
                                          gpointer     user_data);

void        eek_keyboard_set_layout      (EekKeyboard *keyboard,
                                          EekLayout   *layout);

G_END_DECLS
#endif  /* EEK_KEYBOARD_H */
