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
# python the-poet-scan.py <timeline>
# timeline - file with a dictionary mapping strings (curve ids) to
#            pairs of floats (stroke times, wait times)
#
# python the-poet-scan.py <radii> <timeline>
# radii    - location to store a dictionary mapping strings (circle
#            ids) to floats (radii)
# timeline - location to store a dictionary mapping strings (curve
#            ids) to pairs of floats (stroke times, wait times)
#
# Standard input:
# Augmented SVG edited by translating blue circles (anchors) and
# rescaling red circles (control points)
#
# Standard output:
# Dictionary (font) mapping strings (names) to pairs of lists of
# triples of floats (stroke times), floats (wait times) and triples
# of floats (starting cap ratios), floats (ending cap ratios) and
# lists (curves) of points (circles) and points (anchors)



import re
import sys
import xml.dom.minidom



def string (s):
  q = '\"' if '\'' in s else '\''

  s = ''.join (['\\' + c if c == '\\' or c == q else c for c in s])

  return '%s%s%s' % (q, s, q)



def output (font):
  text = ''
  comma = ['', '']

  for name, (glyph, anchor) in font.items ():
    text = '%s%s\n  %s:\n  (\n    [' % (text, comma[0], string (name))
    comma[1] = ''

    for curve in glyph:
      text = '%s%s\n      %s' % (text, comma[1], repr (curve))
      comma[1] = ','

    text = '%s\n    ],\n    (%f, %f)\n  )' % (text, anchor[0], anchor[1])

    comma[0] = ','

  return '{%s\n}' % text



def radii (sizes, anchors):
  text = ''
  comma = ''

  for key, anchor in anchors.items ():
    text = '%s%s\n  %s: (%f, %f)' % (text, comma, string (key), anchor[0], anchor[1])
    comma = ','

  for key, size in sizes.items ():
    text = '%s%s\n  %s: %f' % (text, comma, string (key), size)
    comma = ','

  return '{%s\n}' % text



def timeline (paths):
  text = ''
  comma = ''

  for path in paths:
    text = '%s%s\n  %s: (1.0, 0.0, 1.0, 1.0)' % (text, comma, string (path))
    comma = ','

  return '{%s\n}' % text



def special (name, namespace):
  i = name.rfind ('_')

  return i < 0 or name[:i] in namespace



def main (argv):
  data = {}

  if len (argv) == 2:
    with open (argv[1]) as f:
      data = eval (f.read ())

  svg = xml.dom.minidom.parseString (sys.stdin.read ())
  paths = svg.getElementsByTagName ('path')
  groups = svg.getElementsByTagName ('g')
  pathnames = set (p.getAttribute ('id') for p in paths)
  groupnames = set (g.getAttribute ('id') for g in groups)
  circles = svg.getElementsByTagName ('circle')
  circles = dict ((c.getAttribute ('id'), c) for c in circles)

  font    = {}
  anchors = {}

  for group in svg.getElementsByTagName ('g'):
    name = group.getAttribute ('id')
    ignore = name[-2:] == '__' and name[:-2] in pathnames
    ignore = ignore or '%s_0' % name not in circles
    ignore = ignore or '%s_1' % name not in circles

    if ignore:
      continue

    origin = circles['%s_0' % name]
    x0 = float (origin.getAttribute ('cx'))
    y0 = float (origin.getAttribute ('cy'))
    t0 = origin.getAttribute ('transform')
    t0 = re.match ('\s*translate\s*\(\s*(-?\d+(?:\.\d+)?(?:[Ee][+-]?\d+)?)\s*,\s*(-?\d+(?:\.\d+)?(?:[Ee][+-]?\d+)?)\s*\)', t0)
    t0 = (float (t0.group (1)), float (t0.group (2))) if t0 else (0, 0)
    anchor = circles['%s_1' % name]
    x1 = float (anchor.getAttribute ('cx'))
    y1 = float (anchor.getAttribute ('cy'))
    t1 = anchor.getAttribute ('transform')
    t1 = re.match ('\s*translate\s*\(\s*(-?\d+(?:\.\d+)?(?:[Ee][+-]?\d+)?)\s*,\s*(-?\d+(?:\.\d+)?(?:[Ee][+-]?\d+)?)\s*\)', t1)
    t1 = (float (t1.group (1)), float (t1.group (2))) if t1 else (0, 0)
    x1 += t1[0] - t0[0]
    y1 += t1[1] - t0[1]

    anchors['%s_1' % name] = x1 - x0, y1 - y0

    paths = group.getElementsByTagName ('path')
    paths = [p.getAttribute ('id') for p in paths if p.hasAttribute ('id')]
    paths.sort ()

    glyph = []

    for path in paths:
      curve = []

      i = 0

      while '%s_%d' % (path, i) in circles:
        circle = circles['%s_%d' % (path, i)]
        scale = 1

        if circle.hasAttribute ('transform'):
          transform = circle.getAttribute ('transform')
          match = re.match ('\s*scale\s*\(\s*(-?\d+(?:\.\d+)?(?:[Ee][+-]?\d+)?)\s*,\s*-?\d+(?:\.\d+)?(?:[Ee][+-]?\d+)?\s*\)', transform) or re.match ('\s*matrix\s*\(\s*(-?\d+(?:\.\d+)?(?:[Ee][+-]?\d+)?)(?:\s*,\s*-?\d+(?:\.\d+)?(?:[Ee][+-]?\d+)?){5}\s*\)', transform)
          scale = float (match.group (1))

        r = float (circle.getAttribute ('r')) * scale

        curve.append ((float (circle.getAttribute ('cx')) - x0,
                       float (circle.getAttribute ('cy')) - y0, r))

        i += 1

      param = data[path] if path in data else (1, 0, 1, 1)

      glyph.append ((param[0], param[1], (param[2], param[3], curve)))

    font[name] = glyph, (x1 - x0, y1 - y0)

  print output (font)

  if len (argv) == 3:
    with open (argv[1], 'w') as f:
      sizes = dict ((k, float (v.getAttribute ('r'))) for k, v in circles.items () if not special (k, groupnames))

      f.write (radii (sizes, anchors))
      f.write ('\n')

    with open (argv[2], 'w') as f:
      f.write (timeline (pathnames))
      f.write ('\n')



if __name__ == '__main__':
  main (sys.argv)
