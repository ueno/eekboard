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

bin_PROGRAMS = \
	eekboard \
	eekboard-server

eekboard_CFLAGS = \
	-I$(top_srcdir) \
	$(GIO2_CFLAGS) \
	$(GTK_CFLAGS) \
	$(XKB_CFLAGS) \
	$(LIBXKLAVIER_CFLAGS) \
	-DKEYBOARDDIR=\"$(pkgdatadir)/keyboards\" \
	-DPKGDATADIR=\"$(pkgdatadir)\"

eekboard_LDADD = \
	$(top_builddir)/eekboard/libeekboard.la \
	$(top_builddir)/eek/libeek.la \
	$(top_builddir)/eek/libeek-xkl.la \
	$(GIO2_LIBS) \
	$(GTK_LIBS) \
	$(XKB_LIBS) \
	$(LIBXKLAVIER_LIBS)

if ENABLE_XTEST
eekboard_CFLAGS += \
	$(XTEST_CFLAGS)
eekboard_LDADD += \
	$(XTEST_LIBS)
endif

if ENABLE_ATSPI
eekboard_CFLAGS += \
	$(ATSPI2_CFLAGS)
eekboard_LDADD += \
	$(ATSPI2_LIBS)
endif

if ENABLE_IBUS
eekboard_CFLAGS += \
	$(IBUS_CFLAGS)
eekboard_LDADD += \
	$(IBUS_LIBS)
endif

eekboard_headers = client.h preferences-dialog.h
eekboard_SOURCES = client.c preferences-dialog.c client-main.c

eekboard_server_CFLAGS = \
	-I$(top_srcdir) \
	$(GIO2_CFLAGS) \
	$(GTK_CFLAGS) \
	$(LIBXKLAVIER_CFLAGS) \
	-DTHEMEDIR=\"$(pkgdatadir)/themes\" \
	-DKEYBOARDDIR=\"$(pkgdatadir)/keyboards\"

eekboard_server_LDADD = \
	$(top_builddir)/eekboard/libeekboard.la \
	$(top_builddir)/eek/libeek.la \
	$(top_builddir)/eek/libeek-gtk.la \
	$(top_builddir)/eek/libeek-xkl.la \
	$(GIO2_LIBS) \
	$(GTK_LIBS) \
	$(LIBXKLAVIER_LIBS)

if ENABLE_XDOCK
eekboard_server_CFLAGS += $(XDOCK_CFLAGS)
eekboard_server_LDADD += $(XDOCK_LIBS)
endif

eekboard_server_headers = server-service.h server-context-service.h
eekboard_server_SOURCES = server-service.c server-context-service.c server-main.c

eekboarddir = $(includedir)/eekboard-$(EEK_API_VERSION)/eekboard
eekboard_HEADERS =				\
	$(libeekboard_headers)

noinst_HEADERS = \
	$(eekboard_headers) \
	$(eekboard_server_headers)

dist_pkgdata_DATA = preferences-dialog.ui
