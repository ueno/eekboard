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

import dbus
import dbus.mainloop.glib
import gobject
from context import Context

dbus.mainloop.glib.DBusGMainLoop(set_as_default = True)

class Client(gobject.GObject):
    __gtype_name__ = "PYEekboardClient"
    __gsignals__ = {
        'destroyed': (
            gobject.SIGNAL_RUN_LAST,
            gobject.TYPE_NONE,
            ())
        }

    def __init__(self):
        super(Client, self).__init__()
        self.__bus = dbus.SessionBus()
        _service = self.__bus.get_object("org.fedorahosted.Eekboard",
                                         "/org/fedorahosted/Eekboard")
        self.__service = dbus.Interface(_service, dbus_interface="org.fedorahosted.Eekboard")
        self.__service.connect_to_signal("Destroyed", self.__destroyed_cb)

    def __destroyed_cb(self):
        self.emit("destroyed")

    def create_context(self, client_name):
        object_path = self.__service.CreateContext(client_name)
        return Context(self.__bus, object_path)

    def push_context(self, context):
        self.__service.PushContext(context.object_path)

    def pop_context(self):
        self.__service.PopContext()

    def destroy_context(self, context):
        self.__service.DestroyContext(context.object_path)
