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
 * SECTION:eek-keyboard
 * @short_description: Base interface of a keyboard
 * @see_also: #EekSection
 *
 * The #EekKeyboardIface interface represents a keyboard, which
 * consists of one or more sections of the #EekSectionIface interface.
 *
 * #EekKeyboardIface follows the Builder pattern and is responsible
 * for creation of sections.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif  /* HAVE_CONFIG_H */

#include "eek-keyboard.h"

static void
eek_keyboard_base_init (gpointer g_iface)
{
    static gboolean is_initialized = FALSE;

    if (!is_initialized) {
        GParamSpec *pspec;

        /**
         * EekKeyboard:bounds:
         *
         * The bounding box of #EekKeyboard.
         */
        pspec = g_param_spec_boxed ("bounds",
                                    "Bounds",
                                    "Bounding box of the keyboard",
                                    EEK_TYPE_BOUNDS,
                                    G_PARAM_READWRITE);
        g_object_interface_install_property (g_iface, pspec);

        is_initialized = TRUE;
    }
}

GType
eek_keyboard_get_type (void)
{
    static GType iface_type = 0;

    if (iface_type == 0) {
        static const GTypeInfo info = {
            sizeof (EekKeyboardIface),
            eek_keyboard_base_init,
            NULL
        };

        iface_type = g_type_register_static (G_TYPE_INTERFACE,
                                             "EekKeyboard",
                                             &info,
                                             0);
    }
    return iface_type;
}

/**
 * eek_keyboard_set_bounds:
 * @keyboard: an #EekKeyboard
 * @bounds: bounding box of the keyboard
 *
 * Set the bounding box of @keyboard to @bounds.
 */
void
eek_keyboard_set_bounds (EekKeyboard *keyboard,
                         EekBounds   *bounds)
{
    EekKeyboardIface *iface = EEK_KEYBOARD_GET_IFACE(keyboard);

    g_return_if_fail (iface);
    g_return_if_fail (iface->set_bounds);
    (*iface->set_bounds) (keyboard, bounds);
}

/**
 * eek_keyboard_get_bounds:
 * @keyboard: an #EekKeyboard
 * @bounds: the bounding box of @keyboard
 *
 * Get the bounding box of @keyboard.
 */
void
eek_keyboard_get_bounds (EekKeyboard *keyboard,
                         EekBounds   *bounds)
{
    EekKeyboardIface *iface = EEK_KEYBOARD_GET_IFACE(keyboard);

    g_return_if_fail (iface);
    g_return_if_fail (iface->get_bounds);
    return (*iface->get_bounds) (keyboard, bounds);
}

/**
 * eek_keyboard_set_keysym_index:
 * @keyboard: an #EekKeyboard
 * @group: row index of the symbol matrix of keys on @keyboard
 * @level: column index of the symbol matrix of keys on @keyboard
 *
 * Select a cell of the symbol matrix of each key on @keyboard.
 */
void
eek_keyboard_set_keysym_index (EekKeyboard *self,
                               gint         group,
                               gint         level)
{
    EekKeyboardIface *iface = EEK_KEYBOARD_GET_IFACE(self);

    g_return_if_fail (iface);
    g_return_if_fail (iface->set_keysym_index);
    (*iface->set_keysym_index) (self, group, level);
}

/**
 * eek_keyboard_set_keysym_index:
 * @keyboard: an #EekKeyboard
 * @group: a pointer where row index of the symbol matrix of keys on
 * @keyboard will be stored
 * @level: a pointer where column index of the symbol matrix of keys
 * on @keyboard will be stored
 *
 * Get the current cell position of the symbol matrix of each key on @keyboard.
 */
void
eek_keyboard_get_keysym_index (EekKeyboard *self,
                               gint        *group,
                               gint        *level)
{
    EekKeyboardIface *iface = EEK_KEYBOARD_GET_IFACE(self);

    g_return_if_fail (iface);
    g_return_if_fail (iface->get_keysym_index);
    return (*iface->get_keysym_index) (self, group, level);
}

/**
 * eek_keyboard_create_section:
 * @keyboard: an #EekKeyboard
 * @name: name of the section
 * @angle: rotation angle of the section
 * @bounds: bounding box of the section
 *
 * Create an #EekSection instance and attach it to @keyboard.
 */
EekSection *
eek_keyboard_create_section (EekKeyboard *keyboard,
                             const gchar *name,
                             gint         angle,
                             EekBounds   *bounds)
{
    EekKeyboardIface *iface = EEK_KEYBOARD_GET_IFACE(keyboard);

    g_return_val_if_fail (iface, NULL);
    g_return_val_if_fail (iface->create_section, NULL);
    return (*iface->create_section) (keyboard, name, angle, bounds);
}

/**
 * eek_keyboard_foreach_section:
 * @keyboard: an #EekKeyboard
 * @func: a callback of #GFunc
 * @user_data: a pointer to an object passed to @func
 *
 * Iterate over @keyboard's children list.
 */
void
eek_keyboard_foreach_section (EekKeyboard *keyboard,
                              GFunc        func,
                              gpointer     user_data)
{
    EekKeyboardIface *iface = EEK_KEYBOARD_GET_IFACE(keyboard);

    g_return_if_fail (iface);
    g_return_if_fail (iface->foreach_section);
    (*iface->foreach_section) (keyboard, func, user_data);
}

/**
 * eek_keyboard_set_layout:
 * @keyboard: an #EekKeyboard
 * @layout: an #EekLayout
 *
 * Set the layout of @keyboard to @layout.  This actually turns
 * @keyboard to be ready to be drawn on the screen.
 */
void
eek_keyboard_set_layout (EekKeyboard *keyboard,
                         EekLayout   *layout)
{
    EekKeyboardIface *iface = EEK_KEYBOARD_GET_IFACE(keyboard);

    g_return_if_fail (iface);
    g_return_if_fail (iface->set_layout);
    g_return_if_fail (EEK_IS_LAYOUT(layout));
    (*iface->set_layout) (keyboard, layout);
}
