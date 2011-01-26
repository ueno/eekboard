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

/**
 * SECTION:eek-keysym
 * @short_description: Keysym conversion utilities
 */

#include <glib.h>
#include <stdlib.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif  /* HAVE_CONFIG_H */
#include "eek-keysym.h"

struct eek_keysym_label {
    guint keysym;
    const gchar *label;
    EekKeysymCategory category;
};

#include "eek-special-keysym-labels.h"
#include "eek-unicode-keysym-labels.h"
#include "eek-keyname-keysym-labels.h"

static gchar *
unichar_to_utf8 (gunichar uc)
{
    if (g_unichar_isgraph (uc)) {
        gchar *buf;
        gint len;

        len = g_unichar_to_utf8 (uc, NULL);
        buf = g_malloc0 (len + 1);
        g_unichar_to_utf8 (uc, buf);
        return buf;
    }
    return g_strdup ("");
}

static int
keysym_label_compare (const void *key0, const void *key1)
{
    const struct eek_keysym_label *entry0 = key0, *entry1 = key1;
    return (gint)entry0->keysym - (gint)entry1->keysym;
}

static gboolean
find_keysym (guint              keysym,
             gchar            **label,
             EekKeysymCategory *category)
{
    struct eek_keysym_label bsearch_key, *bsearch_val;

    /* First, search special keysyms. */
    bsearch_key.keysym = keysym;
    bsearch_val = bsearch (&bsearch_key,
                           special_keysym_labels,
                           G_N_ELEMENTS(special_keysym_labels),
                           sizeof (struct eek_keysym_label),
                           keysym_label_compare);
    if (bsearch_val) {
        if (label)
            *label = g_strdup (bsearch_val->label);
        if (category)
            *category = bsearch_val->category;
        return TRUE;
    }
  
    /* Check for Latin-1 characters (1:1 mapping) */
    if ((keysym >= 0x0020 && keysym <= 0x007e) ||
        (keysym >= 0x00a0 && keysym <= 0x00ff)) {
        if (label)
            *label = unichar_to_utf8 (keysym);
        if (category)
            *category = EEK_KEYSYM_CATEGORY_LETTER;
        return TRUE;
    }

    /* Also check for directly encoded 24-bit UCS characters:
     */
    if ((keysym & 0xff000000) == 0x01000000) {
        if (label)
            *label = unichar_to_utf8 (keysym & 0x00ffffff);
        if (category)
            *category = EEK_KEYSYM_CATEGORY_LETTER;
        return TRUE;
    }

    /* Search known unicode keysyms. */
    bsearch_key.keysym = keysym;
    bsearch_val = bsearch (&bsearch_key,
                           unicode_keysym_labels,
                           G_N_ELEMENTS(unicode_keysym_labels),
                           sizeof (struct eek_keysym_label),
                           keysym_label_compare);
    if (bsearch_val) {
        if (label)
            *label = g_strdup (bsearch_val->label);
        if (category)
            *category = bsearch_val->category;
        return TRUE;
    }

    /* Finally, search keynames. */
    bsearch_key.keysym = keysym;
    bsearch_val = bsearch (&bsearch_key,
                           keyname_keysym_labels,
                           G_N_ELEMENTS(keyname_keysym_labels),
                           sizeof (struct eek_keysym_label),
                           keysym_label_compare);
    if (bsearch_val) {
        if (label)
            *label = g_strdup (bsearch_val->label);
        if (category)
            *category = bsearch_val->category;
        return TRUE;
    }

    return FALSE;
}

/**
 * eek_keysym_to_string:
 * @keysym: keysym ID
 *
 * Return a string representation of @keysym.
 */
gchar *
eek_keysym_to_string (guint keysym)
{
    gchar *label;

    if (find_keysym (keysym, &label, NULL))
        return label;
    return g_strdup ("");
}

/**
 * eek_keysym_get_category:
 * @keysym: keysym ID
 *
 * Return a string representation of @keysym.
 */
EekKeysymCategory
eek_keysym_get_category (guint keysym)
{
    EekKeysymCategory category;

    if (find_keysym (keysym, NULL, &category))
        return category;
    return EEK_KEYSYM_CATEGORY_UNKNOWN;
}

/**
 * eek_keysym_to_modifier:
 * @keysym: keysym ID
 *
 * Return a modifier mask which @keysym affects.
 */
EekModifierType
eek_keysym_to_modifier (guint keysym)
{
    switch (keysym) {
    case EEK_KEY_Shift_L:
    case EEK_KEY_Shift_R:
    case EEK_KEY_Caps_Lock:
    case EEK_KEY_Shift_Lock:
        return EEK_SHIFT_MASK;
    case EEK_KEY_ISO_Level3_Shift:
        return EEK_MOD5_MASK;
    case EEK_KEY_Control_L:
    case EEK_KEY_Control_R:
        return EEK_CONTROL_MASK;
    case EEK_KEY_Alt_L:
    case EEK_KEY_Alt_R:
        return EEK_MOD1_MASK;
    case EEK_KEY_Meta_L:
    case EEK_KEY_Meta_R:
        return EEK_META_MASK;
    case EEK_KEY_Super_L:
    case EEK_KEY_Super_R:
        return EEK_SUPER_MASK;
    case EEK_KEY_Hyper_L:
    case EEK_KEY_Hyper_R:
        return EEK_HYPER_MASK;
    }
    return 0;
}
