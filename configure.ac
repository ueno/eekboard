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

AC_INIT([eekboard], [0.90.7], [ueno@unixuser.org])

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
       EEK_API_VERSION=0.90
       EEK_API_MAJOR_VERSION=0
       EEK_API_MINOR_VERSION=90
       EEK_API_PC_VERSION=0.90
       EEK_LIBRARY_SUFFIX="-$EEK_API_VERSION"
       ;;
  3.0) GTK_API_VERSION=3.0
       GTK_REQUIRED=2.91.0
       EEK_API_VERSION=0.90
       EEK_API_MAJOR_VERSION=0
       EEK_API_MINOR_VERSION=90
       EEK_API_PC_VERSION=0.90
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
PKG_CHECK_MODULES([XKB], [x11], ,
  [AC_MSG_ERROR([XKB support not found])])
PKG_CHECK_MODULES([LIBXKLAVIER], [libxklavier x11], ,
  [AC_MSG_ERROR([Libxklavier not found])])
PKG_CHECK_MODULES([LIBCROCO], [libcroco-0.6], ,
  [AC_MSG_ERROR([libcroco not found])])

dnl use libfakekey to generate key events
AC_MSG_CHECKING([whether you enable fakekey])
AC_ARG_ENABLE(fakekey,
              AS_HELP_STRING([--enable-fakekey=no/yes],
                             [Enable fakekey default=yes]),,
              enable_fakekey=yes)

if test x$enable_fakekey = xyes; then
  PKG_CHECK_MODULES([FAKEKEY], [libfakekey], , enable_fakekey=no)
  if test x$enable_fakekey = xyes; then
    AC_DEFINE([HAVE_FAKEKEY], [1], [Define if fakekey is found])
  fi
fi
AM_CONDITIONAL(ENABLE_FAKEKEY, [test x$enable_fakekey = xyes])
AC_MSG_RESULT($enable_fakekey)

dnl use AT-SPI 2 to capture focus/keystroke events
AC_MSG_CHECKING([whether you enable AT-SPI 2 event handling])
AC_ARG_ENABLE(atspi,
              AS_HELP_STRING([--enable-atspi=no/yes],
                             [Enable AT-SPI 2 event handling default=no]),,
              enable_atspi=yes)

if test x$enable_atspi = xyes; then
  PKG_CHECK_MODULES([ATSPI2], [atspi-2 dbus-glib-1], , enable_atspi=no)
  if test x$enable_atspi = xyes; then
    AC_DEFINE([HAVE_ATSPI], [1], [Define if AT-SPI 2 is found])
  fi
fi
AC_MSG_RESULT($enable_atspi)
AM_CONDITIONAL(ENABLE_ATSPI, [test x$enable_atspi = xyes])

dnl Python language binding
AC_MSG_CHECKING([whether you enable Python language support])
AC_ARG_ENABLE(python,
              AS_HELP_STRING([--enable-python=no/yes],
                             [Enable Python language binding default=yes]),,
              enable_python=yes)

dnl check python unconditionally to re-generate
dnl eek/*-keysym-labels.txt when maintainer-mode enabled
AM_PATH_PYTHON([2.5], , enable_python=no)

if test x"$enable_python" = x"yes"; then
  if test x$enable_python = xyes; then
    AC_PATH_PROGS(PYTHON_CONFIG, [python$PYTHON_VERSION-config python-config])
    if test x"$PYTHON_CONFIG" != x""; then
      PYTHON_CFLAGS=`$PYTHON_CONFIG --includes`
      PYTHON_LIBS=`$PYTHON_CONFIG --libs`
    else
      PYTHON_CFLAGS=`$PYTHON $srcdir/python-config.py --includes`
      PYTHON_LIBS=`$PYTHON $srcdir/python-config.py --libs`
    fi
    PYTHON_INCLUDES="$PYTHON_CFLAGS"
    AC_SUBST(PYTHON_CFLAGS)
    AC_SUBST(PYTHON_INCLUDES)
    AC_SUBST(PYTHON_LIBS)
  fi
fi

AC_MSG_RESULT($enable_python)
AM_CONDITIONAL(ENABLE_PYTHON, [test x$enable_python = xyes])

GOBJECT_INTROSPECTION_CHECK([0.9.0])

dnl Vala langauge binding
AC_MSG_CHECKING([whether you enable Vala language support])
AC_ARG_ENABLE(vala,
              AS_HELP_STRING([--enable-vala=no/yes],
                             [Enable Vala language binding default=yes]),,
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

dnl clutter
AC_MSG_CHECKING([whether you enable clutter])
AC_ARG_ENABLE(clutter,
              AS_HELP_STRING([--enable-clutter=no/yes],
                             [Enable clutter user interface default=yes]),,
              enable_clutter=yes)

if test x$enable_clutter = xyes; then
  PKG_CHECK_MODULES([CLUTTER], [clutter-1.0], , enable_clutter=no)
  if test x$enable_clutter = xyes; then
    AC_DEFINE([HAVE_CLUTTER], [1], [Define if clutter is found])
  fi
fi
AM_CONDITIONAL(ENABLE_CLUTTER, [test x$enable_clutter = xyes])
AC_MSG_RESULT($enable_clutter)

dnl clutter-gtk
AC_MSG_CHECKING([whether you enable clutter-gtk])
AC_ARG_ENABLE(clutter-gtk,
              AS_HELP_STRING([--enable-clutter-gtk=no/yes],
                             [Enable clutter user interface default=yes]),,
              enable_clutter_gtk=yes)

if test x$enable_clutter_gtk = xyes; then
  PKG_CHECK_MODULES([CLUTTER_GTK], [clutter-gtk-1.0], , enable_clutter_gtk=yes)
  if test x$enable_clutter_gtk = xyes; then
    AC_DEFINE([HAVE_CLUTTER_GTK], [1], [Define if Clutter-Gtk is found])
  fi
fi
AC_MSG_RESULT($enable_clutter_gtk)
AM_CONDITIONAL(ENABLE_CLUTTER_GTK, [test x$enable_clutter_gtk = xyes])

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
bindings/python/Makefile
bindings/python/eekboard/Makefile
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
examples/eekboard-inscript/Makefile
examples/simple-client/Makefile
eek/eek-${EEK_API_VERSION}.pc
eek/eek-clutter-${EEK_API_VERSION}.pc
eek/eek-gtk-${EEK_API_VERSION}.pc
eek/eek-xkb-${EEK_API_VERSION}.pc
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
  Build Clutter UI          $enable_clutter
  Build Vala binding        $enable_vala
  Build Python binding      $enable_python
  Build document            $enable_gtk_doc
  Support accessibility     $enable_atspi
])
