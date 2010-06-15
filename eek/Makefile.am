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

lib_LTLIBRARIES = \
	libeek.la \
	libeek-clutter.la \
	libeek-gtk.la \
	libeek-xkb.la \
	libeek-xkl.la

libeek_la_SOURCES = \
	eek-layout.c \
	eek-layout.h \
	eek-element.c \
	eek-element.h \
	eek-container.c \
	eek-container.h \
	eek-keyboard.c \
	eek-keyboard.h \
	eek-section.c \
	eek-section.h \
	eek-key.c \
	eek-key.h \
	eek-types.h \
	eek-types.c \
	eek-keysym.h \
	eek-keysym.c \
	eek-special-keysym-labels.h \
	eek-unicode-keysym-labels.h \
	eek-keyname-keysym-labels.h

libeek_la_CFLAGS = $(GOBJECT2_CFLAGS) $(CAIRO_CFLAGS) $(PANGO_CFLAGS)
libeek_la_LIBADD = $(GOBJECT2_LIBS) $(CAIRO_LIBS) $(PANGO_LIBS)

libeek_clutter_la_SOURCES = \
	eek-clutter-keyboard.c \
	eek-clutter-keyboard.h \
	eek-clutter-section.c \
	eek-clutter-section.h \
	eek-clutter-key.c \
	eek-clutter-key.h \
	eek-clutter-key-actor.c \
	eek-clutter-key-actor.h \
	eek-drawing.h \
	eek-drawing.c \
	eek-clutter.h

libeek_clutter_la_CFLAGS = $(CLUTTER_CFLAGS)
libeek_clutter_la_LIBADD = libeek.la $(CLUTTER_LIBS)

libeek_gtk_la_SOURCES = \
	eek-gtk-keyboard.c \
	eek-gtk-keyboard.h \
	eek-drawing.h \
	eek-drawing.c \
	eek-gtk.h

libeek_gtk_la_CFLAGS = $(GTK2_CFLAGS)
libeek_gtk_la_LIBADD = libeek.la $(GTK2_LIBS)

libeek_xkb_la_SOURCES = \
	eek-xkb-layout.h \
	eek-xkb-layout.c

libeek_xkb_la_CFLAGS = $(GTK2_CFLAGS) $(XKB_CFLAGS)
libeek_xkb_la_LIBADD = libeek.la $(GTK2_LIBS) $(XKB_LIBS)

libeek_xkl_la_SOURCES = \
	eek-xkl-layout.h \
	eek-xkl-layout.c

libeek_xkl_la_CFLAGS = $(GTK2_CFLAGS) $(LIBXKLAVIER_CFLAGS)
libeek_xkl_la_LIBADD = libeek-xkb.la $(GTK2_LIBS) $(LIBXKLAVIER_LIBS)

eekdir = $(includedir)/eek-$(EEK_API_VERSION)/eek
eek_HEADERS = \
	$(top_srcdir)/eek/eek-element.h \
	$(top_srcdir)/eek/eek-container.h \
	$(top_srcdir)/eek/eek-keyboard.h \
	$(top_srcdir)/eek/eek-section.h \
	$(top_srcdir)/eek/eek-key.h \
	$(top_srcdir)/eek/eek-layout.h \
	$(top_srcdir)/eek/eek-keysym.h \
	$(top_srcdir)/eek/eek-types.h \
	$(top_srcdir)/eek/eek-clutter-keyboard.h \
	$(top_srcdir)/eek/eek-clutter-section.h \
	$(top_srcdir)/eek/eek-clutter-key.h \
	$(top_srcdir)/eek/eek-xkb-layout.h \
	$(top_srcdir)/eek/eek-xkl-layout.h \
	$(top_srcdir)/eek/eek.h \
	$(top_srcdir)/eek/eek-clutter.h \
	$(top_srcdir)/eek/eek-xkb.h \
	$(top_srcdir)/eek/eek-xkl.h

eek-keysym.c: eek-special-keysym-labels.h eek-unicode-keysym-labels.h eek-keyname-keysym-labels.h

eek-special-keysym-labels.h: special-keysym-labels.txt
	$(PYTHON) ./gen-keysym-labels.py special_keysym_labels < $< > $@
eek-unicode-keysym-labels.h: unicode-keysym-labels.txt
	$(PYTHON) ./gen-keysym-labels.py unicode_keysym_labels < $< > $@
eek-keyname-keysym-labels.h: keyname-keysym-labels.txt
	$(PYTHON) ./gen-keysym-labels.py keyname_keysym_labels < $< > $@

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libeek.pc libeek-clutter.pc libeek-xkb.pc libeek-xkl.pc

DISTCLEANFILES = \
	eek-special-keysym-labels.h \
	eek-unicode-keysym-labels.h \
	eek-keyname-keysym-labels.h \
	$(pkgconfig_DATA)

EXTRA_DIST = \
	gen-keysym-labels.py \
	special-keysym-labels.txt \
	unicode-keysym-labels.txt \
	keyname-keysym-labels.txt
