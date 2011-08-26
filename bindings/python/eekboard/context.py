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
import gobject
import serializable

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
            (gobject.TYPE_STRING, gobject.TYPE_PYOBJECT, gobject.TYPE_UINT)),
        'destroyed': (
            gobject.SIGNAL_RUN_LAST,
            gobject.TYPE_NONE,
            ()),
        }

    __gproperties__ = {
        'visible': (gobject.TYPE_BOOLEAN, 'Visible', 'Visible',
                    False, gobject.PARAM_READWRITE),
        'keyboard': (gobject.TYPE_UINT, 'Keyboard', 'Keyboard',
                     0, gobject.G_MAXUINT, 0, gobject.PARAM_READWRITE),
        'group': (gobject.TYPE_UINT, 'Group', 'Group',
                  0, gobject.G_MAXUINT, 0, gobject.PARAM_READWRITE),
        }

    def __init__(self, bus, object_path):
        super(Context, self).__init__()
        self.__bus = bus
        self.__object_path = object_path
        self.__properties = {}
        _context = self.__bus.get_object("org.fedorahosted.Eekboard",
                                         object_path)
        self.__context = dbus.Interface(_context, dbus_interface="org.fedorahosted.Eekboard.Context")

        self.__context.connect_to_signal('Enabled', self.__enabled_cb)
        self.__context.connect_to_signal('Disabled', self.__disabled_cb)
        self.__context.connect_to_signal('KeyPressed', self.__key_pressed_cb)
        self.__context.connect_to_signal('Destroyed', self.__destroyed_cb)
        self.__context.connect_to_signal('VisibilityChanged', self.__visibility_changed_cb)
        self.__context.connect_to_signal('KeyboardChanged', self.__keyboard_changed_cb)
        self.__context.connect_to_signal('GroupChanged', self.__group_changed_cb)

    object_path = property(lambda self: self.__object_path)

    def __enabled_cb(self):
        self.emit('enabled')

    def __disabled_cb(self):
        self.emit('disabled')

    def __key_pressed_cb(self, *args):
        keyname = args[0]
        symbol = serializable.deserialize_object(args[1])
        modifiers = args[2]
        self.emit('key-pressed', keyname, symbol, modifiers)

    def __visibility_changed_cb(self, *args):
        self.set_property('visible', args[0])
        self.notify('visible')

    def __keyboard_changed_cb(self, *args):
        self.set_property('keyboard', args[0])
        self.notify('keyboard')

    def __group_changed_cb(self, *args):
        self.set_property('group', args[0])
        self.notify('group')

    def __destroyed_cb(self):
        self.emit("destroyed")

    def do_set_property(self, pspec, value):
        self.__properties[pspec.name] = value

    def do_get_property(self, pspec):
        return self.__properties.get(pspec.name, pspec.default_value)

    def add_keyboard(self, keyboard_type):
        return self.__context.AddKeyboard(keyboard_type)

    def remove_keyboard(self, keyboard_id):
        return self.__context.RemoveKeyboard(keyboard_id)
        
    def set_keyboard(self, keyboard_id):
        self.__context.SetKeyboard(keyboard_id)

    def show_keyboard(self):
        self.__context.ShowKeyboard()

    def hide_keyboard(self):
        self.__context.HideKeyboard()

    def set_group(self, group):
        self.__context.SetGroup(group)

    def press_keycode(self, keycode):
        self.__context.PressKeycode(keycode)

    def release_keycode(self, keycode):
        self.__context.ReleaseKeycode(keycode)
