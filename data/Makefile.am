SUBDIRS = icons themes keyboards

@GSETTINGS_RULES@
@INTLTOOL_XML_NOMERGE_RULE@
gsettings_schemas_in_files = org.fedorahosted.eekboard.gschema.xml.in
gsettings_SCHEMAS = $(gsettings_schemas_in_files:.gschema.xml.in=.gschema.xml)

servicedir       = $(datadir)/dbus-1/services
service_in_files = eekboard-server.service.in
service_DATA     = $(service_in_files:.service.in=.service)

$(service_DATA): $(service_in_files) Makefile
	$(AM_V_GEN) sed -e "s|\@bindir\@|$(bindir)|" $< > $@

desktopdir = $(datadir)/applications
desktop_in_files = eekboard.desktop.in
desktop_DATA = $(desktop_in_files:.desktop.in=.desktop)

if ENABLE_ATSPI
autostartdir	 = $(sysconfdir)/xdg/autostart
autostart_in_files = eekboard-autostart.desktop.in
autostart_DATA   = $(autostart_in_files:.desktop.in=.desktop)
endif

@INTLTOOL_DESKTOP_RULE@

CLEANFILES = $(service_DATA) $(desktop_DATA) $(gsettings_SCHEMAS)
EXTRA_DIST = $(service_in_files) $(desktop_in_files) $(gsettings_schemas_in_files)

if ENABLE_ATSPI
CLEANFILES += $(autostart_DATA)
EXTRA_DIST += $(autostart_in_files)
endif

