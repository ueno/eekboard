# Copyright (C) 2011 Daiki Ueno <ueno@unixuser.org>
# Copyright (C) 2011 Red Hat, Inc.

# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see
# <http://www.gnu.org/licenses/>.

from gi.repository import Gio
import gi.repository
import gobject
from context import Context

class Eekboard(gobject.GObject):
    __gtype_name__ = "PYEekboardEekboard"
    __gsignals__ = {
        'destroyed': (
            gobject.SIGNAL_RUN_LAST,
            gobject.TYPE_NONE,
            ())
        }

    def __init__(self):
        super(Eekboard, self).__init__()
        self.__connection = Gio.bus_get_sync(Gio.BusType.SESSION, None)
        self.__eekboard = gi.repository.Eekboard.Eekboard.new(self.__connection, None);
        self.__eekboard.connect('destroyed', lambda *args: self.emit('destroyed'))

    def create_context(self, client_name):
        context = self.__eekboard.create_context(client_name, None)
        return Context(context)

    def push_context(self, context):
        self.__eekboard.push_context(context.get_giobject(), None)

    def pop_context(self):
        self.__eekboard.pop_context(None)

    def destroy_context(self, context):
        self.__eekboard.destroy_context(context.get_giobject(), None)
