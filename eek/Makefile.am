# Copyright (C) 2010-2011 Daiki Ueno <ueno@unixuser.org>
# Copyright (C) 2010-2011 Red Hat, Inc.

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

lib_LTLIBRARIES =				\
	libeek.la				\
	libeek-gtk.la				\
	libeek-xkl.la

libeek_public_headers =				\
	$(srcdir)/eek-layout.h			\
	$(srcdir)/eek-element.h			\
	$(srcdir)/eek-container.h		\
	$(srcdir)/eek-keyboard.h		\
	$(srcdir)/eek-section.h			\
	$(srcdir)/eek-key.h			\
	$(srcdir)/eek-symbol.h			\
	$(srcdir)/eek-keysym.h			\
	$(srcdir)/eek-text.h			\
	$(srcdir)/eek-symbol-matrix.h		\
	$(srcdir)/eek-types.h			\
	$(srcdir)/eek-xml.h			\
	$(srcdir)/eek-xml-layout.h		\
	$(srcdir)/eek-serializable.h		\
	$(srcdir)/eek-theme.h			\
	$(srcdir)/eek.h

libeek_private_headers =			\
	$(srcdir)/eek-renderer.h		\
	$(srcdir)/eek-special-keysym-entries.h	\
	$(srcdir)/eek-unicode-keysym-entries.h	\
	$(srcdir)/eek-xkeysym-keysym-entries.h	\
	$(srcdir)/eek-marshalers.h		\
	$(srcdir)/eek-theme-context.h		\
	$(srcdir)/eek-theme-private.h		\
	$(srcdir)/eek-theme-node.h

libeek_sources =				\
	$(srcdir)/eek.c				\
	$(srcdir)/eek-layout.c			\
	$(srcdir)/eek-element.c			\
	$(srcdir)/eek-container.c		\
	$(srcdir)/eek-keyboard.c		\
	$(srcdir)/eek-section.c			\
	$(srcdir)/eek-key.c			\
	$(srcdir)/eek-symbol-matrix.c		\
	$(srcdir)/eek-symbol.c			\
	$(srcdir)/eek-keysym.c			\
	$(srcdir)/eek-text.c			\
	$(srcdir)/eek-types.c			\
	$(srcdir)/eek-serializable.c		\
	$(srcdir)/eek-xml.c			\
	$(srcdir)/eek-xml-layout.c		\
	$(srcdir)/eek-renderer.c		\
	$(srcdir)/eek-keyboard-drawing.c	\
	$(srcdir)/eek-theme.c			\
	$(srcdir)/eek-theme-context.c		\
	$(srcdir)/eek-theme-node.c

libeek_keysym_sources =				\
	$(srcdir)/eek-special-keysym-entries.h	\
	$(srcdir)/eek-unicode-keysym-entries.h	\
	$(srcdir)/eek-xkeysym-keysym-entries.h

libeek_enumtypes_sources =			\
	$(srcdir)/eek-enumtypes.c		\
	$(srcdir)/eek-enumtypes.h

libeek_marshalers_sources =			\
	$(srcdir)/eek-marshalers.c		\
	$(srcdir)/eek-marshalers.h

BUILT_SOURCES =					\
	$(libeek_keysym_sources)		\
	$(libeek_enumtypes_sources)		\
	$(libeek_marshalers_sources)

libeek_la_SOURCES =				\
	$(libeek_sources)			\
	$(srcdir)/eek-enumtypes.c		\
	$(srcdir)/eek-marshalers.c

libeek_la_CFLAGS =					\
	-DEEK_COMPILATION=1				\
	-DKEYBOARDSDIR=\"$(pkgdatadir)/keyboards\"	\
	$(GIO2_CFLAGS)					\
	$(PANGOCAIRO_CFLAGS)				\
	$(LIBCROCO_CFLAGS)				\
	$(NULL)

libeek_la_LIBADD =				\
	$(GIO2_LIBS)				\
	$(PANGOCAIRO_LIBS)			\
	$(LIBCROCO_LIBS)			\
	-lm					\
	$(NULL)

libeek_gtk_public_headers =			\
	$(srcdir)/eek-gtk-keyboard.h		\
	$(srcdir)/eek-gtk.h
libeek_gtk_private_headers =			\
	$(srcdir)/eek-gtk-renderer.h
libeek_gtk_sources =				\
	$(srcdir)/eek-gtk-keyboard.c		\
	$(srcdir)/eek-gtk-renderer.c

libeek_gtk_la_SOURCES = $(libeek_gtk_sources)
libeek_gtk_la_CFLAGS = -DEEK_COMPILATION=1 $(GTK_CFLAGS) $(LIBCANBERRA_CFLAGS)
libeek_gtk_la_LIBADD = libeek.la $(GTK_LIBS) $(LIBCANBERRA_LIBS)

libeek_xkl_public_headers =			\
	$(srcdir)/eek-xkl-layout.h		\
	$(srcdir)/eek-xkl.h			\
	$(srcdir)/eek-xkb-layout.h		\
	$(srcdir)/eek-xkb.h

libeek_xkl_sources =				\
	$(srcdir)/eek-xkb-layout.c		\
	$(srcdir)/eek-xkl-layout.c

libeek_xkl_la_SOURCES = $(libeek_xkl_sources)
libeek_xkl_la_CFLAGS = -DEEK_COMPILATION=1 $(LIBXKLAVIER_CFLAGS)
libeek_xkl_la_LIBADD = libeek.la $(LIBXKLAVIER_LIBS)

eekdir = $(includedir)/eek-$(EEK_API_VERSION)/eek
eek_HEADERS =					\
	$(libeek_public_headers)		\
	$(srcdir)/eek-enumtypes.h		\
	$(libeek_gtk_public_headers)		\
	$(libeek_xkl_public_headers)

noinst_HEADERS =				\
	$(libeek_private_headers)		\
	$(libeek_gtk_private_headers)		\
	$(libeek_xkl_private_headers)

eek-special-keysym-entries.h: special-keysym-entries.txt
	$(AM_V_GEN) $(PYTHON) ./gen-keysym-entries.py special_keysym_entries \
		< $< > $@
eek-unicode-keysym-entries.h: unicode-keysym-entries.txt
	$(AM_V_GEN) $(PYTHON) ./gen-keysym-entries.py unicode_keysym_entries \
		< $< > $@
eek-xkeysym-keysym-entries.h: xkeysym-keysym-entries.txt
	$(AM_V_GEN) $(PYTHON) ./gen-keysym-entries.py xkeysym_keysym_entries \
		< $< > $@

eek-enumtypes.h: $(libeek_public_headers) eek-enumtypes.h.template
	$(AM_V_GEN) $(GLIB_MKENUMS) --template eek-enumtypes.h.template \
		$(libeek_public_headers) > eek-enumtypes.h.tmp && \
		mv eek-enumtypes.h.tmp eek-enumtypes.h

eek-enumtypes.c: $(libeek_public_headers) eek-enumtypes.c.template
	$(AM_V_GEN) $(GLIB_MKENUMS) --template eek-enumtypes.c.template \
		$(libeek_public_headers) > eek-enumtypes.c.tmp && \
		mv eek-enumtypes.c.tmp eek-enumtypes.c

# gen marshal
eek-marshalers.h: eek-marshalers.list
	$(AM_V_GEN) $(GLIB_GENMARSHAL) --prefix=_eek_marshal $(srcdir)/eek-marshalers.list --header --internal > $@.tmp && \
	mv $@.tmp $@

eek-marshalers.c: eek-marshalers.list eek-marshalers.h
	$(AM_V_GEN) (echo "#include \"eek-marshalers.h\""; \
	$(GLIB_GENMARSHAL) --prefix=_eek_marshal $(srcdir)/eek-marshalers.list --body --internal) > $@.tmp && \
	mv $@.tmp $@

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA =				\
	eek-$(EEK_API_VERSION).pc		\
	eek-gtk-$(EEK_API_VERSION).pc		\
	eek-xkl-$(EEK_API_VERSION).pc

CLEANFILES =

DISTCLEANFILES =				\
	$(BUILT_SOURCES)			\
	$(pkgconfig_DATA)

EXTRA_DIST =					\
	gen-keysym-entries.py			\
	special-keysym-entries.txt		\
	unicode-keysym-entries.txt		\
	xkeysym-keysym-entries.txt		\
	eek-enumtypes.h.template		\
	eek-enumtypes.c.template		\
	eek-marshalers.list

-include $(INTROSPECTION_MAKEFILE)
INTROSPECTION_GIRS =
INTROSPECTION_SCANNER_ARGS = --add-include-path=$(builddir)
INTROSPECTION_COMPILER_ARGS = --includedir=$(srcdir)

if HAVE_INTROSPECTION

Eek@EEK_LIBRARY_SUFFIX@.gir: libeek.la
Eek@EEK_LIBRARY_SUFFIX_U@_gir_SCANNERFLAGS = --strip-prefix=Eek --pkg=glib-2.0 --pkg-export=eek-$(EEK_API_VERSION)
Eek@EEK_LIBRARY_SUFFIX_U@_gir_INCLUDES = GLib-2.0 GObject-2.0 Gio-2.0
Eek@EEK_LIBRARY_SUFFIX_U@_gir_CFLAGS = $(libeek_la_CFLAGS)
Eek@EEK_LIBRARY_SUFFIX_U@_gir_LIBS = libeek.la
Eek@EEK_LIBRARY_SUFFIX_U@_gir_FILES = $(libeek_sources) $(libeek_public_headers) $(srcdir)/eek-enumtypes.h

EekGtk@EEK_LIBRARY_SUFFIX@.gir: libeek-gtk.la Eek@EEK_LIBRARY_SUFFIX@.gir
EekGtk@EEK_LIBRARY_SUFFIX_U@_gir_SCANNERFLAGS = --pkg-export=eek-gtk-$(EEK_API_VERSION)
EekGtk@EEK_LIBRARY_SUFFIX_U@_gir_INCLUDES = GObject-2.0 Gtk-@GTK_API_VERSION@ Eek@EEK_LIBRARY_SUFFIX@
EekGtk@EEK_LIBRARY_SUFFIX_U@_gir_CFLAGS = $(libeek_gtk_la_CFLAGS)
EekGtk@EEK_LIBRARY_SUFFIX_U@_gir_LIBS = libeek-gtk.la
EekGtk@EEK_LIBRARY_SUFFIX_U@_gir_FILES = $(libeek_gtk_sources) $(libeek_gtk_public_headers)

EekXkl@EEK_LIBRARY_SUFFIX@.gir: libeek-xkl.la Eek@EEK_LIBRARY_SUFFIX@.gir
EekXkl@EEK_LIBRARY_SUFFIX_U@_gir_INCLUDES = GObject-2.0 Eek@EEK_LIBRARY_SUFFIX@
EekXkl@EEK_LIBRARY_SUFFIX_U@_gir_CFLAGS = $(libeek_xkl_la_CFLAGS)
EekXkl@EEK_LIBRARY_SUFFIX_U@_gir_LIBS = libeek-xkl.la
EekXkl@EEK_LIBRARY_SUFFIX_U@_gir_FILES = $(libeek_xkl_sources) $(libeek_xkl_public_headers)

INTROSPECTION_GIRS +=				\
	Eek@EEK_LIBRARY_SUFFIX@.gir		\
	EekGtk@EEK_LIBRARY_SUFFIX@.gir		\
	EekXkl@EEK_LIBRARY_SUFFIX@.gir

girdir = $(datadir)/gir-1.0
gir_DATA = $(INTROSPECTION_GIRS)

typelibdir = $(libdir)/girepository-1.0
typelib_DATA = $(INTROSPECTION_GIRS:.gir=.typelib)

CLEANFILES += $(gir_DATA) $(typelib_DATA)
endif

-include $(top_srcdir)/git.mk
