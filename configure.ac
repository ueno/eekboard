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

AC_INIT([eek], [0.0.0], [ueno@unixuser.org])
AC_CONFIG_SRCDIR([configure.ac])
AC_PREREQ(2.63)
AM_INIT_AUTOMAKE
AM_PROG_CC_C_O
AC_CONFIG_MACRO_DIR([m4])
LT_INIT

PKG_CHECK_MODULES([GLIB2], [glib-2.0], ,
  [AC_MSG_ERROR([GLib2 not found])])
PKG_CHECK_MODULES([GOBJECT2], [gobject-2.0], ,
  [AC_MSG_ERROR([GObject2 not found])])
PKG_CHECK_MODULES([CAIRO], [cairo], ,
  [AC_MSG_ERROR([Cairo not found])])
PKG_CHECK_MODULES([PANGO], [pango], ,
  [AC_MSG_ERROR([Pango not found])])
PKG_CHECK_MODULES([CLUTTER], [clutter-1.0], ,
  [AC_MSG_ERROR([Clutter not found])])
PKG_CHECK_MODULES([GTK2], [gtk+-2.0 gdk-2.0], ,
  [AC_MSG_ERROR([GTK2 not found])])
PKG_CHECK_MODULES([XKB], [x11], ,
  [AC_MSG_ERROR([XKB support not found])])
PKG_CHECK_MODULES([LIBXKLAVIER], [libxklavier x11], ,
  [AC_MSG_ERROR([Libxklavier not found])])

GTK_DOC_CHECK([1.14],[--flavour no-tmpl])

EEK_API_VERSION=0.1
AC_SUBST(EEK_API_VERSION)

AC_CHECK_PROGS([PYTHON], [python])

AC_CONFIG_HEADERS([eek/config.h])
AC_CONFIG_FILES([Makefile
eek/Makefile
examples/Makefile
tests/Makefile
docs/Makefile
docs/reference/Makefile
docs/reference/eek/Makefile
eek/eek.pc
eek/eek-clutter.pc
eek/eek-xkb.pc
eek/eek-xkl.pc])
AC_OUTPUT
