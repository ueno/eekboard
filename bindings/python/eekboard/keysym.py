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

import symbol

class Keysym(symbol.Symbol):
    __gtype_name__ = "PYEekKeysym"
    __NAME__ = "EekKeysym"

    def __init__(self):
        super(Keysym, self).__init__()

    xkeysym = property(lambda self: self.xkeysym)

    def serialize(self, struct):
        super(Keysym, self).serialize(struct)
        struct.append(dbus.UInt32(self.__xkeysym))

    def deserialize(self, struct):
        super(Keysym, self).deserialize(struct)
        self.__xkeysym = struct.pop(0)
