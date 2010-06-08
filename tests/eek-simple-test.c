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
#include "eek-simple-keyboard.h"

static void
test_create (void)
{
    EekKeyboard *keyboard;
    EekSection *section;
    EekKey *key;

    EekOutline outline = {45.0, NULL, 0};
    EekBounds bounds = {0.1, 0.2, 3.0, 4.0};
    EekKeysymMatrix *matrix;
    GValue value = {0};
    gint iv;
    const gchar *sv;
    gpointer bv;
    guint keysyms[] = {'a', 'b', 'c', 'd', 'e', 'f'};

    keyboard = eek_simple_keyboard_new ();
    g_assert (keyboard);
    g_assert (g_object_is_floating (keyboard));

    section = eek_keyboard_create_section (keyboard,
                                           "test-section",
                                           45,
                                           &bounds);
    g_assert (section);
    g_value_init (&value, G_TYPE_STRING);
    g_object_get_property (G_OBJECT(section), "name", &value);
    sv = g_value_get_string (&value);
    g_assert_cmpstr (sv, ==, "test-section");
    g_value_unset (&value);

    g_value_init (&value, G_TYPE_INT);
    g_object_get_property (G_OBJECT(section), "angle", &value);
    iv = g_value_get_int (&value);
    g_assert_cmpint (iv, ==, 45);
    g_value_unset (&value);

    g_value_init (&value, EEK_TYPE_BOUNDS);
    g_object_get_property (G_OBJECT(section), "bounds", &value);
    bv = g_value_get_boxed (&value);
    g_assert (bv);
    g_assert_cmpfloat (((EekBounds *)bv)->x, ==, 0.1);
    g_value_unset (&value);

    key = eek_section_create_key (section,
                                  "test-key",
                                  0,
                                  keysyms,
                                  3,
                                  2,
                                  1,
                                  2,
                                  &outline,
                                  &bounds);
    g_assert (key);
    g_value_init (&value, EEK_TYPE_KEYSYM_MATRIX);
    g_object_get_property (G_OBJECT(key), "keysyms", &value);
    matrix = g_value_get_boxed (&value);
    g_assert_cmpint (matrix->data[0], ==, 'a');
    g_value_unset (&value);

    g_value_init (&value, G_TYPE_POINTER);
    g_object_get_property (G_OBJECT(key), "outline", &value);
    bv = g_value_get_pointer (&value);
    g_assert (bv == &outline);
    g_value_unset (&value);
    g_object_unref (keyboard);
}

int
main (int argc, char **argv)
{
    g_type_init ();
    g_test_init (&argc, &argv, NULL);
    g_test_add_func ("/eek-simple-test/create", test_create);
    return g_test_run ();
}
