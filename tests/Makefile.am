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

INCLUDES = -I$(top_srcdir) $(CLUTTER_CFLAGS) $(GOBJECT2_CFLAGS) $(GTK2_CFLAGS) $(XKB_CFLAGS)

TESTS = eek-simple-test eek-xkb-test
noinst_PROGRAMS = $(TESTS)

eek_simple_test_SOURCES = eek-simple-test.c
eek_simple_test_LDADD = $(top_builddir)/eek/libeek.la $(top_builddir)/eek/libeek-clutter.la

eek_xkb_test_SOURCES = eek-xkb-test.c
eek_xkb_test_LDADD = $(top_builddir)/eek/libeek.la $(top_builddir)/eek/libeek-xkb.la
