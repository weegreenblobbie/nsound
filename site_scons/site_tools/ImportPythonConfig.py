"""

A SCons tool that tries to run python-config to collect the build environment
variables.

Sets env['HAVE_PYTHON_CONFIG'] to True on success, False otherwise.

$Id: ImportPythonConfig.py 806 2013-12-26 21:50:22Z weegreenblobbie $
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

import distutils
import distutils.sysconfig
import os
import os.path
import sys
import warnings

# Local tools
import _nsound_utils as nu

#------------------------------------------------------------------------------
# Globals & constants

on_windows = sys.platform == 'win32'
on_mac     = sys.platform == 'darwin'


def generate(env):
    """
    On posix systems, try to run the pythonA.B-config tool to import the build
    flags necessary to build and link Python extensions.
    """

    env['HAVE_PYTHON_CONFIG'] = False

    # Print all the config vars
#    for x in distutils.sysconfig.get_config_vars():
#
#        k = str(x)
#        v = str(distutils.sysconfig.get_config_var(x))
#
#        nu.dbg_print(env, __file__, "%-16s : %s" % (k, v))

    #--------------------------------------------------------------------------
    # Ask Python for specifics

    include_dir = distutils.sysconfig.get_config_var('INCLUDEPY')
    version     = distutils.sysconfig.get_python_version()
    libname     = distutils.sysconfig.get_config_var('LDLIBRARY')
    libdir      = distutils.sysconfig.get_config_var('LIBDIR')
    libversion  = distutils.sysconfig.get_config_var('VERSION')

    #--------------------------------------------------------------------------
    # Do some extra work on Windows boxes

    if libname is None and on_windows:
        libname = "python%s.lib" % libversion

    elif on_mac:
	libname = "python%s.dylib" % libversion

    if libname is None:
        warnings.warn(
            "\n\nCould not find the system Python library\n\n")

    nu.dbg_print(env, __file__, "python_exe:         %s" % sys.executable)
    nu.dbg_print(env, __file__, "python_include_dir: %s" % include_dir)
    nu.dbg_print(env, __file__, "version:            %s" % version)
    nu.dbg_print(env, __file__, "python_libname:     %s" % libname)
    nu.dbg_print(env, __file__, "python_lib_dir:     %s" % libdir)

    #--------------------------------------------------------------------------
    # Search for the directory containing python.so

    # Collect search paths

    lib_keys = [x for x in distutils.sysconfig.get_config_vars()
        if x.startswith("LIB")]

    lib_search_paths = []

    for key in lib_keys:

        path = distutils.sysconfig.get_config_var(key)

        if os.path.isdir(path):
            path = os.path.realpath(path)
            lib_search_paths.append(path)

    # If on windows, add PYTHON\libs
    if on_windows:
        prefix = os.path.realpath(os.path.dirname(sys.executable))
        path = os.path.join(prefix, "libs")
        if os.path.isdir(path):
            lib_search_paths.append(path)

    # Search for the library in each path
    lib_dir = None

    for path in lib_search_paths:

	name = libname

        if on_mac:
            name = "lib" + libname

        lib = os.path.join(path, name)

        if os.path.isfile(lib):
            lib_dir = os.path.dirname(lib)

    if lib_dir is None:

        path_str = ""
        for p in lib_search_paths:
            path_str += "\t%s\n" % p

        warnings.warn(
            "\n\nCould not find the Python library %s while searching these "
            "paths:\n\n    %s\n\nDo you have the python-dev package installed?\n" % (
                repr(libname), path_str))
        return

    nu.dbg_print(env, __file__, "python_lib_dir:     %s" % lib_dir)

    # Check that Python.h can be found
    python_h = os.path.join(include_dir, "Python.h")

    if not os.path.isfile(python_h):
        message = "Can't find Python.h: %s\n" % python_h
        message += "\nDo yo uhave the python-dev package installed?\n"
        warnings.warn(message)
        return

    #--------------------------------------------------------------------------
    # Construction vars

    # LIBS, stip off the library prefix and suffix

    lib = str(libname)

    prefix = env.subst('$SHLIBPREFIX')
    suffix = env.subst('$SHLIBSUFFIX')

    if lib.startswith(prefix):
        lib = lib[len(prefix):]

    if lib.endswith(suffix):
        lib = lib[:-len(suffix)]

    LIBS = [lib]

    # CPPPATH

    CPPPATH = [include_dir]

    # LIBPATH

    LIBPATH = [lib_dir]

    python_config = {
        'CPPPATH' : CPPPATH,
        'LIBPATH' : LIBPATH,
        'LIBS'    : LIBS,
        'VERSION' : version,
        }

    env['PYTHON_CONFIG'] = python_config

    s = ""
    for k, v in python_config.iteritems():
        s += "\t%-10s: %s\n" %(k, repr(v))

    nu.dbg_print(env, __file__, "\nPYTHON_CONFIG = \n%s" % s)

    env['HAVE_PYTHON_CONFIG'] = True


def exists(env):
    return True
