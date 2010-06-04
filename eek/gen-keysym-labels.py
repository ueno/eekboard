#!/usr/bin/python

from __future__ import with_statement
import sys
import re

if len(sys.argv) != 3:
    print >> sys.stderr, "Usage: %s table.txt table-name" % sys.argv[0]
    sys.exit(-1)

table = dict()
with open(sys.argv[1], 'r') as fp:
    for line in fp:
        line = line.decode('UTF-8')
        match = re.match(r'\s*(0x[0-9A-F]+)\s+(\S*)', line, re.I)
        if match:
            table[int(match.group(1), 16)] = match.group(2)

sys.stdout.write("static const struct eek_keysym_label %s[] = {\n" %
                 sys.argv[2])

for index, (keysym, label) in enumerate([(keysym, table[keysym])
                                         for keysym in sorted(table.keys())]):
    sys.stdout.write("  { 0x%X, %s }" % (keysym, label.encode('UTF-8')))
    if index < len(table) - 1:
        sys.stdout.write(",")
    sys.stdout.write("\n")
sys.stdout.write("};\n")
