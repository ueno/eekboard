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
#include "eek.h"

static void
test_create (void)
{
    EekKeyboard *keyboard;
    EekSection *section;
    EekKey *key0, *key1;

    keyboard = g_object_new (EEK_TYPE_KEYBOARD, NULL);
    section = eek_keyboard_create_section (keyboard);
    g_assert (EEK_IS_SECTION(section));
    eek_section_add_row (section, 2, EEK_ORIENTATION_HORIZONTAL);
    key0 = eek_section_create_key (section, 0, 0);
    g_assert (EEK_IS_KEY(key0));
    key1 = eek_section_create_key (section, 1, 0);
    g_assert (EEK_IS_KEY(key1));
}

int
main (int argc, char **argv)
{
    g_type_init ();
    g_test_init (&argc, &argv, NULL);
    g_test_add_func ("/eek-simple-test/create", test_create);
    return g_test_run ();
}
