bin_SCRIPTS = eekboard-inscript
keyboarddir = $(pkgdatadir)/keyboards

eekboard_inscript_datadir = $(datarootdir)/eekboard-inscript
eekboard_inscript_data_PYTHON = inscript.py main.py

eekboard-inscript: eekboard-inscript.in
	$(AM_V_GEN) sed -e 's!@''PYTHON@!'$(PYTHON)'!' \
			-e 's!@EEKBOARD_KEYBOARDDIR@!'$(keyboarddir)'!' \
			-e 's!@M17N_DIR@!'$(datadir)/m17n'!' \
			-e 's!@EEKBOARD_INSCRIPT_DATADIR@!'$(eekboard_inscript_datadir)'!' < $< > $@

CLEANFILES = eekboard-inscript
EXTRA_DIST = eekboard-inscript.in
