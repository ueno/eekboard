SUBDIRS = icons themes keyboards

desktopdir = $(datadir)/applications
desktop_in_files = eekboard.desktop.in
desktop_DATA = $(desktop_in_files:.desktop.in=.desktop)

if ENABLE_ATSPI
autostartdir	 = $(sysconfdir)/xdg/autostart
autostart_in_files = eekboard-autostart.desktop.in
autostart_DATA   = $(autostart_in_files:.desktop.in=.desktop)
endif

@INTLTOOL_DESKTOP_RULE@

CLEANFILES = $(desktop_DATA)
EXTRA_DIST = $(desktop_in_files)

if ENABLE_ATSPI
CLEANFILES += $(autostart_DATA)
EXTRA_DIST += $(autostart_in_files)
endif

