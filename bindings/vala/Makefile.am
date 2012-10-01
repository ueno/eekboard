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

NULL =

if ENABLE_VALA
vapidir = $(datadir)/vala/vapi
dist_vapi_DATA =				\
	eek-$(EEK_API_VERSION).vapi		\
	eek-$(EEK_API_VERSION).deps		\
	eek-gtk-$(EEK_API_VERSION).vapi		\
	eek-gtk-$(EEK_API_VERSION).deps		\
	eek-xkl-$(EEK_API_VERSION).vapi		\
	eek-xkl-$(EEK_API_VERSION).deps		\
	$(NULL)

EXTRA_DIST =					\
	Eek-$(EEK_API_VERSION).metadata		\
	EekGtk-$(EEK_API_VERSION).metadata	\
	EekXkl-$(EEK_API_VERSION).metadata	\
	$(NULL)

GITIGNOREFILES = \
	eek-$(EEK_API_VERSION).vapi		\
	eek-gtk-$(EEK_API_VERSION).vapi		\
	eek-xkl-$(EEK_API_VERSION).vapi		\
	$(NULL)

maintainer-clean-local:
	rm -f *.vapi

eek_vapi_deps =						\
	$(srcdir)/Eek-$(EEK_API_VERSION).metadata	\
	|						\
	$(top_builddir)/eek/Eek-$(EEK_API_VERSION).gir	\
	$(NULL)

eek-$(EEK_API_VERSION).vapi: $(eek_vapi_deps)
	$(VAPIGEN_V)$(VAPIGEN) \
		--library eek-$(EEK_API_VERSION) \
		--pkg gio-2.0 \
		--metadatadir=$(srcdir) \
		$(top_builddir)/eek/Eek-$(EEK_API_VERSION).gir

eek_gtk_vapi_deps =					\
	$(srcdir)/EekGtk-$(EEK_API_VERSION).metadata	\
	|						\
	$(top_builddir)/eek/EekGtk-$(EEK_API_VERSION).gir	\
	$(NULL)

eek-gtk-$(EEK_API_VERSION).vapi: $(eek_gtk_vapi_deps)
	$(VAPIGEN_V)$(VAPIGEN) --vapidir=$(builddir) \
		--library eek-gtk-$(EEK_API_VERSION) \
		--pkg eek-$(EEK_API_VERSION) \
		--pkg gio-2.0 \
		--pkg gtk+-3.0 \
		--metadatadir=$(srcdir)	\
		$(top_builddir)/eek/EekGtk-$(EEK_API_VERSION).gir

eek_xkl_vapi_deps =						\
	$(srcdir)/EekXkl-$(EEK_API_VERSION).metadata		\
	|							\
	$(top_builddir)/eek/EekXkl-$(EEK_API_VERSION).gir	\
	$(NULL)

eek-xkl-$(EEK_API_VERSION).vapi: $(eek_xkl_vapi_deps)
	$(VAPIGEN_V)$(VAPIGEN) \
		--vapidir=$(builddir) \
		--library eek-xkl-$(EEK_API_VERSION) \
		--pkg eek-$(EEK_API_VERSION) \
		--pkg gio-2.0 \
		--metadatadir=$(srcdir) \
		$(top_builddir)/eek/EekXkl-$(EEK_API_VERSION).gir

# set up the verbosity rules to avoid some build noise
VAPIGEN_V = $(VAPIGEN_V_$(V))
VAPIGEN_V_ = $(VAPIGEN_V_$(AM_DEFAULT_VERBOSITY))
VAPIGEN_V_0 = @echo "  VAPIG " $@;
endif

-include $(top_srcdir)/git.mk
