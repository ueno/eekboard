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
#include "eek/eek-xkl.h"

static void
test_output_parse (void)
{
    GString *output;
    GInputStream *input;
    EekLayout *layout;
    EekKeyboard *keyboard;
    Display *display;

    output = g_string_sized_new (8192);

    display = XOpenDisplay (NULL);
    layout = eek_xkl_layout_new (display, NULL);

    keyboard = eek_keyboard_new (layout, 640, 480);
    g_object_unref (layout);

    eek_keyboard_output (keyboard, output, 0);
    g_object_unref (keyboard);

#if 0
    fwrite (output->str, sizeof(gchar), output->len, stdout);
#endif

    input = g_memory_input_stream_new_from_data (output->str,
                                                 output->len,
                                                 NULL);
    layout = eek_xml_layout_new (input);
    g_object_unref (input);

    keyboard = eek_keyboard_new (layout, 640, 480);
    g_object_unref (layout);
    g_object_unref (keyboard);

    g_string_free (output, TRUE);
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
