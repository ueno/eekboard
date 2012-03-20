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

/* For gdk_x11_display_get_xdisplay().  See main(). */
#include <gtk/gtk.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif  /* HAVE_CONFIG_H */

#include "eek/eek.h"

static void
test_output_parse (void)
{
    EekLayout *layout;
    EekKeyboard *keyboard;
    GError *error;

    error = NULL;
    layout = eek_xml_layout_new ("us", &error);
    g_assert_no_error (error);

    keyboard = eek_keyboard_new (layout, 640, 480);
    g_object_unref (layout);
    g_object_unref (keyboard);
}

int
main (int argc, char **argv)
{
    g_type_init ();
    g_test_init (&argc, &argv, NULL);
    gtk_init (&argc, &argv);  /* for gdk_x11_display_get_xdisplay() */

    g_test_add_func ("/eek-xml-test/output-parse", test_output_parse);

    return g_test_run ();
}
