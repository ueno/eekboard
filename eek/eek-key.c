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
 * SECTION:eek-key
 * @short_description: Base interface of a key
 * @see_also: #EekSection
 *
 * The #EekKeyIface interface represents a key, which is parented to
 * #EekSectionIface.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif  /* HAVE_CONFIG_H */

#include "eek-key.h"
#include "eek-keysym.h"

static void
eek_key_base_init (gpointer g_iface)
{
    static gboolean is_initialized = FALSE;

    if (!is_initialized) {
        GParamSpec *pspec;

        /**
         * EekKey:name:
         *
         * The name of #EekKey.
         */
        pspec = g_param_spec_string ("name",
                                     "Name",
                                     "Name",
                                     NULL,
                                     G_PARAM_READWRITE);
        g_object_interface_install_property (g_iface, pspec);

        /**
         * EekKey:keycode:
         *
         * The keycode of #EekKey.
         */
        pspec = g_param_spec_uint ("keycode",
                                   "Keycode",
                                   "Keycode of the key",
                                   0, G_MAXUINT, 0,
                                   G_PARAM_READWRITE);
        g_object_interface_install_property (g_iface, pspec);

        /**
         * EekKey:keysyms:
         *
         * The symbol matrix of #EekKey.
         */
        pspec = g_param_spec_boxed ("keysyms",
                                    "Keysyms",
                                    "Symbol matrix of the key",
                                    EEK_TYPE_KEYSYM_MATRIX,
                                    G_PARAM_READWRITE);
        g_object_interface_install_property (g_iface, pspec);

        /**
         * EekKey:column:
         *
         * The column index of #EekKey in the parent #EekSection.
         */
        pspec = g_param_spec_int ("column",
                                  "Column",
                                  "Column index of the key in section",
                                  -1, G_MAXINT, -1,
                                  G_PARAM_READWRITE);
        g_object_interface_install_property (g_iface, pspec);

        /**
         * EekKey:row:
         *
         * The row index of #EekKey in the parent #EekSection.
         */
        pspec = g_param_spec_int ("row",
                                  "Row",
                                  "Row index of the key in section",
                                  -1, G_MAXINT, -1,
                                  G_PARAM_READWRITE);
        g_object_interface_install_property (g_iface, pspec);

        /**
         * EekKey:outline:
         *
         * The pointer to the outline shape of #EekKey.
         */
        /* Use pointer instead of boxed to avoid copy, since we can
           assume that only a few outline shapes are used in a whole
           keyboard (unlike keysyms and bounds). */
        pspec = g_param_spec_pointer ("outline",
                                      "Outline",
                                      "Pointer to outline shape of the key",
                                      G_PARAM_READWRITE);
        g_object_interface_install_property (g_iface, pspec);

        /**
         * EekKey:bounds:
         *
         * The bounding box of #EekKey.
         */
        pspec = g_param_spec_boxed ("bounds",
                                    "Bounds",
                                    "Bounding box of the key",
                                    EEK_TYPE_BOUNDS,
                                    G_PARAM_READWRITE);
        g_object_interface_install_property (g_iface, pspec);

        /**
         * EekKey:group:
         *
         * The column index of #EekKey in the symbol matrix #EekKey:keysyms.
         */
        pspec = g_param_spec_int ("group",
                                  "Group",
                                  "Current group of the key",
                                  0, 64, 0,
                                  G_PARAM_READWRITE);
        g_object_interface_install_property (g_iface, pspec);

        /**
         * EekKey:level:
         *
         * The row index of #EekKey in the symbol matrix #EekKey:keysyms.
         */
        pspec = g_param_spec_int ("level",
                                  "Level",
                                  "Current level of the key",
                                  0, 3, 0,
                                  G_PARAM_READWRITE);
        g_object_interface_install_property (g_iface, pspec);

        is_initialized = TRUE;
    }
}

GType
eek_key_get_type (void)
{
    static GType iface_type = 0;

    if (iface_type == 0) {
        static const GTypeInfo info = {
            sizeof (EekKeyIface),
            eek_key_base_init,  /* iface_base_init */
            NULL                /* iface_base_finalize */
        };

        iface_type = g_type_register_static (G_TYPE_INTERFACE,
                                             "EekKey",
                                             &info,
                                             0);
    }
    return iface_type;
}

/**
 * eek_key_get_keycode:
 * @key: an #EekKey
 *
 * Get the keycode of @key.
 */
guint
eek_key_get_keycode (EekKey *key)
{
    EekKeyIface *iface = EEK_KEY_GET_IFACE(key);

    g_return_val_if_fail (iface, EEK_INVALID_KEYCODE);
    g_return_val_if_fail (iface->get_keycode, EEK_INVALID_KEYCODE);
    return (*iface->get_keycode) (key);
}

/**
 * eek_key_set_keysyms:
 * @key: an #EekKey
 * @keysyms: symbol matrix of @key
 * @num_groups: the number of groups (rows) of @keysyms
 * @num_levels: the number of levels (columns) of @keysyms
 *
 * Set the symbol matrix of @key to @keysyms. @keysyms is an array of
 * symbols (unsigned int) and the length must match with @num_groups *
 * @num_levels.
 */
void
eek_key_set_keysyms (EekKey *key,
                     guint  *keysyms,
                     gint    num_groups,
                     gint    num_levels)
{
    EekKeyIface *iface = EEK_KEY_GET_IFACE(key);

    g_return_if_fail (iface);
    g_return_if_fail (iface->set_keysyms);
    (*iface->set_keysyms) (key, keysyms, num_groups, num_levels);
}

/**
 * eek_key_get_keysyms:
 * @key: an #EekKey
 * @keysyms: pointer where symbol matrix of @key will be stored
 * @num_groups: pointer where the number of groups (rows) of @keysyms
 * will be stored
 * @num_levels: pointer where the number of levels (columns) of
 * @keysyms will be stored
 *
 * Get the symbol matrix of @key to @keysyms. @keysyms is an array of
 * symbols (unsigned int) and the length must match with @num_groups *
 * @num_levels.
 */
void
eek_key_get_keysyms (EekKey *key,
                     guint **keysyms,
                     gint   *num_groups,
                     gint   *num_levels)
{
    EekKeyIface *iface = EEK_KEY_GET_IFACE(key);

    g_return_if_fail (iface);
    g_return_if_fail (iface->get_keysyms);
    (*iface->get_keysyms) (key, keysyms, num_groups, num_levels);
}

/**
 * eek_key_get_groups:
 * @key: an #EekKey
 *
 * Get the number of groups (rows) of the symbol matrix of @key.
 */
gint
eek_key_get_groups  (EekKey *key)
{
    EekKeyIface *iface = EEK_KEY_GET_IFACE(key);

    g_return_val_if_fail (iface, -1);
    g_return_val_if_fail (iface->get_groups, -1);
    return (*iface->get_groups) (key);
}

/**
 * eek_key_get_keysym:
 * @key: an #EekKey
 *
 * Get the current symbol of @key.  It is depend on the current group
 * and level, and the symbol matrix of @key.  They are set through
 * eek_key_set_keysym_index() and eek_key_set_keysyms(), respectively.
 */
guint
eek_key_get_keysym (EekKey *key)
{
    EekKeyIface *iface = EEK_KEY_GET_IFACE(key);

    g_return_val_if_fail (iface, EEK_INVALID_KEYSYM);
    g_return_val_if_fail (iface->get_keysym, EEK_INVALID_KEYSYM);
    return (*iface->get_keysym) (key);
}

/**
 * eek_key_set_index:
 * @key: an #EekKey
 * @column: column index in the section
 * @row: row index in the section
 *
 * Set column and row index of @key in the parent section.
 */
void
eek_key_set_index  (EekKey *key,
                    gint    column,
                    gint    row)
{
    EekKeyIface *iface = EEK_KEY_GET_IFACE(key);

    g_return_if_fail (iface);
    g_return_if_fail (iface->set_index);
    (*iface->set_index) (key, column, row);
}

/**
 * eek_key_get_index:
 * @key: an #EekKey
 * @column: a pointer to where column index in the section is stored
 * @row: a pointer to where row index in the section is stored
 *
 * Get column and row index of @key in the parent section.
 */
void
eek_key_get_index  (EekKey *key,
                    gint   *column,
                    gint   *row)
{
    EekKeyIface *iface = EEK_KEY_GET_IFACE(key);

    g_return_if_fail (iface);
    g_return_if_fail (iface->get_index);
    return (*iface->get_index) (key, column, row);
}

/**
 * eek_key_set_outline:
 * @key: an #EekKey
 * @outline: a pointer to the outline shape of @key
 *
 * Set outline shape of @key.
 */
void
eek_key_set_outline (EekKey     *key,
                     EekOutline *outline)
{
    EekKeyIface *iface = EEK_KEY_GET_IFACE(key);

    g_return_if_fail (iface);
    g_return_if_fail (iface->set_outline);
    (*iface->set_outline) (key, outline);
}

/**
 * eek_key_get_outline:
 * @key: an #EekKey
 *
 * Get outline shape of @key as a pointer.
 */
EekOutline *
eek_key_get_outline (EekKey *key)
{
    EekKeyIface *iface = EEK_KEY_GET_IFACE(key);

    g_return_val_if_fail (iface, NULL);
    g_return_val_if_fail (iface->get_outline, NULL);
    return (*iface->get_outline) (key);
}

/**
 * eek_key_set_bounds:
 * @key: an #EekKey
 * @bounds: bounding box of @key
 *
 * Set the bounding box of @key to @bounds.
 */
void
eek_key_set_bounds (EekKey    *key,
                    EekBounds *bounds)
{
    EekKeyIface *iface = EEK_KEY_GET_IFACE(key);

    g_return_if_fail (iface);
    g_return_if_fail (iface->set_bounds);
    (*iface->set_bounds) (key, bounds);
}

/**
 * eek_key_get_bounds:
 * @key: an #EekKey
 * @bounds: the bounding box of @key
 *
 * Get the bounding box of @key.
 */
void
eek_key_get_bounds (EekKey    *key,
                    EekBounds *bounds)
{
    EekKeyIface *iface = EEK_KEY_GET_IFACE(key);

    g_return_if_fail (iface);
    g_return_if_fail (iface->get_bounds);
    return (*iface->get_bounds) (key, bounds);
}

/**
 * eek_key_set_keysym_index:
 * @key: an #EekKey
 * @group: row index of the symbol matrix #EekKey:keysyms
 * @level: column index of the symbol matrix #EekKey:keysyms
 *
 * Select a cell of the symbol matrix of @key.
 */
void
eek_key_set_keysym_index (EekKey *key,
                         gint    group,
                         gint    level)
{
    EekKeyIface *iface = EEK_KEY_GET_IFACE(key);

    g_return_if_fail (iface);
    g_return_if_fail (iface->set_keysym_index);
    (*iface->set_keysym_index) (key, group, level);
}

/**
 * eek_key_get_keysym_index:
 * @key: an #EekKey
 * @group: a pointer where row index of the symbol matrix #EekKey:keysyms
 * will be stored
 * @level: a pointer where column index of the symbol matrix
 * #EekKey:keysyms will be stored
 *
 * Get the current cell position of the symbol matrix of @key.
 */
void
eek_key_get_keysym_index (EekKey *key, gint *column, gint *row)
{
    EekKeyIface *iface = EEK_KEY_GET_IFACE(key);

    g_return_if_fail (iface);
    g_return_if_fail (iface->get_keysym_index);
    return (*iface->get_keysym_index) (key, column, row);
}
