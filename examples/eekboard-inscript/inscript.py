#!/usr/bin/env python

# Copyright (C) 2011 Daiki Ueno <ueno@unixuser.org>
# Copyright (C) 2011 Red Hat, Inc.

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

import gobject, gtk, eekboard, virtkey
import sys, os.path, re

KEYCODE_TABLE = {
    '1': 10, '2': 11, '3': 12, '4': 13, '5': 14, '6': 15, '7': 16, '8': 17,
    '9': 18, '0': 19, '-': 20, '=': 21, 'q': 24, 'w': 25, 'e': 26, 'r': 27,
    't': 28, 'y': 29, 'u': 30, 'i': 31, 'o': 32, 'p': 33, '[': 34, ']': 35,
    'a': 38, 's': 39, 'd': 40, 'f': 41, 'g': 42, 'h': 43, 'j': 44, 'k': 45,
    'l': 46, ';': 47, '\'': 48, '`': 49, '\\': 51, 'z': 52, 'x': 53, 'c': 54,
    'v': 55, 'b': 56, 'n': 57, 'm': 58, ',': 59, '.': 60, '/': 61
}

MARK_UPPER = '~!@#$%^&*()_+{}|:"<>?'
MARK_LOWER = '`1234567890-=[]\\;\',./'

class MapFile(object):
    MAPENTRY_PATTERN = re.compile(r'\A\s*\((?:\((.*?)\)|"(.*?)")\s*"(.*?)"\)')

    def __init__(self, path):
        self.__dict = dict()

        with open(path, 'r') as fp:
            for line in fp:
                match = re.match(self.MAPENTRY_PATTERN, line)
                if match:
                    insert = match.group(3).decode('UTF-8')
                    if match.group(1):
                        keyseq = re.sub(r'\\(.)', r'\1', match.group(1))
                        self.__add_symbol_entry(keyseq, insert)
                    else:
                        keyseq = re.sub(r'\\(.)', r'\1', match.group(2))
                        self.__add_text_entry(keyseq, insert)

    def get_entry_for_keycode(self, keycode):
        return self.__dict.get(keycode)

    def __add_entry(self, letter, level, insert):
        if letter.isupper():
            level |= 1
            letter = letter.lower()
        elif letter in MARK_UPPER:
            level |= 1
            letter = MARK_LOWER[MARK_UPPER.index(letter)]
        keycode = KEYCODE_TABLE[letter]
        if keycode not in self.__dict:
            self.__dict[keycode] = list([None,None,None,None])
        self.__dict[keycode][level] = insert
        
    def __add_symbol_entry(self, symbol, insert):
        level = 0
        if symbol.startswith('G-'):
            level |= 2
            symbol = symbol[2:]
        if not symbol.startswith('KP_'):
            self.__add_entry(symbol, level, insert)

    def __add_text_entry(self, text, insert):
        self.__add_entry(text, 0, insert)

class Keyboard(gobject.GObject):
    __gtype_name__ = "PYInscriptKeyboard"
    __gsignals__ = {
        'quit': (
            gobject.SIGNAL_RUN_LAST,
            gobject.TYPE_NONE,
            ()),
        }

    def __init__(self, client_name, map_path, kbd_path):
        self.__keyboard = self.__create_keyboard(map_path, kbd_path)
        self.__eekboard = eekboard.Eekboard()
        self.__context = self.__eekboard.create_context(client_name)
        keyboard_id = self.__context.add_keyboard(self.__keyboard)
        self.__context.set_keyboard(keyboard_id)
        self.__keyboard.connect('key-pressed', self.__key_pressed_cb)
        self.__keyboard.connect('key-released', self.__key_released_cb)
        self.__virtkey = virtkey.virtkey()
        self.__english = False
        self.__eekboard.connect('destroyed', self.__destroyed_cb)
        self.__context.connect('destroyed', self.__destroyed_cb)
        self.__context.connect('notify::keyboard-visible', self.__notify_keyboard_visible_cb)

    def __create_keyboard(self, map_path, kbd_path):
        def __each_key(element, data):
            keycode = element.get_keycode()
            # keycode 37 is used to toggle English/Inscript
            if keycode == 37:
                matrix = eekboard.SymbolMatrix.new(2, 1)
                keysym = eekboard.Keysym.new(0)
                keysym.set_label("Ind")
                keysym.set_category(eekboard.SymbolCategory.FUNCTION)
                matrix.set_symbol(0, 0, keysym)
                keysym = eekboard.Keysym.new(0)
                keysym.set_label("Eng")
                keysym.set_category(eekboard.SymbolCategory.FUNCTION)
                matrix.set_symbol(1, 0, keysym)
                element.set_symbol_matrix(matrix)
                return

            # group(0) is us keyboard
            matrix = eekboard.SymbolMatrix.new(2, 4)
            for l in xrange(4):
                keysym = element.get_symbol_at_index(0, l, 0, 0)
                matrix.set_symbol(0, l, keysym)
            # group(1) is inscript keyboard
            entry = data.get_entry_for_keycode(keycode)
            for l in xrange(4):
                keysym = None
                if entry and entry[l]:
                    try:
                        keyval = gtk.gdk.unicode_to_keyval(ord(entry[l]))
                        keysym = eekboard.Keysym.new(keyval)
                    except:
                        print >> sys.stderr, "can't convert %s to keyval" % entry[l]
                if not keysym:
                    keysym = element.get_symbol_at_index(1, l, 0, 0)
                matrix.set_symbol(1, l, keysym)
            element.set_symbol_matrix(matrix)

        def __each_section(element, data):
            element.foreach_child(__each_key, data)

        mapfile = MapFile(map_path)
        keyboard = eekboard.XmlKeyboard(kbd_path,
                                        eekboard.MODIFIER_BEHAVIOR_LATCH)
        keyboard.foreach_child(__each_section, mapfile)
        return keyboard

    def __destroyed_cb(self, *args):
        # self.emit('quit')
        gtk.main_quit()

    def __notify_keyboard_visible_cb(self, obj, pspec):
        if not obj.get_property(pspec.name):
            # self.emit('quit')
            gtk.main_quit()

    def enable(self):
        self.__eekboard.push_context(self.__context)

    def disable(self):
        self.__eekboard.pop_context(self.__context)

    def show(self):
        self.__context.show_keyboard()

    def set_group(self, group):
        self.__group = group
        self.__context.set_group(self.__group)

    def __key_pressed_cb(self, keyboard, key):
        if key.get_keycode() == 37:
            return
        symbol = key.get_symbol()
        if isinstance(symbol, eekboard.Keysym):
            xkeysym = symbol.get_xkeysym()
            modifiers = self.__keyboard.get_modifiers()
            self.__virtkey.latch_mod(modifiers)
            self.__virtkey.press_keysym(xkeysym)
            self.__virtkey.unlatch_mod(modifiers)

    def __key_released_cb(self, keyboard, key):
        if key.get_keycode() == 37:
            if self.__english:
                self.__context.set_group(self.__group)
                self.__english = False
            else:
                self.__context.set_group(0)
                self.__english = True
            return
        symbol = key.get_symbol()
        if isinstance(symbol, eekboard.Keysym):
            xkeysym = symbol.get_xkeysym()
            self.__virtkey.release_keysym(xkeysym)
