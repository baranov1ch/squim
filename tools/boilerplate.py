#!/usr/bin/env python

# Copy of chromium tools/boilerplate.py (with minor changes).
# LICENSE can be found here:
# https://code.google.com/p/chromium/codesearch#chromium/src/LICENSE

# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Create files with copyright boilerplate and header include guards.

Usage: tools/boilerplate.py path/to/file.{h,cc}
"""

from datetime import date
import os
import os.path
import sys

LINES = [
  'Copyright %d Alexey Baranov <me@kotiki.cc>. All rights reserved.' %
      date.today().year,
  '',
  'Licensed under the Apache License, Version 2.0 (the "License");',
  'you may not use this file except in compliance with the License.',
  'You may obtain a copy of the License at',
  '',
  '  http://www.apache.org/licenses/LICENSE-2.0',
  '',
  'Unless required by applicable law or agreed to in writing, software',
  'distributed under the License is distributed on an "AS IS" BASIS,',
  'WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.',
  'See the License for the specific language governing permissions and',
  'limitations under the License.',
]

EXTENSIONS_TO_COMMENTS = {
    'h': ' *',
    'cc': ' *',
    'mm': ' *',
    'js': ' *',
    'py': '#',
    'sh': '#',
}

def _BuildCopyrightLine(comment, line):
  if not line:
    return comment
  return comment + ' ' + line

def _GetHeader(filename):
  _, ext = os.path.splitext(filename)
  ext = ext[1:]
  comment = EXTENSIONS_TO_COMMENTS[ext]
  copyright_lines = [_BuildCopyrightLine(comment, line) for line in LINES]
  if comment == ' *':
    copyright_lines = ['/*'] + copyright_lines + [' */']
  return '\n'.join(copyright_lines)


def _CppHeader(filename):
  guard = filename.replace('/', '_').replace('.', '_').upper() + '_'
  return '\n'.join([
    '',
    '#ifndef ' + guard,
    '#define ' + guard,
    '',
    '#endif  // ' + guard,
    ''
  ])


def _CppImplementation(filename):
  base, _ = os.path.splitext(filename)
  include = '#include "' + base + '.h"'
  return '\n'.join(['', include])


def _CreateFile(filename):
  contents = _GetHeader(filename) + '\n'

  if filename.endswith('.h'):
    contents += _CppHeader(filename)
  elif filename.endswith('.cc') or filename.endswith('.mm'):
    contents += _CppImplementation(filename)

  fd = open(filename, 'w')
  fd.write(contents)
  fd.close()


def Main():
  files = sys.argv[1:]
  if len(files) < 1:
    print >> sys.stderr, 'Usage: boilerplate.py path/to/file.h path/to/file.cc'
    return 1

  # Perform checks first so that the entire operation is atomic.
  for f in files:
    _, ext = os.path.splitext(f)
    if not ext[1:] in EXTENSIONS_TO_COMMENTS:
      print >> sys.stderr, 'Unknown file type for %s' % f
      return 2

    if os.path.exists(f):
      print >> sys.stderr, 'A file at path %s already exists' % f
      return 2

  for f in files:
    _CreateFile(f)


if __name__ == '__main__':
  sys.exit(Main())
