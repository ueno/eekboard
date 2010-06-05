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
 * SECTION:eek-section
 * @short_description: Base interface of a keyboard section
 * @see_also: #EekKeyboard, #EekKey
 *
 * The #EekSectionIface interface represents a keyboard section, which
 * is parented to #EekKeyboardIface and can have one or more keys of
 * the #EekKeyIface interface.
 *
 * #EekSectionIface follows the Builder pattern and is responsible for
 * creation of keys.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif  /* HAVE_CONFIG_H */

#include "eek-section.h"

static void
eek_section_base_init (gpointer g_iface)
{
    static gboolean is_initialized = FALSE;

    if (!is_initialized) {
        GParamSpec *pspec;

        /**
         * EekSection:name:
         *
         * The name of #EekSection.
         */
        pspec = g_param_spec_string ("name",
                                     "Name",
                                     "Name",
                                     NULL,
                                     G_PARAM_READWRITE);
        g_object_interface_install_property (g_iface, pspec);

        /**
         * EekSection:columns:
         *
         * The number of columns in #EekSection.
         */
        pspec = g_param_spec_int ("columns",
                                  "Columns",
                                  "The number of columns in the section",
                                  0, G_MAXINT, 0,
                                  G_PARAM_READWRITE);
        g_object_interface_install_property (g_iface, pspec);

        /**
         * EekSection:rows:
         *
         * The number of rows in #EekSection.
         */
        pspec = g_param_spec_int ("rows",
                                  "Rows",
                                  "The number of rows of the section",
                                  0, G_MAXINT, 0,
                                  G_PARAM_READWRITE);
        g_object_interface_install_property (g_iface, pspec);

        /**
         * EekSection:angle:
         *
         * The rotation angle of #EekSection.
         */
        pspec = g_param_spec_int ("angle",
                                  "Angle",
                                  "Rotation angle of the section",
                                  -360, 360, 0,
                                  G_PARAM_READWRITE);
        g_object_interface_install_property (g_iface, pspec);

        /**
         * EekSection:bounds:
         *
         * The bounding box of #EekSection.
         */
        pspec = g_param_spec_boxed ("bounds",
                                    "Bounds",
                                    "Bounding box of the section",
                                    EEK_TYPE_BOUNDS,
                                    G_PARAM_READWRITE);
        g_object_interface_install_property (g_iface, pspec);

        is_initialized = TRUE;
    }
}

GType
eek_section_get_type (void)
{
    static GType iface_type = 0;

    if (iface_type == 0) {
        static const GTypeInfo info = {
            sizeof (EekSectionIface),
            eek_section_base_init,
            NULL
        };

        iface_type = g_type_register_static (G_TYPE_INTERFACE,
                                             "EekSection",
                                             &info,
                                             0);
    }
    return iface_type;
}

/**
 * eek_section_set_rows:
 * @section: an #EekSection
 * @rows: the number of rows in @section
 *
 * Set the number of rows in @section to @rows.
 */
void
eek_section_set_rows (EekSection *section,
                      gint        rows)
{
    EekSectionIface *iface = EEK_SECTION_GET_IFACE(section);

    g_return_if_fail (iface->set_rows);
    (*iface->set_rows) (section, rows);
}

/**
 * eek_section_get_rows:
 * @section: an #EekSection
 *
 * Get the number of rows in @section.
 */
gint
eek_section_get_rows (EekSection *section)
{
    EekSectionIface *iface = EEK_SECTION_GET_IFACE(section);

    g_return_if_fail (iface->get_rows);
    return (*iface->get_rows) (section);
}

/**
 * eek_section_set_columns:
 * @section: an #EekSection
 * @row: the row index in @section
 * @columns: the number of keys on @row
 *
 * Set the number of keys on @row.
 */
void
eek_section_set_columns (EekSection *section,
                         gint        row,
                         gint        columns)
{
    EekSectionIface *iface = EEK_SECTION_GET_IFACE(section);

    g_return_if_fail (iface->set_columns);
    (*iface->set_columns) (section, row, columns);
}

/**
 * eek_section_get_columns:
 * @section: an #EekSection
 * @row: the row index in @section
 *
 * Get the number of keys on @row.
 */
gint
eek_section_get_columns (EekSection *section,
                         gint        row)
{
    EekSectionIface *iface = EEK_SECTION_GET_IFACE(section);

    g_return_if_fail (iface->get_columns);
    return (*iface->get_columns) (section, row);
}

/**
 * eek_section_set_angle:
 * @section: an #EekSection
 * @angle: rotation angle of @section
 *
 * Set the rotation angle of @section to @angle.
 */
void
eek_section_set_angle (EekSection *section,
                       gint        angle)
{
    EekSectionIface *iface = EEK_SECTION_GET_IFACE(section);

    g_return_if_fail (iface->set_angle);
    (*iface->set_angle) (section, angle);
}

/**
 * eek_section_get_angle:
 * @section: an #EekSection
 *
 * Get the rotation angle of @section.
 */
gint
eek_section_get_angle (EekSection *section)
{
    EekSectionIface *iface = EEK_SECTION_GET_IFACE(section);

    g_return_val_if_fail (iface->get_angle, 0);
    return (*iface->get_angle) (section);
}

/**
 * eek_section_set_bounds:
 * @section: an #EekSection
 * @bounds: bounding box of @section
 *
 * Set the bounding box of @section to @bounds.
 */
void
eek_section_set_bounds (EekSection *section,
                        EekBounds  *bounds)
{
    EekSectionIface *iface = EEK_SECTION_GET_IFACE(section);

    g_return_if_fail (iface->set_bounds);
    (*iface->set_bounds) (section, bounds);
}

/**
 * eek_section_get_bounds:
 * @section: an #EekSection
 * @bounds: the bounding box of @section
 *
 * Get the bounding box of @section.
 */
void
eek_section_get_bounds (EekSection *section,
                        EekBounds  *bounds)
{
    EekSectionIface *iface = EEK_SECTION_GET_IFACE(section);

    g_return_if_fail (iface->get_bounds);
    return (*iface->get_bounds) (section, bounds);
}

/**
 * eek_section_create_key:
 * @section: an #EekSection
 * @name: name of the key
 * @keysyms: symbol matrix of the key
 * @num_groups: number of rows in the @keysyms
 * @num_levels: number of columns in the @keysyms
 * @column: column index in the @section
 * @row: row index in the section
 * @outline: outline shape of the key
 * @bounds: bounding box of the key
 *
 * Create an #EekKey instance and attach it to @section.
 */
EekKey *
eek_section_create_key (EekSection  *section,
                        const gchar *name,
                        guint       *keysyms,
                        gint         num_groups,
                        gint         num_levels,
                        gint         column,
                        gint         row,
                        EekOutline  *outline,
                        EekBounds   *bounds)
{
    EekSectionIface *iface;

    g_return_if_fail (EEK_IS_SECTION(section));

    iface = EEK_SECTION_GET_IFACE(section);
    g_return_if_fail (iface->create_key);

    return (*iface->create_key) (section,
                                 name,
                                 keysyms,
                                 num_groups,
                                 num_levels,
                                 column,
                                 row,
                                 outline,
                                 bounds);
}

/**
 * eek_section_foreach_key:
 * @section: an #EekSection
 * @func: a callback of #GFunc
 * @user_data: a pointer to an object passed to @func
 *
 * Iterate over @section's children list.
 */
void
eek_section_foreach_key (EekSection *section,
                         GFunc       func,
                         gpointer    user_data)
{
    EekSectionIface *iface;

    g_return_if_fail (EEK_IS_SECTION(section));

    iface = EEK_SECTION_GET_IFACE(section);
    g_return_if_fail (iface->foreach_key);

    return (*iface->foreach_key) (section, func, user_data);
}
