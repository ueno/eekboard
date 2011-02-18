# Copyright (C) 2011 Daiki Ueno <ueno@unixuser.org>
# Copyright (C) 2011 Red Hat, Inc.

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

lib_LTLIBRARIES = libeekboard.la

libeekboard_headers = eekboard.h eekboard-keyboard.h
libeekboard_sources = eekboard-keyboard.c

libeekboard_la_SOURCES = $(libeekboard_sources)
libeekboard_la_CFLAGS = -I$(top_srcdir) $(GIO2_CFLAGS)
libeekboard_la_LIBADD = $(top_builddir)/eek/libeek.la $(GIO2_LIBS)

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA =				\
	eekboard-$(EEK_API_VERSION).pc
DISTCLEANFILES =				\
	$(pkgconfig_DATA)

CLEANFILES =

-include $(INTROSPECTION_MAKEFILE)
INTROSPECTION_GIRS =
INTROSPECTION_SCANNER_ARGS = --add-include-path=$(builddir) --add-include-path=$(top_builddir)/eek
INTROSPECTION_COMPILER_ARGS = --includedir=$(srcdir) --includedir=$(top_srcdir)/eek

if HAVE_INTROSPECTION
Eekboard@EEK_LIBRARY_SUFFIX@.gir: libeekboard.la
Eekboard@EEK_LIBRARY_SUFFIX_U@_gir_INCLUDES = Eek@EEK_LIBRARY_SUFFIX@
Eekboard@EEK_LIBRARY_SUFFIX_U@_gir_CFLAGS = $(libeekboard_la_CFLAGS)
Eekboard@EEK_LIBRARY_SUFFIX_U@_gir_LIBS = libeekboard.la
Eekboard@EEK_LIBRARY_SUFFIX_U@_gir_FILES = $(libeekboard_sources) $(libeekboard_headers)

INTROSPECTION_GIRS += Eekboard@EEK_LIBRARY_SUFFIX@.gir

girdir = $(datadir)/gir-1.0
gir_DATA = $(INTROSPECTION_GIRS)

typelibdir = $(libdir)/girepository-1.0
typelib_DATA = $(INTROSPECTION_GIRS:.gir=.typelib)

CLEANFILES += $(gir_DATA) $(typelib_DATA)
endif
