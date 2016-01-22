"""

Tool for running python setup.py install --user

Reference: http://scons.org/wiki/ToolsForFools

$Id: PythonSetup.py 763 2013-09-18 00:43:44Z weegreenblobbie $
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


import datetime
import shlex
import sys
import subprocess

from SCons.Action import Action
from SCons.Script import Delete
from SCons.Script import EnsureSConsVersion
from SCons.Script import File
from SCons.Script import Touch

EnsureSConsVersion(0,96,92)

def emitter(target, source, env):

	env.Depends(target, source)

	return target, source


def generate(env):
	"""
	Add builders and construction variables for the SvnVersion builder.
	"""

	python = File(sys.executable).get_abspath()
	python = python.replace("\\", "\\\\") # windows fix

	env['PYTHON'] = python
	cmd = "$PYTHON $SOURCE install --user"
	env['PYTHONSETUPCOM'] = cmd

	action = Action('$PYTHONSETUPCOM', '$PYTHONSETUPCOMSTR')

	bld = env.Builder(
		action = [Delete("build"), action, Touch("$TARGETS")],
		emitter = emitter,
		target_factor = env.fs.Entry,
		src_suffic = ".py",
		)

	env['BUILDERS']['PythonSetup'] = bld


def exists(env):
	return True

