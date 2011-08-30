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

class Text(symbol.Symbol):
    __gtype_name__ = "PYEekText"
    __NAME__ = "EekText"

    def __init__(self):
        super(Text, self).__init__()

    text = property(lambda self: self.__text)

    def serialize(self, struct):
        super(Text, self).serialize(struct)
        struct.append(dbus.String(self.__text))

    def deserialize(self, struct):
        super(Text, self).deserialize(struct)
        self.__text = struct.pop(0)
