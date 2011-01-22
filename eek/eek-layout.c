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
 * SECTION:eek-layout
 * @short_description: Base interface of a layout engine
 *
 * The #EekLayout class is a base interface of layout engine which
 * arranges keyboard elements.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif  /* HAVE_CONFIG_H */

#include "eek-layout.h"
#include "eek-keyboard.h"

enum {
    GROUP_CHANGED,
    CHANGED,
    LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0, };

static void
eek_layout_base_init (gpointer gobject_class)
{
    static gboolean is_initialized = FALSE;

    if (!is_initialized) {
        /**
         * EekLayout::group-changed:
         * @layout: an #EekLayout that received the signal
         * @group: group index
         *
         * The ::group-changed signal is emitted each time group
         * configuration of @layout changed.
         */
        signals[GROUP_CHANGED] =
            g_signal_new ("group-changed",
                          G_TYPE_FROM_INTERFACE(gobject_class),
                          G_SIGNAL_RUN_FIRST,
                          G_STRUCT_OFFSET(EekLayoutIface, group_changed),
                          NULL,
                          NULL,
                          g_cclosure_marshal_VOID__INT,
                          G_TYPE_NONE, 1,
                          G_TYPE_INT);

        /**
         * EekLayout::changed:
         * @layout: an #EekLayout that received the signal
         *
         * The ::changed signal is emitted each time @layout changed
         * and re-layout of #EekKeyboard is needed.
         */
        signals[CHANGED] =
            g_signal_new ("changed",
                          G_TYPE_FROM_INTERFACE(gobject_class),
                          G_SIGNAL_RUN_FIRST,
                          G_STRUCT_OFFSET(EekLayoutIface, changed),
                          NULL,
                          NULL,
                          g_cclosure_marshal_VOID__VOID,
                          G_TYPE_NONE, 0);
        is_initialized = TRUE;
    }
}

GType
eek_layout_get_type (void)
{
    static GType iface_type = 0;
    if (iface_type == 0) {
        static const GTypeInfo info = {
            sizeof (EekLayoutIface),
            eek_layout_base_init,
            NULL,
        };
        iface_type = g_type_register_static (G_TYPE_INTERFACE,
                                             "EekLayout",
                                             &info, 0);
    }
    return iface_type;
}

/**
 * eek_layout_apply:
 * @layout: an #EekLayout
 * @keyboard: an #EekKeyboard
 *
 * Relayout @keyboard with the @layout.
 */
void
eek_layout_apply (EekLayout *layout, EekKeyboard *keyboard)
{
    g_return_if_fail (EEK_IS_LAYOUT(layout));
    g_return_if_fail (EEK_IS_KEYBOARD(keyboard));

    return EEK_LAYOUT_GET_IFACE(layout)->apply (layout, keyboard);
}

/**
 * eek_layout_get_group:
 * @layout: an #EekLayout
 *
 * Get the group index from the @layout.  This function normally
 * called after #EekLayout::group-changed signal to change group index
 * of all the keys in #EekKeyboard at a time.
 */
gint
eek_layout_get_group (EekLayout *layout)
{
    g_return_val_if_fail (EEK_IS_LAYOUT(layout), -1);
    return EEK_LAYOUT_GET_IFACE(layout)->get_group (layout);
}
