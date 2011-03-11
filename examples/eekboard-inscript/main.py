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

import inscript
import gtk
from optparse import OptionParser
import sys, os, os.path, glob

parser = OptionParser()
parser.add_option("-n", "--name=LANGCODE", dest="langcode",
                  help="Specify language code to LANGCODE",
                  metavar="LANGCODE")
parser.add_option("-l", "--list", dest="list", default=False,
                  action="store_true",
                  help="List available language codes")
(options, args) = parser.parse_args()

if options.list:
    pat = os.path.join(os.getenv("M17N_DIR"), "*.mim")
    for fname in sorted(glob.glob(pat)):
        mname = os.path.basename(fname[:-4])
        if mname in inscript.INSCRIPT_MAPS:
            print mname
    exit(0)

if options.langcode is None:
    print >> sys.stderr, "Specify language code with -n"
    exit(1)

map_path = os.path.join(os.getenv("M17N_DIR"), options.langcode + ".mim")
if not os.path.exists(map_path):
    print >> sys.stderr, "%s not found" % map_path
    exit(1)

kbd_path = os.path.join(os.getenv("EEKBOARD_KEYBOARDDIR"), "us-qwerty.xml")
if not os.path.exists(kbd_path):
    print >> sys.stderr, "%s not found" % kbd_path
    exit(1)

keyboard = inscript.Keyboard("eekboard-inscript", map_path, kbd_path)
keyboard.connect('quit', lambda *args: gtk.main_quit())
keyboard.set_group(1)
keyboard.enable()
keyboard.show()
gtk.main()
