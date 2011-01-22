# Copyright (C) 2010 Daiki Ueno <ueno@unixuser.org>
# Copyright (C) 2010 Red Hat, Inc.

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

vapidir = $(datadir)/vala/vapi
dist_vapi_DATA = \
	eek-$(EEK_API_VERSION).vapi \
	eek-clutter-$(EEK_API_VERSION).vapi \
	eek-gtk-$(EEK_API_VERSION).vapi \
	eek-xkb-$(EEK_API_VERSION).vapi \
	eek-xkl-$(EEK_API_VERSION).vapi
MAINTAINERCLEANFILES = $(dist_vapi_DATA)

eek-$(EEK_API_VERSION).vapi:
	vapigen --library eek-$(EEK_API_VERSION) \
		eek-$(EEK_API_VERSION)/eek-$(EEK_API_VERSION).gi

eek-clutter-$(EEK_API_VERSION).vapi: eek-$(EEK_API_VERSION).vapi
	vapigen --vapidir=$(builddir) --library eek-clutter-$(EEK_API_VERSION) \
		--pkg eek-$(EEK_API_VERSION) --pkg clutter-1.0 \
		eek-clutter-$(EEK_API_VERSION)/eek-clutter-$(EEK_API_VERSION).gi

eek-gtk-$(EEK_API_VERSION).vapi: eek-$(EEK_API_VERSION).vapi
	vapigen --vapidir=$(builddir) --library eek-gtk-$(EEK_API_VERSION) \
		--pkg eek-$(EEK_API_VERSION) --pkg gtk+-2.0 \
		eek-gtk-$(EEK_API_VERSION)/eek-gtk-$(EEK_API_VERSION).gi

eek-xkb-$(EEK_API_VERSION).vapi: eek-$(EEK_API_VERSION).vapi
	vapigen --vapidir=$(builddir) --library eek-xkb-$(EEK_API_VERSION) \
		--pkg eek-$(EEK_API_VERSION) \
		eek-xkb-$(EEK_API_VERSION)/eek-xkb-$(EEK_API_VERSION).gi

eek-xkl-$(EEK_API_VERSION).vapi: eek-xkb-$(EEK_API_VERSION).vapi
	vapigen --vapidir=$(builddir) --library eek-xkl-$(EEK_API_VERSION) \
		--pkg eek-$(EEK_API_VERSION) --pkg eek-xkb-$(EEK_API_VERSION) \
		eek-xkl-$(EEK_API_VERSION)/eek-xkl-$(EEK_API_VERSION).gi
