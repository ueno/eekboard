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

from gi.repository import Eek, EekXkl, Gio

from eekboard import Eekboard
from context import Context

Keyboard = Eek.Keyboard
Section = Eek.Section
Key = Eek.Key
Symbol = Eek.Symbol
Keysym = Eek.Keysym

MODIFIER_BEHAVIOR_NONE, \
MODIFIER_BEHAVIOR_LOCK, \
MODIFIER_BEHAVIOR_LATCH = \
(Eek.ModifierBehavior.NONE,
 Eek.ModifierBehavior.LOCK,
 Eek.ModifierBehavior.LATCH)

CSW = 640
CSH = 480

def XmlKeyboard(path, modifier_behavior=MODIFIER_BEHAVIOR_NONE):
    _file = Gio.file_new_for_path(path)
    layout = Eek.XmlLayout.new(_file.read())
    keyboard = Eek.Keyboard.new(layout, CSW, CSH)
    keyboard.set_modifier_behavior(modifier_behavior)
    keyboard.set_alt_gr_mask(Eek.ModifierType.MOD5_MASK)
    return keyboard

def XklKeyboard(modifier_behavior=MODIFIER_BEHAVIOR_NONE):
    layout = EekXkl.Layout.new()
    keyboard = Eek.Keyboard.new(layout, CSW, CSH)
    keyboard.set_modifier_behavior(modifier_behavior)
    return keyboard

__all__ = ['Eekboard',
           'Context',
           'Keyboard',
           'Section',
           'Key',
           'Symbol',
           'Keysym',
           'MODIFIER_BEHAVIOR_NONE',
           'MODIFIER_BEHAVIOR_LOCK',
           'MODIFIER_BEHAVIOR_LATCH',
           'XmlKeyboard',
           'XklKeyboard']
