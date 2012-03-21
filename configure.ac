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

AC_PREREQ(2.63)
dnl AC_CONFIG_SRCDIR([configure.ac])
AC_CONFIG_MACRO_DIR([m4])

AC_INIT([eekboard], [1.0.5], [ueno@unixuser.org])

dnl Init automake
AM_INIT_AUTOMAKE
AM_MAINTAINER_MODE([enable])
AC_GNU_SOURCE

dnl Support silent build
m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])

dnl Check for programs
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_CC_STDC
AC_PROG_INSTALL
AC_PROG_CXX

# define PACKAGE_VERSION_* variables
AM_DISABLE_STATIC
AC_ISC_POSIX
AC_HEADER_STDC
LT_INIT
IT_PROG_INTLTOOL([0.35.0])

GTK_API_VERSION=3.0
GTK_REQUIRED=2.91.0
EEK_API_VERSION=0.90
EEK_API_MAJOR_VERSION=0
EEK_API_MINOR_VERSION=90
EEK_API_PC_VERSION=0.90
EEK_LIBRARY_SUFFIX="-$EEK_API_VERSION"

AC_SUBST([GTK_API_VERSION])
AC_SUBST([EEK_API_VERSION])
AC_SUBST([EEK_API_MAJOR_VERSION])
AC_SUBST([EEK_API_MINOR_VERSION])
AC_SUBST([EEK_API_PC_VERSION])
AC_SUBST([EEK_LIBRARY_SUFFIX])
AC_SUBST([EEK_LIBRARY_SUFFIX_U],[AS_TR_SH([$EEK_LIBRARY_SUFFIX])])

AM_PATH_GLIB_2_0
PKG_CHECK_MODULES([GLIB2], [glib-2.0 >= 2.26.0], ,
  [AC_MSG_ERROR([GLib2 not found])])
PKG_CHECK_MODULES([GIO2], [gio-2.0], ,
  [AC_MSG_ERROR([Gio2 not found])])
GLIB_GSETTINGS

PKG_CHECK_MODULES([PANGOCAIRO], [pangocairo], ,
  [AC_MSG_ERROR([PangoCairo not found])])
PKG_CHECK_MODULES([GTK], [
  gtk+-$GTK_API_VERSION >= $GTK_REQUIRED
  gdk-$GTK_API_VERSION >= $GTK_REQUIRED], ,
  [AC_MSG_ERROR([GTK not found])])
PKG_CHECK_MODULES([LIBXKLAVIER], [libxklavier x11], ,
  [AC_MSG_ERROR([Libxklavier not found])])
PKG_CHECK_MODULES([LIBCROCO], [libcroco-0.6], ,
  [AC_MSG_ERROR([libcroco not found])])

dnl use XTest to generate key events
AC_MSG_CHECKING([whether you enable XTest])
AC_ARG_ENABLE(xtest,
              AS_HELP_STRING([--enable-xtest=no/yes],
                             [Enable XTest default=yes]),
              enable_xtest=$enableval,
              enable_xtest=yes)

if test x$enable_xtest = xyes; then
  PKG_CHECK_MODULES([XTEST], [xtst], , enable_xtest=no)
  if test x$enable_xtest = xyes; then
    AC_DEFINE([HAVE_XTEST], [1], [Define if XTest is found])
  fi
fi
AM_CONDITIONAL(ENABLE_XTEST, [test x$enable_xtest = xyes])
AC_MSG_RESULT($enable_xtest)

dnl use X to mark the fullscreen window as dock
AC_MSG_CHECKING([whether you enable X dock])
AC_ARG_ENABLE(x-dock,
              AS_HELP_STRING([--enable-x-dock=no/yes],
                             [Enable X dock default=yes]),
              enable_x_dock=$enableval,
              enable_x_dock=yes)

if test x$enable_x_dock = xyes; then
  PKG_CHECK_MODULES([XDOCK], [x11], , enable_x_dock=no)
  if test x$enable_x_dock = xyes; then
    AC_DEFINE([HAVE_XDOCK], [1], [Define if X dock is found])
  fi
fi
AM_CONDITIONAL(ENABLE_XDOCK, [test x$enable_x_dock = xyes])
AC_MSG_RESULT($enable_x_dock)

focus_listeners=""
keystroke_listeners=""

focus_listeners=""
keystroke_listeners=""

dnl use AT-SPI 2 to capture focus/keystroke events
AC_MSG_CHECKING([whether you enable AT-SPI 2 event handling])
AC_ARG_ENABLE(atspi,
              AS_HELP_STRING([--enable-atspi=no/yes],
                             [Enable AT-SPI 2 event handling default=yes]),
              enable_atspi=$enableval,
              enable_atspi=yes)

if test x$enable_atspi = xyes; then
  PKG_CHECK_MODULES([ATSPI2], [atspi-2 dbus-glib-1], , enable_atspi=no)
  if test x$enable_atspi = xyes; then
    AC_DEFINE([HAVE_ATSPI], [1], [Define if AT-SPI 2 is found])
    focus_listeners="atspi $focus_listeners"
    keystroke_listeners="atspi $keystroke_listeners"
  fi
fi
AC_MSG_RESULT($enable_atspi)
AM_CONDITIONAL(ENABLE_ATSPI, [test x$enable_atspi = xyes])

dnl use IBus to capture focus events
AC_MSG_CHECKING([whether you enable IBus focus tracking])
AC_ARG_ENABLE(ibus,
              AS_HELP_STRING([--enable-ibus=no/yes],
                             [Enable IBus focus tracking default=yes]),
              enable_ibus=$enableval,
              enable_ibus=yes)

if test x$enable_ibus = xyes; then
  PKG_CHECK_MODULES([IBUS], [ibus-1.0 >= 1.3.99], , enable_ibus=no)
  if test x$enable_ibus = xyes; then
    AC_DEFINE([HAVE_IBUS], [1], [Define if IBus is found])
    focus_listeners="ibus $focus_listeners"
  fi
fi
AC_MSG_RESULT($enable_ibus)
AM_CONDITIONAL(ENABLE_IBUS, [test x$enable_ibus = xyes])

if test -n "$focus_listeners"; then
  AC_DEFINE(ENABLE_FOCUS_LISTENER, [1], [Define if eekboard can follow focus changes])
fi

GOBJECT_INTROSPECTION_CHECK([0.9.0])

dnl Vala langauge binding
AC_MSG_CHECKING([whether you enable Vala language support])
AC_ARG_ENABLE(vala,
              AS_HELP_STRING([--enable-vala=no/yes],
                             [Enable Vala language binding default=yes]),
              enable_vala=$enableval,
              enable_vala=yes)
if test x$enable_vala = xyes; then
  if test "x$INTROSPECTION_SCANNER" = x; then
    enable_vala=no
    AC_MSG_WARN([GObject-Introspection must be enabled for Vala bindings])
  fi

  AM_PROG_VALAC([0.10.0])
  have_vala=yes

  AC_PATH_PROG(VALAC, valac, valac)
  AC_SUBST(VALAC)
  AC_SUBST(VALAFLAGS)

  AC_PATH_PROG([VAPIGEN], [vapigen], [false])

  if test "x$VAPIGEN" = "xfalse"; then
    enable_vala=no
    AC_MSG_WARN([vapigen not found. Was vala compiled with --enable-vapigen?])
  fi

  AC_SUBST(VAPIGEN)
fi
AC_MSG_RESULT($enable_vala)
AM_CONDITIONAL(ENABLE_VALA, [test x$enable_vala = xyes])

dnl libcanberra
AC_MSG_CHECKING([whether you enable libcanberra])
AC_ARG_ENABLE(libcanberra,
              AS_HELP_STRING([--enable-libcanberra=no/yes],
                             [Enable libcanberra user interface default=no]),
              enable_libcanberra=$enableval,
              enable_libcanberra=yes)

if test x$enable_libcanberra = xyes; then
  PKG_CHECK_MODULES([LIBCANBERRA], [libcanberra-gtk3], , enable_libcanberra=no)
  if test x$enable_libcanberra = xyes; then
    AC_DEFINE([HAVE_LIBCANBERRA], [1], [Define if libcanberra is found])
  fi
fi
AM_CONDITIONAL(ENABLE_LIBCANBERRA, [test x$enable_libcanberra = xyes])
AC_MSG_RESULT($enable_libcanberra)

GTK_DOC_CHECK([1.14],[--flavour no-tmpl])

dnl define GETTEXT_* variables
GETTEXT_PACKAGE=$PACKAGE
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [Define to the read-only architecture-independent data directory.])
AM_GLIB_GNU_GETTEXT
AM_GLIB_DEFINE_LOCALEDIR(EEKBOARD_LOCALEDIR)

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile
eek/Makefile
eekboard/Makefile
src/Makefile
tests/Makefile
bindings/Makefile
bindings/vala/Makefile
docs/Makefile
docs/reference/Makefile
docs/reference/eek/Makefile
docs/reference/eekboard/Makefile
po/Makefile.in
data/Makefile
data/icons/Makefile
data/icons/16x16/Makefile
data/icons/22x22/Makefile
data/icons/24x24/Makefile
data/icons/32x32/Makefile
data/icons/48x48/Makefile
data/icons/scalable/Makefile
data/themes/Makefile
data/keyboards/Makefile
examples/Makefile
examples/simple-client/Makefile
eek/eek-${EEK_API_VERSION}.pc
eek/eek-gtk-${EEK_API_VERSION}.pc
eek/eek-xkl-${EEK_API_VERSION}.pc
eekboard/eekboard-${EEK_API_VERSION}.pc])
AC_OUTPUT
AC_MSG_RESULT([
Build options:
  Version                   $VERSION
  Install prefix            $prefix
  Build shared libs         $enable_shared
  Build static libs         $enable_static
  CFLAGS                    $CFLAGS
  GTK version               $with_gtk
  Build Vala binding        $enable_vala
  Sound support             $enable_libcanberra
  Build document            $enable_gtk_doc
  Focus listeners           $focus_listeners
  Keystroke listeners       $keystroke_listeners
])


