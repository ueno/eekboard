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
#ifndef EEK_KEYSYM_H
#define EEK_KEYSYM_H 1

#include <glib-object.h>

/**
 * EEK_INVALID_KEYSYM:
 *
 * Pseudo keysym used for error reporting.
 */
#define EEK_INVALID_KEYSYM ((guint)(-1))

/**
 * EEK_INVALID_KEYCODE:
 *
 * Pseudo keycode used for error reporting.
 */
#define EEK_INVALID_KEYCODE ((guint)(-1))

/**
 * EekKeysymCategory:
 * @EEK_KEYSYM_CATEGORY_LETTER: the symbol represents an alphabet letter
 * @EEK_KEYSYM_CATEGORY_FUNCTION: the symbol represents a function
 * @EEK_KEYSYM_CATEGORY_KEYNAME: the symbol does not have meaning but
 * have a name
 * @EEK_KEYSYM_CATEGORY_UNKNOWN: used for error reporting
 *
 * Category of the key symbols.
 */
typedef enum {
    EEK_KEYSYM_CATEGORY_LETTER,
    EEK_KEYSYM_CATEGORY_FUNCTION,
    EEK_KEYSYM_CATEGORY_KEYNAME,
    EEK_KEYSYM_CATEGORY_UNKNOWN,
    /*< private >*/
    EEK_KEYSYM_CATEGORY_LAST = EEK_KEYSYM_CATEGORY_UNKNOWN
} EekKeysymCategory;

gchar *eek_keysym_to_string (guint keysym);

gchar *eek_xkeysym_to_string (guint xkeysym);
guint eek_xkeysym_from_string (gchar *string);

EekKeysymCategory eek_keysym_get_category (guint keysym);

typedef enum
{
  EEK_SHIFT_MASK    = 1 << 0,
  EEK_LOCK_MASK	    = 1 << 1,
  EEK_CONTROL_MASK  = 1 << 2,
  EEK_MOD1_MASK	    = 1 << 3,
  EEK_MOD2_MASK	    = 1 << 4,
  EEK_MOD3_MASK	    = 1 << 5,
  EEK_MOD4_MASK	    = 1 << 6,
  EEK_MOD5_MASK	    = 1 << 7,
  EEK_BUTTON1_MASK  = 1 << 8,
  EEK_BUTTON2_MASK  = 1 << 9,
  EEK_BUTTON3_MASK  = 1 << 10,
  EEK_BUTTON4_MASK  = 1 << 11,
  EEK_BUTTON5_MASK  = 1 << 12,

  /* The next few modifiers are used by XKB, so we skip to the end.
   * Bits 15 - 25 are currently unused. Bit 29 is used internally.
   */
  
  EEK_SUPER_MASK    = 1 << 26,
  EEK_HYPER_MASK    = 1 << 27,
  EEK_META_MASK     = 1 << 28,
  
  EEK_RELEASE_MASK  = 1 << 30,

  EEK_MODIFIER_MASK = 0x5c001fff
} EekModifierType;

#define EEK_KEY_Shift_L 0xffe1
#define EEK_KEY_Shift_R 0xffe2
#define EEK_KEY_ISO_Level3_Shift 0xfe03
#define EEK_KEY_Caps_Lock 0xffe5
#define EEK_KEY_Shift_Lock 0xffe6
#define EEK_KEY_Control_L 0xffe3
#define EEK_KEY_Control_R 0xffe4
#define EEK_KEY_Alt_L 0xffe9
#define EEK_KEY_Alt_R 0xffea
#define EEK_KEY_Meta_L 0xffe7
#define EEK_KEY_Meta_R 0xffe8
#define EEK_KEY_Super_L 0xffeb
#define EEK_KEY_Super_R 0xffec
#define EEK_KEY_Hyper_L 0xffed
#define EEK_KEY_Hyper_R 0xffee

EekModifierType eek_keysym_to_modifier (guint keysym);

/**
 * eek_keysym_is_modifier:
 * @keysym: keysym ID
 *
 * Check if @keysym is a modifier key.
 * Returns: %TRUE if @keysym is a modifier.
 */
#define eek_keysym_is_modifier(keysym) (eek_keysym_to_modifier ((keysym)) != 0)

#endif  /* EEK_KEYSYM_H */
