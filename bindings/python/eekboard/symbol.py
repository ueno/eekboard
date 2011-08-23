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

import serializable

class Symbol(serializable.Serializable):
    __gtype_name__ = "PYEekSymbol"
    __NAME__ = "EekSymbol"

    def __init__(self):
        super(Symbol, self).__init__()

    name = property(lambda self: self.__name)
    label = property(lambda self: self.__label)
    category = property(lambda self: self.__category)
    modifier_mask = property(lambda self: self.__modifier_mask)
    icon_name = property(lambda self: self.__icon_name)

    def serialize(self, struct):
        super(Symbol, self).serialize(struct)
        struct.append(dbus.String(self.__name))
        struct.append(dbus.String(self.__label))
        struct.append(dbus.UInt32(self.__category))
        struct.append(dbus.UInt32(self.__modifier_mask))
        struct.append(dbus.String(self.__icon_name))

    def deserialize(self, struct):
        super(Symbol, self).deserialize(struct)
        self.__name = struct.pop(0)
        self.__label = struct.pop(0)
        self.__category = struct.pop(0)
        self.__modifier_mask = struct.pop(0)
        self.__icon_name = struct.pop(0)
