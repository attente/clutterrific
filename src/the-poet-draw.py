#!/usr/bin/env python
#
# Clutterrific
# Copyright (C) 2010 William Hua
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of the
# License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
# 02110-1301, USA.



# Usage for generating lookup tables:
# 1. generate resizable control points:
#    $ python the-poet-draw.py < input.svg > output.svg
# 2. edit sizes of control points:
#    $ inkscape output.svg
# 3. generate lookup tables:
#    $ python the-poet-scan.py radii timeline < output.svg
#
# Usage for generating fonts from lookup tables:
# $ python the-poet-draw.py radii < input.svg | \
#   python the-poet-scan.py timeline
#
# Command line:
# python the-poet-draw.py [<radii>]
# radii - file with a dictionary mapping strings (circle ids) to
#         floats (radii)
#
# Standard input:
# SVG of groups (glyphs) of paths (curves)
#
# Standard output:
# SVG augmented with red circles (origins), orange circles (anchors),
# and groups of blue circles (control points)



import re
import sys
import xml.dom.minidom



def main (argv):
  radii = {}

  if len (argv) == 2:
    with open (argv[1]) as f:
      radii = eval (f.read ())

  svg = xml.dom.minidom.parseString (sys.stdin.read ())

  for group in svg.getElementsByTagName ('g'):
    paths = [p for p in group.childNodes if p.localName == 'path']
    paths.sort (None, lambda p: p.getAttribute ('id'))

    if not paths:
      continue

    pattern = '\s*[Mm]\s*(-?\d+(?:\.\d+)?(?:[Ee][+-]?\d+)?)\s*,\s*(-?\d+(?:\.\d+)?(?:[Ee][+-]?\d+)?)'
    x0, y0 = re.match (pattern, paths[0].getAttribute ('d')).groups ()
    x0, y0 = float (x0), float (y0)
    x1, y1 = 0, 0

    for path in paths:
      subgroup = svg.createElementNS ('svg', 'g')
      subgroup.setAttribute ('id', '%s__' % path.getAttribute ('id'))
      subgroup.setAttribute ('transform', group.getAttribute ('transform'))

      relative = False
      previous = [0, 0]

      i = 0

      pattern = '([CMcm]?)\s*(-?\d+(?:\.\d+)?(?:[Ee][+-]?\d+)?)\s*,\s*(-?\d+(?:\.\d+)?(?:[Ee][+-]?\d+)?)'
      commands = re.findall (pattern, path.getAttribute ('d'))

      for op, ox, oy in commands:
        x = float (ox)
        y = float (oy)

        if op:
          relative = op.islower ()

        if relative:
          x += previous[0]
          y += previous[1]

        name = '%s_%d' % (path.getAttribute ('id'), i)
        radius = radii[name] if name in radii else 2

        circle = svg.createElementNS ('svg', 'circle')
        circle.setAttribute ('id', name)
        circle.setAttribute ('cx', str (x))
        circle.setAttribute ('cy', str (y))
        circle.setAttribute ('r', str (radius))
        circle.setAttribute ('fill', 'blue')
        subgroup.appendChild (circle)

        if not relative or i % 3 is 0:
          previous[0] = x
          previous[1] = y

        x1 = x
        y1 = y

        i += 1

      group.parentNode.appendChild (subgroup)

    circle = svg.createElementNS ('svg', 'circle')
    circle.setAttribute ('id', '%s_0' % group.getAttribute ('id'))
    circle.setAttribute ('transform', group.getAttribute ('transform'))
    circle.setAttribute ('fill', 'red')
    circle.setAttribute ('cx', str (x0))
    circle.setAttribute ('cy', str (y0))
    circle.setAttribute ('r', '1')
    group.parentNode.appendChild (circle)

    name = '%s_1' % group.getAttribute ('id')

    anchor = svg.createElementNS ('svg', 'circle')
    anchor.setAttribute ('id', name)
    anchor.setAttribute ('transform', group.getAttribute ('transform'))
    anchor.setAttribute ('fill', 'orange')
    anchor.setAttribute ('r', '1')

    if name in radii:
      anchor.setAttribute ('cx', str (x0 + (radii[name])[0]))
      anchor.setAttribute ('cy', str (y0 + (radii[name])[1]))
    else:
      anchor.setAttribute ('cx', str (x1))
      anchor.setAttribute ('cy', str (y1))

    group.parentNode.appendChild (anchor)

  print svg.toprettyxml ()



if __name__ == '__main__':
  main (sys.argv)
