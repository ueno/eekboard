SUBDIRS = icons themes keyboards

@GSETTINGS_RULES@
@INTLTOOL_XML_NOMERGE_RULE@
gsettings_schemas_in_files = org.fedorahosted.eekboard.gschema.xml.in
gsettings_SCHEMAS = $(gsettings_schemas_in_files:.gschema.xml.in=.gschema.xml)

desktopdir = $(datadir)/applications
desktop_in_files = eekboard.desktop.in
desktop_DATA = $(desktop_in_files:.desktop.in=.desktop)

if ENABLE_ATSPI
autostartdir	 = $(sysconfdir)/xdg/autostart
autostart_in_files = eekboard-autostart.desktop.in
autostart_DATA   = $(autostart_in_files:.desktop.in=.desktop)
endif

@INTLTOOL_DESKTOP_RULE@

CLEANFILES = $(desktop_DATA) $(gsettings_SCHEMAS)
EXTRA_DIST = $(desktop_in_files) $(gsettings_schemas_in_files)

if ENABLE_ATSPI
CLEANFILES += $(autostart_DATA)
EXTRA_DIST += $(autostart_in_files)
endif

