#!/usr/bin/env python

# Copyright (C) 2010-2011 Daiki Ueno <ueno@unixuser.org>
# Copyright (C) 2010-2011 Red Hat, Inc.

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

import sys
import re

if len(sys.argv) != 2:
    print >> sys.stderr, "Usage: %s TABLE-NAME" % sys.argv[0]
    sys.exit(-1)

table = dict()
for line in sys.stdin:
    line = line.decode('UTF-8')
    match = re.match(r'\s*(0x[0-9A-F]+)\s+(\S*)\s+(\S*)', line, re.I)
    if match:
        table[int(match.group(1), 16)] = (match.group(2), match.group(3))

sys.stdout.write("static const EekKeysymEntry %s[] = {\n" %
                 sys.argv[1])

for index, (keysym, (l, c)) in enumerate([(keysym, table[keysym])
                                          for keysym in sorted(table.keys())]):
    sys.stdout.write("    { 0x%X, %s, %s }" %
                     (keysym, l.encode('UTF-8'), c.encode('UTF-8')))
    if index < len(table) - 1:
        sys.stdout.write(",")
    sys.stdout.write("\n")
sys.stdout.write("};\n")
