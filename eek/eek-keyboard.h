/* 
 * Copyright (C) 2010-2011 Daiki Ueno <ueno@unixuser.org>
 * Copyright (C) 2010-2011 Red Hat, Inc.
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

#include <glib-object.h>
#include "eek-container.h"
#include "eek-types.h"
#include "eek-layout.h"

G_BEGIN_DECLS

#define EEK_TYPE_KEYBOARD (eek_keyboard_get_type())
#define EEK_KEYBOARD(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), EEK_TYPE_KEYBOARD, EekKeyboard))
#define EEK_KEYBOARD_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), EEK_TYPE_KEYBOARD, EekKeyboardClass))
#define EEK_IS_KEYBOARD(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EEK_TYPE_KEYBOARD))
#define EEK_IS_KEYBOARD_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), EEK_TYPE_KEYBOARD))
#define EEK_KEYBOARD_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), EEK_TYPE_KEYBOARD, EekKeyboardClass))

typedef struct _EekKeyboardClass EekKeyboardClass;
typedef struct _EekKeyboardPrivate EekKeyboardPrivate;

/**
 * EekKeyboard:
 *
 * The #EekKeyboard structure contains only private data and should
 * only be accessed using the provided API.
 */
struct _EekKeyboard
{
    /*< private >*/
    EekContainer parent;

    EekKeyboardPrivate *priv;
};

/**
 * EekKeyboardClass:
 * @create_section: virtual function for creating a section
 * @find_key_by_keycode: virtual function for finding a key in the
 * keyboard by keycode
 * @key_pressed: class handler for #EekKeyboard::key-pressed signal
 * @key_released: class handler for #EekKeyboard::key-released signal
 */
struct _EekKeyboardClass
{
    /*< private >*/
    EekContainerClass parent_class;

    /* obsolete members moved to EekElement */
    gpointer set_symbol_index;
    gpointer get_symbol_index;

    /*< public >*/
    EekSection *(* create_section)       (EekKeyboard *self);

    EekKey     *(* find_key_by_keycode)  (EekKeyboard *self,
                                          guint        keycode);

    /* signals */
    void        (* key_pressed)          (EekKeyboard *self,
                                          EekKey      *key);
    void        (* key_released)         (EekKeyboard *self,
                                          EekKey      *key);

    /*< private >*/
    /* obsolete members moved to EekElement */
    gpointer symbol_index_changed;

    /* padding */
    gpointer pdummy[24];
};

GType               eek_keyboard_get_type
                                     (void) G_GNUC_CONST;

EekKeyboard        *eek_keyboard_new (EekLayout          *layout,
                                      gdouble             initial_width,
                                      gdouble             initial_height);
EekLayout          *eek_keyboard_get_layout
                                     (EekKeyboard        *keyboard);
void                eek_keyboard_get_size
                                     (EekKeyboard        *keyboard,
                                      gdouble            *width,
                                      gdouble            *height);
void                eek_keyboard_set_size
                                     (EekKeyboard        *keyboard,
                                      gdouble             width,
                                      gdouble             height);
void                eek_keyboard_set_symbol_index
                                     (EekKeyboard        *keyboard,
                                      gint                group,
                                      gint                level);
void                eek_keyboard_get_symbol_index
                                     (EekKeyboard        *keyboard,
                                      gint               *group,
                                      gint               *level);

void                eek_keyboard_set_group
                                     (EekKeyboard        *keyboard,
                                      gint                group);
void                eek_keyboard_set_level
                                     (EekKeyboard        *keyboard,
                                      gint                level);
gint                eek_keyboard_get_group
                                     (EekKeyboard        *keyboard);
gint                eek_keyboard_get_level
                                     (EekKeyboard        *keyboard);

void                eek_keyboard_set_modifier_behavior
                                     (EekKeyboard        *keyboard,
                                      EekModifierBehavior modifier_behavior);
EekModifierBehavior eek_keyboard_get_modifier_behavior
                                     (EekKeyboard        *keyboard);
EekModifierType     eek_keyboard_get_modifiers
                                     (EekKeyboard        *keyboard);

EekSection         *eek_keyboard_create_section
                                     (EekKeyboard        *keyboard);

EekKey             *eek_keyboard_find_key_by_keycode
                                     (EekKeyboard        *keyboard,
                                      guint               keycode);

gulong              eek_keyboard_add_outline
                                     (EekKeyboard        *keyboard,
                                      EekOutline         *outline);

EekOutline         *eek_keyboard_get_outline
                                     (EekKeyboard        *keyboard,
                                      gulong              oref);

void                eek_keyboard_set_num_lock_mask
                                     (EekKeyboard        *keyboard,
                                      EekModifierType     num_lock_mask);
EekModifierType     eek_keyboard_get_num_lock_mask
                                     (EekKeyboard        *keyboard);

G_END_DECLS
#endif  /* EEK_KEYBOARD_H */
