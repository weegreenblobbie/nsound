"""

Substitues svn version information into target.

$Id: SvnVersion.py 763 2013-09-18 00:43:44Z weegreenblobbie $
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


import os
import shlex
import subprocess

import SCons.Action
import SCons.Builder
import SCons.Util
import SCons.Script

SCons.Script.EnsureSConsVersion(0,96,92)

SvnVersionAction = SCons.Action.Action('$SVNVERSIONCOM', '$SVNVERSIONCOMSTR')


def svnversion_builder(*args):

	env, _, path = args

	dir_ = os.path.abspath(str(path[0]))

	if not os.path.isdir(dir_):
		raise ValueError(
			"target not found or not a directory path: %s" % repr(dir_))

	cmd = ['svnversion', dir_]

	p = subprocess.Popen(cmd, stdout = subprocess.PIPE)
	output = p.communicate()[0].strip()

	if len(output) == 0:
		output = "error"

	# Take the second half of any version reported as XXXX:YYYY.

	if ':' in output:
		output = output.split(":")[-1]

	return output


def generate(env):
	"""
	Add builders and construction variables for the SvnVersion builder.
	"""
	env.Append(BUILDERS = dict(svnversion = env.Builder(
			action = svnversion_builder)))

	try:
		bld = env['BUILDERS']['SvnVersion']
	except KeyError:
		bld = svnversion_builder
		env['BUILDERS']['SvnVersion'] = bld

	env['SVNVERSIONCOM'] = 'svnversion $TARGET'


def exists(env):
	return env.Detect('svnversion')
