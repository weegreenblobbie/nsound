"""

Taken from the SubstInFileBuilder on the SCons Wiki.  Slightly modified to
use @ around keys so it behaves like GNU autotools.

$Id: AcGenerateFile.py 784 2013-12-08 00:08:24Z weegreenblobbie $

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


import re

from SCons.Script import Action
from SCons.Script import Builder
import SCons.Util

def do_subst_in_file(targetfile, sourcefile, sub_dict):
	"""
	Replace all instances of the keys of dict with their values.
	For example, if dict is {'VERSION': '1.2345', 'BASE': 'MyProg'},
	then all instances of @VERSION@ in the file will be replaced with
	1.2345 etc.
	"""
	try:
		f = open(sourcefile, 'rb')
		contents = f.read()
		f.close()

	except:
		raise SCons.Errors.UserError, "Can't read source file %s" % sourcefile

	for k, v in sub_dict.items():
		contents = re.sub("@" + k + "@", r'%s' % v, contents)

	try:
		f = open(targetfile, 'wb')
		f.write(contents)
		f.close()

	except:
		raise SCons.Errors.UserError, "Can't write target file %s" % targetfile

	return 0 # success


def subst_in_file(target, source, env):

	if not env.has_key('AC_GEN_DICT'):
		raise SCons.Errors.UserError, "AcGenerateFile requires AC_GEN_DICT to be set."

	d = dict(env['AC_GEN_DICT']) # copy it

	for (k,v) in d.items():

		if callable(v):
			d[k] = env.subst(v())

		elif SCons.Util.is_String(v):
			d[k]=env.subst(v)

		else:
			raise SCons.Errors.UserError, "AcGenerateFile: key %s: %s must be a string or callable"%(k, repr(v))

	for (t,s) in zip(target, source):
		return do_subst_in_file(str(t), str(s), d)


def subst_in_file_string(target, source, env):
	"""
	This is what gets printed on the console.
	"""
	return '\n'.join(['Generating %s from %s' % (str(t), str(s))
		for (t,s) in zip(target, source)])


def emitter(target, source, env):
	"""
	Add dependency from substituted AC_GEN_DICT to target.
	Returns original target, source tuple unchanged.
	"""

	d = env['AC_GEN_DICT'].copy() # copy it

	for (k,v) in d.items():

		if callable(v):
			d[k] = env.subst(v())

		elif SCons.Util.is_String(v):
			d[k]=env.subst(v)

	env.Depends(target, SCons.Node.Python.Value(d))

	return target, source


def generate(env):

	subst_action = Action(subst_in_file, subst_in_file_string)
	env['BUILDERS']['AcGenerateFile'] = Builder(
		action = subst_action,
		emitter = emitter)


def exists(env):
	return True
