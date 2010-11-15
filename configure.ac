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

AC_INIT([eekboard], [0.0.5], [ueno@unixuser.org])
AC_CONFIG_SRCDIR([configure.ac])
AC_PREREQ(2.63)
AM_INIT_AUTOMAKE
AM_PROG_CC_C_O
AC_CONFIG_MACRO_DIR([m4])
LT_INIT
IT_PROG_INTLTOOL([0.35.0])

AC_MSG_CHECKING([which gtk+ version to compile against])
AC_ARG_WITH([gtk],
  [AS_HELP_STRING([--with-gtk=2.0|3.0],[which gtk+ version to compile against (default: 2.0)])],
  [case "$with_gtk" in
     2.0|3.0) ;;
     *) AC_MSG_ERROR([invalid gtk version specified]) ;;
   esac],
  [with_gtk=2.0])
AC_MSG_RESULT([$with_gtk])

case "$with_gtk" in
  2.0) GTK_API_VERSION=2.0
       GTK_REQUIRED=2.14.0
       EEK_API_VERSION=0.1
       EEK_API_MAJOR_VERSION=0
       EEK_API_MINOR_VERSION=1
       EEK_API_PC_VERSION=0.1
       EEK_LIBRARY_SUFFIX="-$EEK_API_VERSION"
       ;;
  3.0) GTK_API_VERSION=3.0
       GTK_REQUIRED=2.91.0
#       EEK_API_VERSION=0.90
#       EEK_API_MAJOR_VERSION=0
#       EEK_API_MINOR_VERSION=90
#       EEK_API_PC_VERSION=0.90
       EEK_API_VERSION=0.1
       EEK_API_MAJOR_VERSION=0
       EEK_API_MINOR_VERSION=1
       EEK_API_PC_VERSION=0.1
       EEK_LIBRARY_SUFFIX="-$EEK_API_VERSION"
       ;;
esac

AC_SUBST([GTK_API_VERSION])
AC_SUBST([EEK_API_VERSION])
AC_SUBST([EEK_API_MAJOR_VERSION])
AC_SUBST([EEK_API_MINOR_VERSION])
AC_SUBST([EEK_API_PC_VERSION])
AC_SUBST([EEK_LIBRARY_SUFFIX])
AC_SUBST([EEK_LIBRARY_SUFFIX_U],[AS_TR_SH([$EEK_LIBRARY_SUFFIX])])

AM_CONDITIONAL([HAVE_GTK_2],[test "$with_gtk" = "2.0"])
AM_CONDITIONAL([HAVE_GTK_3],[test "$with_gtk" = "3.0"])

PKG_CHECK_MODULES([GLIB2], [glib-2.0], ,
  [AC_MSG_ERROR([GLib2 not found])])
PKG_CHECK_MODULES([GOBJECT2], [gobject-2.0], ,
  [AC_MSG_ERROR([GObject2 not found])])
PKG_CHECK_MODULES([CAIRO], [cairo], ,
  [AC_MSG_ERROR([Cairo not found])])
PKG_CHECK_MODULES([PANGO], [pango], ,
  [AC_MSG_ERROR([Pango not found])])
PKG_CHECK_MODULES([GTK], [
  gtk+-$GTK_API_VERSION >= $GTK_REQUIRED
  gdk-$GTK_API_VERSION >= $GTK_REQUIRED], ,
  [AC_MSG_ERROR([GTK not found])])
PKG_CHECK_MODULES([GCONF2], [gconf-2.0], ,
  [AC_MSG_ERROR([GConf not found])])
PKG_CHECK_MODULES([XKB], [x11], ,
  [AC_MSG_ERROR([XKB support not found])])
PKG_CHECK_MODULES([LIBXKLAVIER], [libxklavier x11], ,
  [AC_MSG_ERROR([Libxklavier not found])])
PKG_CHECK_MODULES([LIBFAKEKEY], [libfakekey], ,
  [AC_MSG_ERROR([libfakekey not found])])
PKG_CHECK_MODULES([CSPI], [cspi-1.0], ,
  [AC_MSG_ERROR([AT-SPI C not found])])

PKG_CHECK_MODULES([NOTIFY], [libnotify], ,
  [AC_MSG_ERROR([libnotify not found])])

dnl libnotify >= 0.7.0 omitted the feature to attach notification to widget.
need_libnotify_attach_workaround=0
save_CFLAGS="$CFLAGS"
CFLAGS="$NOTIFY_CFLAGS"
save_LIBS="$LIBS"
LIBS="$NOTIFY_LIBS"
AC_TRY_LINK([#include <libnotify/notification.h>],
  [notify_notification_new (NULL, NULL, NULL);], ,
  [need_libnotify_attach_workaround=1])
CFLAGS="$save_CFLAGS"
LIBS="$save_LIBS"
AC_DEFINE_UNQUOTED([NEED_LIBNOTIFY_ATTACH_WORKAROUND],
  $need_libnotify_attach_workaround,
  [Define if notify_notification_new work around is needed])

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
  have_clutter_gtk=0
  need_swap_event_workaround=0
  PKG_CHECK_MODULES([CLUTTER_GTK], [clutter-gtk-1.0], [have_clutter_gtk=1],
    [PKG_CHECK_MODULES([CLUTTER_GTK], [clutter-gtk-0.10 clutter-x11-1.0],
      [have_clutter_gtk=1; need_swap_event_workaround=1])])
  AC_DEFINE_UNQUOTED([HAVE_CLUTTER_GTK], $have_clutter_gtk,
    [Define if Clutter-Gtk is found])
  AC_DEFINE_UNQUOTED([NEED_SWAP_EVENT_WORKAROUND], $need_swap_event_workaround,
    [Define if GLX_INTEL_swap_event work around is needed])
fi
AM_CONDITIONAL(HAVE_CLUTTER, [test x$enable_clutter = xyes])

GTK_DOC_CHECK([1.14],[--flavour no-tmpl])

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
