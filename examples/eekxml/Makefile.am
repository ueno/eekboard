bin_SCRIPTS = eekxml
EXTRA_DIST = eekxml.in mim2remap.el
DISTCLEANFILES = eekxml

eekxml: eekxml.in
	$(AM_V_GEN) sed '1s!@''PYTHON@!'$(PYTHON)'!' $< > $@
