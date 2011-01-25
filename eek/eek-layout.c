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
 * @short_description: Base class of a layout engine
 *
 * The #EekLayout class is a base class of layout engine which
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

G_DEFINE_TYPE (EekLayout, eek_layout, G_TYPE_OBJECT);

static void
eek_layout_class_init (EekLayoutClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

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
                      G_TYPE_FROM_CLASS(gobject_class),
                      G_SIGNAL_RUN_FIRST,
                      G_STRUCT_OFFSET(EekLayoutClass, group_changed),
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
                      G_TYPE_FROM_CLASS(gobject_class),
                      G_SIGNAL_RUN_FIRST,
                      G_STRUCT_OFFSET(EekLayoutClass, changed),
                      NULL,
                      NULL,
                      g_cclosure_marshal_VOID__VOID,
                      G_TYPE_NONE, 0);
}

void
eek_layout_init (EekLayout *self)
{
}

/**
 * eek_keyboard_new:
 * @layout: an #EekLayout
 * @initial_width: initial width of the keyboard
 * @initial_height: initial height of the keyboard
 *
 * Create a new #EekKeyboard based on @layout.
 */
EekKeyboard *
eek_keyboard_new (EekLayout *layout,
                  gdouble    initial_width,
                  gdouble    initial_height)
{
    g_assert (EEK_IS_LAYOUT(layout));
    g_assert (EEK_LAYOUT_GET_CLASS(layout)->create_keyboard);

    return EEK_LAYOUT_GET_CLASS(layout)->create_keyboard (layout,
                                                          initial_width,
                                                          initial_height);
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
    g_assert (EEK_IS_LAYOUT(layout));
    g_assert (EEK_LAYOUT_GET_CLASS(layout)->get_group);

    return EEK_LAYOUT_GET_CLASS(layout)->get_group (layout);
}
