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

NULL =

lib_LTLIBRARIES = libeekboard.la

libeekboard_headers =				\
	$(srcdir)/eekboard-service.h		\
	$(srcdir)/eekboard-context-service.h	\
	$(srcdir)/eekboard-client.h		\
	$(srcdir)/eekboard-context.h		\
	$(srcdir)/eekboard-xklutil.h		\
	$(NULL)

libeekboard_private_headers =			\
	$(builddir)/eekboard-marshalers.h	\
	$(NULL)

libeekboard_sources =				\
	$(srcdir)/eekboard-service.c		\
	$(srcdir)/eekboard-context-service.c	\
	$(srcdir)/eekboard-client.c		\
	$(srcdir)/eekboard-context.c		\
	$(srcdir)/eekboard-xklutil.c		\
	$(NULL)

libeekboard_marshalers_sources =		\
	$(builddir)/eekboard-marshalers.c	\
	$(builddir)/eekboard-marshalers.h	\
	$(NULL)

BUILT_SOURCES =					\
	$(libeekboard_marshalers_sources)	\
	$(NULL)

libeekboard_la_SOURCES =			\
	$(libeekboard_sources)			\
	$(builddir)/eekboard-marshalers.c	\
	$(NULL)

libeekboard_la_CFLAGS =					\
	-DEEKBOARD_COMPILATION=1			\
	-DKEYBOARDDIR=\"$(pkgdatadir)/keyboards\"	\
	-I$(top_srcdir)					\
	$(GIO2_CFLAGS)					\
	$(LIBXKLAVIER_CFLAGS)				\
	$(NULL)
libeekboard_la_LIBADD =				\
	$(top_builddir)/eek/libeek.la		\
	$(top_builddir)/eek/libeek-xkl.la	\
	$(GIO2_LIBS)				\
	$(LIBXKLAVIER_LIBS)			\
	$(NULL)

eekboarddir = $(includedir)/eekboard-$(EEK_API_VERSION)/eekboard
eekboard_HEADERS = $(libeekboard_headers)

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA =				\
	eekboard-$(EEK_API_VERSION).pc		\
	$(NULL)
DISTCLEANFILES =				\
	$(BUILT_SOURCES)			\
	$(pkgconfig_DATA)			\
	$(NULL)

CLEANFILES =

EXTRA_DIST = eekboard-marshalers.list

# gen marshal
eekboard-marshalers.h: eekboard-marshalers.list
	$(AM_V_GEN) $(GLIB_GENMARSHAL) \
		--prefix=_eekboard_marshal \
		$(srcdir)/eekboard-marshalers.list --header --internal \
		> $@.tmp && \
	mv $@.tmp $@

eekboard-marshalers.c: eekboard-marshalers.list eekboard-marshalers.h
	$(AM_V_GEN) (echo "#include \"eekboard-marshalers.h\""; \
	$(GLIB_GENMARSHAL) \
		--prefix=_eekboard_marshal \
		$(srcdir)/eekboard-marshalers.list --body --internal) \
		> $@.tmp && \
	mv $@.tmp $@

-include $(INTROSPECTION_MAKEFILE)
INTROSPECTION_GIRS =
INTROSPECTION_SCANNER_ARGS =			\
	--add-include-path=$(builddir)		\
	--add-include-path=$(top_builddir)/eek	\
	$(NULL)
INTROSPECTION_COMPILER_ARGS =			\
	--includedir=$(builddir)		\
	--includedir=$(top_builddir)/eek	\
	$(NULL)

if HAVE_INTROSPECTION
Eekboard@EEK_LIBRARY_SUFFIX@.gir: libeekboard.la
Eekboard@EEK_LIBRARY_SUFFIX_U@_gir_SCANNERFLAGS =	\
	--identifier-prefix=Eekboard			\
	--symbol-prefix=eekboard			\
	$(NULL)
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

-include $(top_srcdir)/git.mk
