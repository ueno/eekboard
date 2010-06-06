# Copyright (C) 2010 Daiki Ueno <ueno@unixuser.org>
# Copyright (C) 2010 Red Hat, Inc.

# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public License
# as published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.

# This library is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.

# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
# 02110-1301 USA

noinst_PROGRAMS = eek-clutter-xkb-test eek-gtk-xkb-test
eek_clutter_xkb_test_CFLAGS = -I$(top_srcdir) $(GOBJECT2_CFLAGS) $(CLUTTER_CFLAGS) $(XKB_CFLAGS)
eek_clutter_xkb_test_LDFLAGS = $(top_builddir)/eek/libeek.la $(top_builddir)/eek/libeek-xkb.la $(top_builddir)/eek/libeek-clutter.la $(GOBJECT2_LIBS) $(CLUTTER_LIBS) $(XKB_LIBS)

eek_gtk_xkb_test_CFLAGS = -I$(top_srcdir) $(GOBJECT2_CFLAGS) $(GTK2_CFLAGS) $(XKB_CFLAGS)
eek_gtk_xkb_test_LDFLAGS = $(top_builddir)/eek/libeek.la $(top_builddir)/eek/libeek-xkb.la $(top_builddir)/eek/libeek-gtk.la $(GOBJECT2_LIBS) $(GTK2_LIBS) $(XKB_LIBS)
