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

bin_PROGRAMS =					\
	eekboard				\
	eekboard-server				\
	$(NULL)

libexec_PROGRAMS =				\
	eekboard-setup				\
	$(NULL)

eekboard_CFLAGS =					\
	-I$(top_srcdir)					\
	$(GIO2_CFLAGS)					\
	$(GTK_CFLAGS)					\
	$(XKB_CFLAGS)					\
	$(LIBXKLAVIER_CFLAGS)				\
	-DLIBEXECDIR=\"$(libexecdir)\"			\
	$(NULL)

eekboard_LDADD =				\
	$(top_builddir)/eekboard/libeekboard.la	\
	$(top_builddir)/eek/libeek.la		\
	$(top_builddir)/eek/libeek-xkl.la	\
	$(GIO2_LIBS)				\
	$(GTK_LIBS)				\
	$(XKB_LIBS)				\
	$(LIBXKLAVIER_LIBS)			\
	$(NULL)

if ENABLE_XTEST
eekboard_CFLAGS += $(XTEST_CFLAGS)
eekboard_LDADD += $(XTEST_LIBS)
endif

if ENABLE_ATSPI
eekboard_CFLAGS += $(ATSPI2_CFLAGS)
eekboard_LDADD += $(ATSPI2_LIBS)
endif

if ENABLE_IBUS
eekboard_CFLAGS += $(IBUS_CFLAGS)
eekboard_LDADD += $(IBUS_LIBS)
endif

eekboard_headers =				\
	client.h				\
	$(NULL)

eekboard_SOURCES =				\
	client.c				\
	client-main.c				\
	$(NULL)

eekboard_server_CFLAGS =				\
	-I$(top_srcdir)					\
	$(GIO2_CFLAGS)					\
	$(GTK_CFLAGS)					\
	$(LIBXKLAVIER_CFLAGS)				\
	-DTHEMESDIR=\"$(pkgdatadir)/themes\"		\
	$(NULL)

eekboard_server_LDADD =				\
	$(top_builddir)/eekboard/libeekboard.la	\
	$(top_builddir)/eek/libeek.la		\
	$(top_builddir)/eek/libeek-gtk.la	\
	$(top_builddir)/eek/libeek-xkl.la	\
	$(GIO2_LIBS)				\
	$(GTK_LIBS)				\
	$(LIBXKLAVIER_LIBS)			\
	$(NULL)

if ENABLE_XDOCK
eekboard_server_CFLAGS += $(XDOCK_CFLAGS)
eekboard_server_LDADD += $(XDOCK_LIBS)
endif

eekboard_server_headers =			\
	server-service.h			\
	server-context-service.h		\
	$(NULL)

eekboard_server_SOURCES =			\
	server-service.c			\
	server-context-service.c		\
	server-main.c				\
	$(NULL)

eekboard_setup_CFLAGS = \
	-I$(top_srcdir)					\
	$(GIO2_CFLAGS)					\
	$(GTK_CFLAGS)					\
	$(XKB_CFLAGS)					\
	$(LIBXKLAVIER_CFLAGS)				\
	-DPKGDATADIR=\"$(pkgdatadir)\"			\
	$(NULL)

eekboard_setup_headers =			\
	preferences-dialog.h			\
	$(NULL)

eekboard_setup_SOURCES =			\
	preferences-dialog.c			\
	setup-main.c				\
	$(NULL)

eekboard_setup_LDADD =				\
	$(top_builddir)/eek/libeek.la		\
	$(GIO2_LIBS)				\
	$(GTK_LIBS)				\
	$(NULL)

dist_pkgdata_DATA = preferences-dialog.ui

noinst_HEADERS =				\
	$(eekboard_headers)			\
	$(eekboard_server_headers)		\
	$(eekboard_setup_headers)		\
	$(NULL)
