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

if ENABLE_EEKBOARD
bin_PROGRAMS = \
	eekboard \
	eekboard-server \
	eekboard-xml

noinst_LTLIBRARIES = libxklutil.la

eekboard_CFLAGS = \
	-I$(top_srcdir) \
	$(GIO2_CFLAGS) \
	$(GTK_CFLAGS) \
	$(XKB_CFLAGS) \
	$(LIBXKLAVIER_CFLAGS) \
	-DKEYBOARDDIR=\"$(pkgdatadir)/keyboards\"

eekboard_LDADD = \
	$(builddir)/libxklutil.la \
	$(top_builddir)/eekboard/libeekboard.la \
	$(top_builddir)/eek/libeek.la \
	$(top_builddir)/eek/libeek-xkl.la \
	$(GIO2_LIBS) \
	$(GTK_LIBS) \
	$(XKB_LIBS) \
	$(LIBXKLAVIER_LIBS)

if ENABLE_FAKEKEY
eekboard_CFLAGS += \
	$(FAKEKEY_CFLAGS)
eekboard_LDADD += \
	$(FAKEKEY_LIBS)
endif

if ENABLE_ATSPI
eekboard_CFLAGS += \
	$(ATSPI2_CFLAGS)
eekboard_LDADD += \
	$(ATSPI2_LIBS)
endif

eekboard_headers = client.h
eekboard_SOURCES = client.c client-main.c

eekboard_server_CFLAGS = \
	-I$(top_srcdir) \
	$(GIO2_CFLAGS) \
	$(GTK_CFLAGS) \
	-DTHEMEDIR=\"$(pkgdatadir)/themes\"

eekboard_server_LDADD = \
	$(top_builddir)/eekboard/libeekboard.la \
	$(top_builddir)/eek/libeek.la \
	$(top_builddir)/eek/libeek-gtk.la \
	$(GIO2_LIBS) \
	$(GTK_LIBS)

if ENABLE_CLUTTER
eekboard_server_CFLAGS += $(CLUTTER_CFLAGS) $(CLUTTER_GTK_CFLAGS)
eekboard_server_LDADD += $(CLUTTER_LIBS) $(top_builddir)/eek/libeek-clutter.la $(CLUTTER_GTK_LIBS)
endif

eekboard_server_headers = server-server.h server-context.h
eekboard_server_SOURCES = server-server.c server-context.c server-main.c

eekboard_xml_CFLAGS = \
	-I$(top_srcdir) \
	$(GIO2_CFLAGS) \
	$(GTK_CFLAGS) \
	$(LIBXKLAVIER_CFLAGS)

eekboard_xml_LDADD = \
	$(builddir)/libxklutil.la \
	$(top_builddir)/eek/libeek.la \
	$(top_builddir)/eek/libeek-xkl.la \
	$(top_builddir)/eek/libeek-gtk.la \
	$(GIO2_LIBS) \
	$(GTK_LIBS) \
	$(LIBXKLAVIER_LIBS)

if ENABLE_CLUTTER
eekboard_xml_CFLAGS += $(CLUTTER_CFLAGS) $(CLUTTER_GTK_CFLAGS)
eekboard_xml_LDADD += $(CLUTTER_LIBS) $(top_builddir)/eek/libeek-clutter.la $(CLUTTER_GTK_LIBS)
endif

eekboard_xml_SOURCES = xml-main.c

libxklutil_la_headers = xklutil.h
libxklutil_la_SOURCES = xklutil.c
libxklutil_la_CFLAGS = $(LIBXKLAVIER_CFLAGS)
libxklutil_la_LIBADD = $(LIBXKLAVIER_LIBS)

eekboarddir = $(includedir)/eekboard-$(EEK_API_VERSION)/eekboard
eekboard_HEADERS =				\
	$(libeekboard_headers)

noinst_HEADERS = \
	$(eekboard_headers) \
	$(eekboard_server_headers) \
	$(eekboard_xml_headers) \
	$(libxklutil_la_headers)

EXTRA_DIST = $(service_in_files)
DISTCLEANFILES = $(service_DATA)

servicedir       = $(datadir)/dbus-1/services
service_in_files = eekboard-server.service.in
service_DATA     = $(service_in_files:.service.in=.service)

$(service_DATA): $(service_in_files) Makefile
	$(AM_V_GEN) sed -e "s|\@bindir\@|$(bindir)|" $< > $@
endif
