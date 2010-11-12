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
	libeek-gtk.la \
	libeek-xkb.la \
	libeek-xkl.la

if HAVE_CLUTTER
lib_LTLIBRARIES += libeek-clutter.la
endif

libeek_la_SOURCES = \
	$(srcdir)/eek-layout.c \
	$(srcdir)/eek-layout.h \
	$(srcdir)/eek-element.c \
	$(srcdir)/eek-element.h \
	$(srcdir)/eek-container.c \
	$(srcdir)/eek-container.h \
	$(srcdir)/eek-keyboard.c \
	$(srcdir)/eek-keyboard.h \
	$(srcdir)/eek-section.c \
	$(srcdir)/eek-section.h \
	$(srcdir)/eek-key.c \
	$(srcdir)/eek-key.h \
	$(srcdir)/eek-types.h \
	$(srcdir)/eek-types.c \
	$(srcdir)/eek-keysym.h \
	$(srcdir)/eek-keysym.c \
	$(srcdir)/eek-special-keysym-labels.h \
	$(srcdir)/eek-unicode-keysym-labels.h \
	$(srcdir)/eek-keyname-keysym-labels.h
libeek_la_CFLAGS = $(GOBJECT2_CFLAGS)
libeek_la_LIBADD = $(GOBJECT2_LIBS) -lm

if HAVE_CLUTTER
libeek_clutter_la_SOURCES = \
	$(srcdir)/eek-clutter-keyboard.c \
	$(srcdir)/eek-clutter-keyboard.h \
	$(srcdir)/eek-clutter-section.c \
	$(srcdir)/eek-clutter-section.h \
	$(srcdir)/eek-clutter-key.c \
	$(srcdir)/eek-clutter-key.h \
	$(srcdir)/eek-clutter-key-actor.c \
	$(srcdir)/eek-clutter-key-actor.h \
	$(srcdir)/eek-clutter-drawing-context.c \
	$(srcdir)/eek-clutter-drawing-context.h \
	$(srcdir)/eek-drawing.h \
	$(srcdir)/eek-drawing.c \
	$(srcdir)/eek-clutter.h

libeek_clutter_la_CFLAGS = $(CLUTTER_CFLAGS) $(CAIRO_LIBS) $(PANGO_LIBS)
libeek_clutter_la_LIBADD = libeek.la $(CLUTTER_LIBS) $(CAIRO_LIBS) $(PANGO_LIBS)
endif

libeek_gtk_la_SOURCES = \
	$(srcdir)/eek-gtk-keyboard.c \
	$(srcdir)/eek-gtk-keyboard.h \
	$(srcdir)/eek-drawing.h \
	$(srcdir)/eek-drawing.c \
	$(srcdir)/eek-gtk.h

libeek_gtk_la_CFLAGS = $(GTK2_CFLAGS) $(CAIRO_LIBS) $(PANGO_LIBS)
libeek_gtk_la_LIBADD = libeek.la $(GTK2_LIBS) $(CAIRO_LIBS) $(PANGO_LIBS)

libeek_xkb_la_SOURCES = \
	$(srcdir)/eek-xkb-layout.h \
	$(srcdir)/eek-xkb-layout.c

libeek_xkb_la_CFLAGS = $(GTK2_CFLAGS) $(XKB_CFLAGS)
libeek_xkb_la_LIBADD = libeek.la $(GTK2_LIBS) $(XKB_LIBS)

libeek_xkl_la_SOURCES = \
	$(srcdir)/eek-xkl-layout.h \
	$(srcdir)/eek-xkl-layout.c

libeek_xkl_la_CFLAGS = $(GTK2_CFLAGS) $(LIBXKLAVIER_CFLAGS)
libeek_xkl_la_LIBADD = libeek-xkb.la $(GTK2_LIBS) $(LIBXKLAVIER_LIBS)

eekdir = $(includedir)/eek-$(EEK_API_VERSION)/eek
eek_HEADERS = \
	$(srcdir)/eek-element.h \
	$(srcdir)/eek-container.h \
	$(srcdir)/eek-keyboard.h \
	$(srcdir)/eek-section.h \
	$(srcdir)/eek-key.h \
	$(srcdir)/eek-layout.h \
	$(srcdir)/eek-keysym.h \
	$(srcdir)/eek-types.h \
	$(srcdir)/eek-gtk-keyboard.h \
	$(srcdir)/eek-xkb-layout.h \
	$(srcdir)/eek-xkl-layout.h \
	$(srcdir)/eek.h \
	$(srcdir)/eek-gtk.h \
	$(srcdir)/eek-xkb.h \
	$(srcdir)/eek-xkl.h

if HAVE_CLUTTER
eek_HEADERS += \
	$(srcdir)/eek-clutter-keyboard.h \
	$(srcdir)/eek-clutter-section.h \
	$(srcdir)/eek-clutter-key.h \
	$(srcdir)/eek-clutter-drawing-context.h \
	$(srcdir)/eek-clutter.h
endif

eek-keysym.c: eek-special-keysym-labels.h eek-unicode-keysym-labels.h eek-keyname-keysym-labels.h

eek-special-keysym-labels.h: special-keysym-labels.txt
	$(PYTHON) ./gen-keysym-labels.py special_keysym_labels < $< > $@
eek-unicode-keysym-labels.h: unicode-keysym-labels.txt
	$(PYTHON) ./gen-keysym-labels.py unicode_keysym_labels < $< > $@
eek-keyname-keysym-labels.h: keyname-keysym-labels.txt
	$(PYTHON) ./gen-keysym-labels.py keyname_keysym_labels < $< > $@

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = \
	eek-$(EEK_API_VERSION).pc \
	eek-clutter-$(EEK_API_VERSION).pc \
	eek-gtk-$(EEK_API_VERSION).pc \
	eek-xkb-$(EEK_API_VERSION).pc \
	eek-xkl-$(EEK_API_VERSION).pc

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

CLEANFILES =

-include $(INTROSPECTION_MAKEFILE)
INTROSPECTION_GIRS =
INTROSPECTION_SCANNER_ARGS = --add-include-path=$(builddir)
INTROSPECTION_COMPILER_ARGS = --includedir=$(srcdir)

if HAVE_INTROSPECTION

Eek-0.1.gir: libeek.la
Eek_0_1_gir_SCANNERFLAGS = --strip-prefix=Eek --pkg=glib-2.0
Eek_0_1_gir_INCLUDES = GLib-2.0 GObject-2.0
Eek_0_1_gir_CFLAGS = $(libeek_la_CFLAGS)
Eek_0_1_gir_LIBS = libeek.la
Eek_0_1_gir_FILES = $(libeek_la_SOURCES)

EekGtk-0.1.gir: libeek-gtk.la Eek-0.1.gir
EekGtk_0_1_gir_INCLUDES = GObject-2.0 Gtk-3.0 Eek-0.1
EekGtk_0_1_gir_CFLAGS = $(libeek_gtk_la_CFLAGS)
EekGtk_0_1_gir_LIBS = libeek-gtk.la
EekGtk_0_1_gir_FILES = $(libeek_gtk_la_SOURCES)

if HAVE_CLUTTER
EekClutter-0.1.gir: libeek-clutter.la Eek-0.1.gir
EekClutter_0_1_gir_INCLUDES = GObject-2.0 Clutter-1.0 Eek-0.1
EekClutter_0_1_gir_CFLAGS = $(libeek_clutter_la_CFLAGS)
EekClutter_0_1_gir_LIBS = libeek-clutter.la
EekClutter_0_1_gir_FILES = $(libeek_clutter_la_SOURCES)
endif

EekXkb-0.1.gir: libeek-xkb.la Eek-0.1.gir
EekXkb_0_1_gir_INCLUDES = GObject-2.0 Eek-0.1
EekXkb_0_1_gir_CFLAGS = $(libeek_xkb_la_CFLAGS)
EekXkb_0_1_gir_LIBS = libeek-xkb.la
EekXkb_0_1_gir_FILES = $(libeek_xkb_la_SOURCES)

EekXkl-0.1.gir: libeek-xkl.la EekXkb-0.1.gir
EekXkl_0_1_gir_INCLUDES = GObject-2.0 EekXkb-0.1
EekXkl_0_1_gir_CFLAGS = $(libeek_xkl_la_CFLAGS)
EekXkl_0_1_gir_LIBS = libeek-xkl.la
EekXkl_0_1_gir_FILES = $(libeek_xkl_la_SOURCES)

INTROSPECTION_GIRS += \
	Eek-0.1.gir \
	EekGtk-0.1.gir \
	EekXkb-0.1.gir \
	EekXkl-0.1.gir

if HAVE_CLUTTER
INTROSPECTION_GIRS += \
	EekClutter-0.1.gir
endif

girdir = $(datadir)/gir-1.0
gir_DATA = $(INTROSPECTION_GIRS)

typelibdir = $(libdir)/girepository-1.0
typelib_DATA = $(INTROSPECTION_GIRS:.gir=.typelib)

CLEANFILES += $(gir_DATA) $(typelib_DATA)
endif
