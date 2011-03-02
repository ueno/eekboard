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
#include "eek-serializable.h"

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
};

struct _EekKeysymEntry {
    guint xkeysym;
    const gchar *name;
    EekSymbolCategory category;
};

typedef struct _EekKeysymEntry EekKeysymEntry;

#include "eek-special-keysym-entries.h"
#include "eek-unicode-keysym-entries.h"
#include "eek-xkeysym-keysym-entries.h"

static void eek_serializable_iface_init (EekSerializableIface *iface);

G_DEFINE_TYPE_WITH_CODE (EekKeysym, eek_keysym, EEK_TYPE_SYMBOL,
                         G_IMPLEMENT_INTERFACE (EEK_TYPE_SERIALIZABLE,
                                                eek_serializable_iface_init));

#define EEK_KEYSYM_GET_PRIVATE(obj)                                  \
    (G_TYPE_INSTANCE_GET_PRIVATE ((obj), EEK_TYPE_KEYSYM, EekKeysymPrivate))

static EekSerializableIface *eek_keysym_parent_serializable_iface;

static void
eek_keysym_real_serialize (EekSerializable *self,
                           GVariantBuilder *builder)
{
    EekKeysymPrivate *priv = EEK_KEYSYM_GET_PRIVATE(self);

    eek_keysym_parent_serializable_iface->serialize (self, builder);

    g_variant_builder_add (builder, "u", priv->xkeysym);
}

static gsize
eek_keysym_real_deserialize (EekSerializable *self,
                             GVariant        *variant,
                             gsize            index)
{
    EekKeysymPrivate *priv = EEK_KEYSYM_GET_PRIVATE(self);

    index = eek_keysym_parent_serializable_iface->deserialize (self,
                                                               variant,
                                                               index);

    g_variant_get_child (variant, index++, "u", &priv->xkeysym);

    return index;
}

static void
eek_serializable_iface_init (EekSerializableIface *iface)
{
    eek_keysym_parent_serializable_iface =
        g_type_interface_peek_parent (iface);

    iface->serialize = eek_keysym_real_serialize;
    iface->deserialize = eek_keysym_real_deserialize;
}

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
    return (gint) (entry0->xkeysym - entry1->xkeysym);
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

static gboolean
get_unichar (guint xkeysym, gunichar *uc) {
    /* Check for Latin-1 characters (1:1 mapping) */
    if ((xkeysym >= 0x0020 && xkeysym <= 0x007e) ||
        (xkeysym >= 0x00a0 && xkeysym <= 0x00ff)) {
        *uc = xkeysym;
        return TRUE;
    }

    /* Also check for directly encoded 24-bit UCS characters:
     */
    if ((xkeysym & 0xff000000) == 0x01000000) {
        *uc = xkeysym & 0x00ffffff;
        return TRUE;
    }

    return FALSE;
}

G_INLINE_FUNC EekModifierType
get_modifier_mask (guint xkeysym)
{
    switch (xkeysym) {
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
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    g_type_class_add_private (gobject_class, sizeof (EekKeysymPrivate));
}

static void
eek_keysym_init (EekKeysym *self)
{
    EekKeysymPrivate *priv;

    priv = self->priv = EEK_KEYSYM_GET_PRIVATE(self);
    priv->xkeysym = EEK_INVALID_KEYSYM;
}

EekKeysym *
eek_keysym_new_with_modifier (guint xkeysym, EekModifierType modifier_mask)
{
    EekKeysym *keysym;
    EekKeysymPrivate *priv;
    EekKeysymEntry *special_entry, *xkeysym_entry, *unicode_entry,
        *unichar_entry;
    gchar *name, *label;
    EekSymbolCategory category;
    gunichar uc;

    special_entry =
        find_keysym_entry_by_xkeysym (xkeysym,
                                      special_keysym_entries,
                                      G_N_ELEMENTS(special_keysym_entries));
    xkeysym_entry =
        find_keysym_entry_by_xkeysym (xkeysym,
                                      xkeysym_keysym_entries,
                                      G_N_ELEMENTS(xkeysym_keysym_entries));
    unicode_entry =
        find_keysym_entry_by_xkeysym (xkeysym,
                                      unicode_keysym_entries,
                                      G_N_ELEMENTS(unicode_keysym_entries));
    unichar_entry = NULL;
    if (get_unichar (xkeysym, &uc)) {
        unichar_entry = g_slice_new (EekKeysymEntry);
        unichar_entry->xkeysym = xkeysym;
        unichar_entry->name = unichar_to_utf8 (uc);
        unichar_entry->category = EEK_SYMBOL_CATEGORY_LETTER;
    }

    /* name and category */
    name = NULL;
    if (xkeysym_entry) {
        name = g_strdup (xkeysym_entry->name);
        category = xkeysym_entry->category;
    } else if (unichar_entry) {
        name = g_strdup (unichar_entry->name);
        category = unichar_entry->category;
    } else if (unicode_entry) {
        name = g_strdup (unicode_entry->name);
        category = unicode_entry->category;
    } else {
        name = g_strdup ("");
        category = EEK_SYMBOL_CATEGORY_UNKNOWN;
    }

    /* label */
    if (special_entry)
        label = g_strdup (special_entry->name);
    else if (unichar_entry)
        label = g_strdup (unichar_entry->name);
    else if (unicode_entry)
        label = g_strdup (unicode_entry->name);
    else
        label = g_strdup (name);

    keysym = g_object_new (EEK_TYPE_KEYSYM,
                           "name", name,
                           "label", label,
                           "category", category,
                           "modifier-mask", modifier_mask,
                           NULL);
    g_free (name);
    g_free (label);

    if (unichar_entry) {
        g_free ((gpointer) unichar_entry->name);
        g_slice_free (EekKeysymEntry, unichar_entry);
    }

    priv = EEK_KEYSYM_GET_PRIVATE(keysym);
    priv->xkeysym = xkeysym;

    return keysym;
}

EekKeysym *
eek_keysym_new (guint xkeysym)
{
    return eek_keysym_new_with_modifier (xkeysym, get_modifier_mask (xkeysym));
}

EekKeysym *
eek_keysym_new_from_name (const gchar *name)
{
    gint i;

    for (i = 0; i < G_N_ELEMENTS(xkeysym_keysym_entries); i++)
        if (g_strcmp0 (xkeysym_keysym_entries[i].name, name) == 0)
            return eek_keysym_new (xkeysym_keysym_entries[i].xkeysym);

    // g_warning ("can't find keysym entry for %s", name);
    return g_object_new (EEK_TYPE_KEYSYM,
                         "name", name,
                         "label", name,
                         "category", EEK_SYMBOL_CATEGORY_UNKNOWN,
                         "modifier-mask", 0,
                         NULL);
}

guint
eek_keysym_get_xkeysym (EekKeysym *keysym)
{
    EekKeysymPrivate *priv;

    g_assert (EEK_IS_KEYSYM(keysym));
    priv = EEK_KEYSYM_GET_PRIVATE(keysym);
    return priv->xkeysym;
}
