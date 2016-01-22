"""

A SCons tool that runs swig on the command line and returns the wrapper source
file generated.

$Id: SwigGen.py 846 2014-05-27 21:56:19Z weegreenblobbie $

"""

"""
Nsound is a C++ library and Python module for audio synthesis featuring
dynamic digital filters. Nsound lets you easily shape waveforms and write
to disk or plot them. Nsound aims to be as powerful as Csound but easy to
use.

Copyright (c) 2004 to Present Nick Hilton

weegreenblobbie_at_yahoo_com

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Library General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
"""

import os.path
import shlex
import sys

POSIX = 'win32' not in sys.platform

import SCons.Action
from SCons.Script import EnsureSConsVersion

SCons.Script.EnsureSConsVersion(0,96,92)

SwigGenAction = SCons.Action.Action('$SWIGGENCOM', '$SWIGGENCOMSTR')

def emitter(target, source, env):
	"""
	Add dependency from target to source
	"""

	# Transform source file into extra target
	#
	# source.i --> source.py
	#

	source_i = str(source[0])
	base, _ = os.path.splitext(source_i)

	target_py = base + ".py"

	# Prefix target_PY with -outdir if specified
	if '-outdir' in env['SWIGFLAGS']:

		target_py = os.path.basename(target_py)

		pos = env['SWIGFLAGS'].index('-outdir')

		outdir = str(env['SWIGFLAGS'][pos + 1])

		# Process outdir, removing possible quotes
		outdir = shlex.split(outdir, POSIX)[0]

		target_py = "%s" % os.path.join(outdir, target_py)

	target = list(target)
	target.append(target_py)

	env.Depends(target, source)

	return target, source


def generate(env):
	"""
	Add builders and construction variables for the SwigGen builder.
	"""

	if 'SWIGCOM' not in env:
		raise SystemError("SCons build environment could not detect tool: swig")

	bld = env.Builder(
		action = SwigGenAction,
		emitter = emitter,
		target_factory = env.fs.File)

	env['BUILDERS']['SwigGen'] = bld

	env['SWIGGENCOM'] = env['SWIGCOM']


def exists(env):
	"""
	Check if the swig executable can be found.
	"""

	return env.Detect('swig')
