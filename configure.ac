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

AC_INIT([eekboard], [0.0.1], [ueno@unixuser.org])
AC_CONFIG_SRCDIR([configure.ac])
AC_PREREQ(2.63)
AM_INIT_AUTOMAKE
AM_PROG_CC_C_O
AC_CONFIG_MACRO_DIR([m4])
LT_INIT
IT_PROG_INTLTOOL([0.35.0])

PKG_CHECK_MODULES([GLIB2], [glib-2.0], ,
  [AC_MSG_ERROR([GLib2 not found])])
PKG_CHECK_MODULES([GOBJECT2], [gobject-2.0], ,
  [AC_MSG_ERROR([GObject2 not found])])
PKG_CHECK_MODULES([CAIRO], [cairo], ,
  [AC_MSG_ERROR([Cairo not found])])
PKG_CHECK_MODULES([PANGO], [pango], ,
  [AC_MSG_ERROR([Pango not found])])
PKG_CHECK_MODULES([GTK2], [gtk+-2.0 gdk-2.0], ,
  [AC_MSG_ERROR([GTK2 not found])])
PKG_CHECK_MODULES([XKB], [x11], ,
  [AC_MSG_ERROR([XKB support not found])])
PKG_CHECK_MODULES([LIBXKLAVIER], [libxklavier x11], ,
  [AC_MSG_ERROR([Libxklavier not found])])
PKG_CHECK_MODULES([LIBFAKEKEY], [libfakekey], ,
  [AC_MSG_ERROR([libfakekey not found])])

AC_MSG_CHECKING([whether you enable Vala language support])
AC_ARG_ENABLE(vala,
              AS_HELP_STRING([--enable-vala=no/yes],
                             [Enable Vala language binding default=yes]),,
              enable_vala=yes)
AC_MSG_RESULT($enable_vala)
AM_CONDITIONAL(ENABLE_VALA, [test x$enable_vala = xyes])

AC_MSG_CHECKING([whether you enable eekboard])
AC_ARG_ENABLE(eekboard,
              AS_HELP_STRING([--enable-eekboard=no/yes],
                             [Build standalone application "ekboard" default=yes]),,
              enable_eekboard=yes)
AC_MSG_RESULT($enable_eekboard)
AM_CONDITIONAL(ENABLE_EEKBOARD, [test x$enable_eekboard = xyes])

AC_MSG_CHECKING([whether you enable Clutter])
AC_ARG_ENABLE(clutter,
              AS_HELP_STRING([--enable-clutter=no/yes],
                             [Enable Clutter user interface default=yes]),,
              enable_clutter=yes)
AC_MSG_RESULT($enable_clutter)

if test x$enable_clutter = xyes; then
  PKG_CHECK_MODULES([CLUTTER], [clutter-1.0], ,
    [AC_MSG_ERROR([Clutter not found -- install it or add --disable-clutter])])
  AC_DEFINE([HAVE_CLUTTER], [1], [Define if Clutter is found])
  PKG_CHECK_MODULES([CLUTTER_GTK], [clutter-gtk-0.90],
    [enable_clutter_gtk=yes])
  if test x$enable_clutter_gtk = xno; then
    PKG_CHECK_MODULES([CLUTTER_GTK], [clutter-gtk-0.10],
      [enable_clutter_gtk=yes])
  fi
  if test x$enable_clutter_gtk = xyes; then
    AC_DEFINE([HAVE_CLUTTER_GTK], [1], [Define if Clutter-Gtk is found])
  fi
fi
AM_CONDITIONAL(HAVE_CLUTTER, [test x$enable_clutter = xyes])
AM_CONDITIONAL(HAVE_CLUTTER_GTK, [test x$enable_clutter_gtk = xyes])

GTK_DOC_CHECK([1.14],[--flavour no-tmpl])

EEK_API_VERSION=0.1
AC_SUBST(EEK_API_VERSION)

dnl to re-generate eek/*-keysym-labels.txt
AC_CHECK_PROGS([PYTHON], [python])

dnl define GETTEXT_* variables
GETTEXT_PACKAGE=$PACKAGE
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [Define to the read-only architecture-independent data directory.])
AM_GLIB_GNU_GETTEXT
AM_GLIB_DEFINE_LOCALEDIR(EEKBOARD_LOCALEDIR)

GOBJECT_INTROSPECTION_CHECK([0.6.7])

AC_CONFIG_HEADERS([eek/config.h])
AC_CONFIG_FILES([Makefile
eek/Makefile
src/Makefile
tests/Makefile
bindings/Makefile
bindings/vala/Makefile
docs/Makefile
docs/reference/Makefile
docs/reference/eek/Makefile
po/Makefile.in
eek/eek-${EEK_API_VERSION}.pc
eek/eek-clutter-${EEK_API_VERSION}.pc
eek/eek-gtk-${EEK_API_VERSION}.pc
eek/eek-xkb-${EEK_API_VERSION}.pc
eek/eek-xkl-${EEK_API_VERSION}.pc])
AC_OUTPUT
