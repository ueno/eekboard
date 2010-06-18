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
    EEK_KEYSYM_CATEGORY_LAST = EEK_KEYSYM_CATEGORY_UNKNOWN
} EekKeysymCategory;

G_CONST_RETURN gchar *eek_keysym_to_string (guint keysym);
EekKeysymCategory eek_keysym_get_category (guint keysym);

#endif  /* EEK_KEYSYM_H */
