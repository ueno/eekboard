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
#include "eek/eek-xkb.h"

/* For gdk_x11_display_get_xdisplay().  See main(). */
#include <gtk/gtk.h>

static void
test_create (void)
{
    EekLayout *layout;
    const gchar *keycodes, *geometry, *symbols;

    layout = eek_xkb_layout_new ();
    g_assert (layout);
    keycodes = eek_xkb_layout_get_keycodes (EEK_XKB_LAYOUT(layout));
    g_assert (keycodes);
    geometry = eek_xkb_layout_get_geometry (EEK_XKB_LAYOUT(layout));
    g_assert (geometry);
    symbols = eek_xkb_layout_get_symbols (EEK_XKB_LAYOUT(layout));
    g_assert (symbols);
    eek_xkb_layout_set_geometry (EEK_XKB_LAYOUT(layout), "winbook");
    geometry = eek_xkb_layout_get_geometry (EEK_XKB_LAYOUT(layout));
    g_assert_cmpstr (geometry, ==, "winbook");
    g_object_unref (layout);
}

int
main (int argc, char **argv)
{
    g_type_init ();
    g_test_init (&argc, &argv, NULL);
    gtk_init (&argc, &argv);  /* for gdk_x11_display_get_xdisplay() */
    g_test_add_func ("/eek-xkb-test/create", test_create);
    return g_test_run ();
}
