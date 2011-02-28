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

from gi.repository import Eekboard
import gobject

class Context(gobject.GObject):
    __gtype_name__ = "PYEekboardContext"
    __gsignals__ = {
        'enabled': (
            gobject.SIGNAL_RUN_LAST,
            gobject.TYPE_NONE,
            ()),
        'disabled': (
            gobject.SIGNAL_RUN_LAST,
            gobject.TYPE_NONE,
            ()),
        'key-pressed': (
            gobject.SIGNAL_RUN_LAST,
            gobject.TYPE_NONE,
            (gobject.TYPE_UINT,)),
        'key-released': (
            gobject.SIGNAL_RUN_LAST,
            gobject.TYPE_NONE,
            (gobject.TYPE_UINT,))
        }

    def __init__(self, giobject):
        super(Context, self).__init__()
        import sys
        self.__giobject = giobject
        self.__giobject.connect('enabled', lambda *args: self.emit('enabled'))
        self.__giobject.connect('disabled', lambda *args: self.emit('disabled'))
        self.__giobject.connect('key-pressed', lambda *args: self.emit('key-pressed', args[1]))
        self.__giobject.connect('key-released', lambda *args: self.emit('key-released', args[1]))

    def get_giobject(self):
        return self.__giobject

    def add_keyboard(self, keyboard):
        return self.__giobject.add_keyboard(keyboard, None)

    def remove_keyboard(self, keyboard_id):
        return self.__giobject.remove_keyboard(keyboard_id, None)
        
    def set_keyboard(self, keyboard_id):
        self.__giobject.set_keyboard(keyboard_id, None)

    def show_keyboard(self):
        self.__giobject.show_keyboard(None)

    def hide_keyboard(self):
        self.__giobject.hide_keyboard(None)

    def set_group(self, group):
        self.__giobject.set_group(group, None)

    def press_key(self, keycode):
        self.__giobject.press_key(keycode, None)

    def release_key(self, keycode):
        self.__giobject.release_key(keycode, None)

    def is_enabled(self):
        return self.__giobject.is_enabled()
