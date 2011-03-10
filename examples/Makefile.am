bin_SCRIPTS = eekboard-inscript
keyboarddir = $(pkgdatadir)/keyboards
dist_keyboard_DATA = inscript2.xml
EXTRA_DIST = gen-inscript-keyboard.py

eekboard-inscript: eekboard-inscript.in
	$(AM_V_GEN) sed 's!@EEKBOARD_INSCRIPT_PATH@!'$(pkgdatadir)/keyboards/inscript2.xml'!' < $< > $@

