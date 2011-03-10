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

import sys, os.path
import sys
import re

import gtk, gtk.gdk
import eekboard
from gi.repository import Eek, GLib

if len(sys.argv) < 3:
    print >> sys.stderr, "Usage: %s KEYBOARD M17N-MAP..." % sys.argv[0]
    sys.exit(-1)

MAPENTRY_PATTERN = re.compile(r'\A\s*\((?:\((.*?)\)|"(.*?)")\s*"(.*?)"\)')
KEYCODE_TABLE = {
    '1': 10,
    '2': 11,
    '3': 12,
    '4': 13,
    '5': 14,
    '6': 15,
    '7': 16,
    '8': 17,
    '9': 18,
    '0': 19,
    '-': 20,
    '=': 21,
    'q': 24,
    'w': 25,
    'e': 26,
    'r': 27,
    't': 28,
    'y': 29,
    'u': 30,
    'i': 31,
    'o': 32,
    'p': 33,
    '[': 34,
    ']': 35,
    'a': 38,
    's': 39,
    'd': 40,
    'f': 41,
    'g': 42,
    'h': 43,
    'j': 44,
    'k': 45,
    'l': 46,
    ';': 47,
    '\'': 48,
    '`': 49,
    '\\': 51,
    'z': 52,
    'x': 53,
    'c': 54,
    'v': 55,
    'b': 56,
    'n': 57,
    'm': 58,
    ',': 59,
    '.': 60,
    '/': 61
}

MARK_UPPER = '~!@#$%^&*()_+{}|:"<>?'
MARK_LOWER = '`1234567890-=[]\\;\',./'

class MapFile(object):
    def __init__(self, path):
        self.__dict = dict()

        with open(path, 'r') as fp:
            for line in fp:
                match = re.match(MAPENTRY_PATTERN, line)
                if match:
                    insert = match.group(3).decode('UTF-8')
                    if match.group(1):
                        keyseq = re.sub(r'\\(.)', r'\1', match.group(1))
                        self.__add_symbol_entry(keyseq, insert)
                    else:
                        keyseq = re.sub(r'\\(.)', r'\1', match.group(2))
                        self.__add_text_entry(keyseq, insert)

    def get_entry_for_keycode(self, keycode):
        return self.__dict.get(keycode, list([None, None, None, None]))

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

files = list()
for path in sys.argv[2:]:
    files.append(MapFile(path))

keyboard = eekboard.XmlKeyboard(os.path.abspath(sys.argv[1]))

def each_key(element, data):
    keycode = element.get_keycode()
    matrix = Eek.SymbolMatrix.new(len(files) + 1, 4)
    for l in xrange(4):
        keysym = element.get_symbol_at_index(0, l, 0, 0)
        matrix.set_symbol(0, l, keysym)
    for g, f in enumerate(files):
        entry = f.get_entry_for_keycode(keycode)
        for l in xrange(4):
            keysym = None
            if entry[l]:
                try:
                    keyval = gtk.gdk.unicode_to_keyval(ord(entry[l]))
                    keysym = Eek.Keysym.new(keyval)
                except:
                    pass
            if not keysym:
                keysym = element.get_symbol_at_index(g + 1, l, 0, 0)
            matrix.set_symbol(g + 1, l, keysym)
    element.set_symbol_matrix(matrix)

def each_section(element, data):
    element.foreach_child(each_key, None)

keyboard.foreach_child(each_section, None)
output = GLib.string_sized_new(4096)
keyboard.output(output, 0)
print output.str
