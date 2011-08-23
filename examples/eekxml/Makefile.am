bin_SCRIPTS = eekxml
EXTRA_DIST = mim2remap

eekxml: eekxml.in
	$(AM_V_GEN) sed '1s!@''PYTHON@!'$(PYTHON)'!' $< > $@
