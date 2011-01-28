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

/**
 * SECTION:eek-keysym
 * @short_description: an #EekSymbol represents an X keysym
 */

#include <string.h>
#include <stdlib.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif  /* HAVE_CONFIG_H */

#include "eek-keysym.h"

/* modifier keys */
#define EEK_KEYSYM_Shift_L 0xffe1
#define EEK_KEYSYM_Shift_R 0xffe2
#define EEK_KEYSYM_ISO_Level3_Shift 0xfe03
#define EEK_KEYSYM_Caps_Lock 0xffe5
#define EEK_KEYSYM_Shift_Lock 0xffe6
#define EEK_KEYSYM_Control_L 0xffe3
#define EEK_KEYSYM_Control_R 0xffe4
#define EEK_KEYSYM_Alt_L 0xffe9
#define EEK_KEYSYM_Alt_R 0xffea
#define EEK_KEYSYM_Meta_L 0xffe7
#define EEK_KEYSYM_Meta_R 0xffe8
#define EEK_KEYSYM_Super_L 0xffeb
#define EEK_KEYSYM_Super_R 0xffec
#define EEK_KEYSYM_Hyper_L 0xffed
#define EEK_KEYSYM_Hyper_R 0xffee

struct _EekKeysymPrivate {
    guint xkeysym;
    const gchar *name;
    EekSymbolCategory category;
};

typedef EekKeysymPrivate EekKeysymEntry;

#include "eek-special-keysym-entries.h"
#include "eek-unicode-keysym-entries.h"
#include "eek-xkeysym-keysym-entries.h"

static const EekKeysymEntry invalid_keysym_entry = {
    EEK_INVALID_KEYSYM,
    "\0",
    EEK_SYMBOL_CATEGORY_UNKNOWN,
};

G_DEFINE_TYPE (EekKeysym, eek_keysym, EEK_TYPE_SYMBOL);

#define EEK_KEYSYM_GET_PRIVATE(obj)                                  \
    (G_TYPE_INSTANCE_GET_PRIVATE ((obj), EEK_TYPE_KEYSYM, EekKeysymPrivate))

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
keysym_entry_compare_by_xkeysym (const void *key0, const void *key1)
{
    const EekKeysymEntry *entry0 = key0, *entry1 = key1;
    return (gint)entry0->xkeysym - (gint)entry1->xkeysym;
}

static EekKeysymEntry *
find_keysym_entry_by_xkeysym (guint xkeysym,
                              const EekKeysymEntry *entries,
                              gint num_entries)
{
    EekKeysymEntry key;

    key.xkeysym = xkeysym;
    return bsearch (&key, entries, num_entries, sizeof (EekKeysymEntry),
                   keysym_entry_compare_by_xkeysym);
}

static G_CONST_RETURN gchar *
eek_keysym_real_get_name (EekSymbol *self)
{
    EekKeysymPrivate *priv = EEK_KEYSYM_GET_PRIVATE(self);
    return priv->name;
}

static gchar *
eek_keysym_real_get_label (EekSymbol *self)
{
    EekKeysymPrivate *priv = EEK_KEYSYM_GET_PRIVATE(self);
    EekKeysymEntry *entry;

    /* First, search special keysyms. */
    entry = find_keysym_entry_by_xkeysym (priv->xkeysym,
                                          special_keysym_entries,
                                          G_N_ELEMENTS(special_keysym_entries));
    if (entry)
        return g_strdup (entry->name);
  
    /* Check for Latin-1 characters (1:1 mapping) */
    if ((priv->xkeysym >= 0x0020 && priv->xkeysym <= 0x007e) ||
        (priv->xkeysym >= 0x00a0 && priv->xkeysym <= 0x00ff))
        return unichar_to_utf8 (priv->xkeysym);

    /* Also check for directly encoded 24-bit UCS characters:
     */
    if ((priv->xkeysym & 0xff000000) == 0x01000000)
        return unichar_to_utf8 (priv->xkeysym & 0x00ffffff);

    /* Search known unicode keysyms. */
    entry = find_keysym_entry_by_xkeysym (priv->xkeysym,
                                          unicode_keysym_entries,
                                          G_N_ELEMENTS(unicode_keysym_entries));
    if (entry)
        return g_strdup (entry->name);

    return g_strdup (eek_symbol_get_name (self));
}

EekSymbolCategory
eek_keysym_real_get_category (EekSymbol *self)
{
    EekKeysymPrivate *priv = EEK_KEYSYM_GET_PRIVATE(self);
    return priv->category;
}

EekModifierType
eek_keysym_real_get_modifier_mask (EekSymbol *self)
{
    EekKeysymPrivate *priv = EEK_KEYSYM_GET_PRIVATE(self);

    switch (priv->xkeysym) {
    case EEK_KEYSYM_Shift_L:
    case EEK_KEYSYM_Shift_R:
    case EEK_KEYSYM_Caps_Lock:
    case EEK_KEYSYM_Shift_Lock:
        return EEK_SHIFT_MASK;
    case EEK_KEYSYM_ISO_Level3_Shift:
        return EEK_MOD5_MASK;
    case EEK_KEYSYM_Control_L:
    case EEK_KEYSYM_Control_R:
        return EEK_CONTROL_MASK;
    case EEK_KEYSYM_Alt_L:
    case EEK_KEYSYM_Alt_R:
        return EEK_MOD1_MASK;
    case EEK_KEYSYM_Meta_L:
    case EEK_KEYSYM_Meta_R:
        return EEK_META_MASK;
    case EEK_KEYSYM_Super_L:
    case EEK_KEYSYM_Super_R:
        return EEK_SUPER_MASK;
    case EEK_KEYSYM_Hyper_L:
    case EEK_KEYSYM_Hyper_R:
        return EEK_HYPER_MASK;
    }
    return 0;
}

static void
eek_keysym_class_init (EekKeysymClass *klass)
{
    EekSymbolClass *symbol_class = EEK_SYMBOL_CLASS (klass);
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    g_type_class_add_private (gobject_class, sizeof (EekKeysymPrivate));

    symbol_class->get_name = eek_keysym_real_get_name;
    symbol_class->get_label = eek_keysym_real_get_label;
    symbol_class->get_category = eek_keysym_real_get_category;
    symbol_class->get_modifier_mask = eek_keysym_real_get_modifier_mask;
}

static void
eek_keysym_init (EekKeysym *self)
{
    EekKeysymPrivate *priv;

    priv = self->priv = EEK_KEYSYM_GET_PRIVATE(self);
    memcpy (priv, &invalid_keysym_entry, sizeof (EekKeysymEntry));
}

EekKeysym *
eek_keysym_new (guint xkeysym)
{
    EekKeysym *keysym;
    EekKeysymPrivate *priv;
    EekKeysymEntry *entry;

    keysym = g_object_new (EEK_TYPE_KEYSYM, NULL);
    priv = EEK_KEYSYM_GET_PRIVATE(keysym);

    entry = find_keysym_entry_by_xkeysym (xkeysym,
                                          xkeysym_keysym_entries,
                                          G_N_ELEMENTS(xkeysym_keysym_entries));
    if (entry)
        memcpy (priv, entry, sizeof (EekKeysymEntry));
    else {
        // g_warning ("can't find keysym entry %u", xkeysym);
        memcpy (priv, &invalid_keysym_entry, sizeof (EekKeysymEntry));
        priv->xkeysym = xkeysym;
    }
    return keysym;
}

EekKeysym *
eek_keysym_new_from_name (const gchar *name)
{
    EekKeysym *keysym;
    EekKeysymPrivate *priv;
    gint i;

    for (i = 0;
         i < G_N_ELEMENTS(xkeysym_keysym_entries) &&
             g_strcmp0 (xkeysym_keysym_entries[i].name, name) != 0; i++)
        ;

    keysym = g_object_new (EEK_TYPE_KEYSYM, NULL);
    priv = EEK_KEYSYM_GET_PRIVATE(keysym);

    if (i < G_N_ELEMENTS(xkeysym_keysym_entries))
        memcpy (priv, &xkeysym_keysym_entries[i], sizeof (EekKeysymEntry));
    else {
        // g_warning ("can't find keysym entry for %s", name);
        memcpy (priv, &invalid_keysym_entry, sizeof (EekKeysymEntry));
    }
    return keysym;
}

guint
eek_keysym_get_xkeysym (EekKeysym *keysym)
{
    EekKeysymPrivate *priv;

    g_assert (EEK_IS_KEYSYM(keysym));
    priv = EEK_KEYSYM_GET_PRIVATE(keysym);
    return priv->xkeysym;
}
