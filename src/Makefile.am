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

if ENABLE_EEKBOARD
bin_PROGRAMS = eekboard
eekboard_CFLAGS = \
	-I$(top_srcdir) \
	$(GOBJECT2_CFLAGS) \
	$(GTK2_CFLAGS) \
	$(XKB_CFLAGS) \
	$(LIBXKLAVIER_CFLAGS) \
	$(LIBFAKEKEY_CFLAGS)

eekboard_LDFLAGS = \
	$(top_builddir)/eek/libeek.la \
	$(top_builddir)/eek/libeek-xkl.la \
	$(top_builddir)/eek/libeek-gtk.la \
	$(GOBJECT2_LIBS) \
	$(GTK2_CFLAGS) \
	$(XKB_LIBS) \
	$(LIBXKLAVIER_LIBS) \
	$(LIBFAKEKEY_LIBS)

if HAVE_CLUTTER
eekboard_CFLAGS += $(CLUTTER_CFLAGS) $(CLUTTER_GTK_CFLAGS)
eekboard_LDFLAGS += $(CLUTTER_LIBS) $(top_builddir)/eek/libeek-clutter.la $(CLUTTER_GTK_LIBS)
endif
endif
