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
bin_PROGRAMS = eekboard-system-client eekboard-client eekboard-server
noinst_LIBRARIES = libeekboard.a

libeekboard_a_headers = proxy.h

libeekboard_a_CFLAGS = \
	-I$(top_srcdir) \
	$(GIO2_CFLAGS)

libeekboard_a_SOURCES = proxy.c

eekboard_system_client_CFLAGS = \
	-I$(top_srcdir) \
	$(GIO2_CFLAGS) \
	$(GTK_CFLAGS) \
	$(GCONF2_CFLAGS) \
	$(XKB_CFLAGS) \
	$(LIBXKLAVIER_CFLAGS) \
	$(LIBFAKEKEY_CFLAGS) \
	$(CSPI_CFLAGS)

eekboard_system_client_LDADD = \
	libeekboard.a \
	$(top_builddir)/eek/libeek.la \
	$(top_builddir)/eek/libeek-xkl.la \
	$(GIO2_LIBS) \
	$(GTK_LIBS) \
	$(GCONF2_LIBS) \
	$(XKB_LIBS) \
	$(LIBXKLAVIER_LIBS) \
	$(LIBFAKEKEY_LIBS) \
	$(CSPI_LIBS)

eekboard_system_client_headers = system-client.h
eekboard_system_client_SOURCES = system-client.c system-client-main.c

eekboard_server_CFLAGS = \
	-I$(top_srcdir) \
	$(GIO2_CFLAGS) \
	$(GTK_CFLAGS)

eekboard_server_LDADD = \
	libeekboard.a \
	$(top_builddir)/eek/libeek.la \
	$(top_builddir)/eek/libeek-gtk.la \
	$(GIO2_LIBS) \
	$(GTK_LIBS)

eekboard_server_headers = server.h
eekboard_server_SOURCES = server.c server-main.c

if HAVE_CLUTTER
eekboard_server_CFLAGS += $(CLUTTER_CFLAGS) $(CLUTTER_GTK_CFLAGS)
eekboard_server_LDADD += $(CLUTTER_LIBS) $(top_builddir)/eek/libeek-clutter.la $(CLUTTER_GTK_LIBS)
endif

eekboard_client_CFLAGS = \
	-I$(top_srcdir) \
	$(GIO2_CFLAGS)

eekboard_client_LDADD = \
	libeekboard.a \
	$(top_builddir)/eek/libeek.la \
	$(GIO2_LIBS)

eekboard_client_SOURCES = client-main.c

noinst_HEADERS = \
	$(libeekboard_a_headers) \
	$(eekboard_system_client_headers) \
	$(eekboard_client_headers) \
	$(eekboard_server_headers)
endif
