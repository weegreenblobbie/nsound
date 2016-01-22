"""

$Id: AcGenerateFile.py 784 2013-12-08 00:08:24Z weegreenblobbie $

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

import glob
import os
import sys
import tempfile
import re

from SCons.Script import GetOption
from SCons import Warnings

VERBOSE = GetOption("verbose")

#------------------------------------------------------------------------------
# Globals & constants

ON_WINDOWS = sys.platform == 'win32'
ON_MAC     = sys.platform == 'darwin'
ON_LINUX   = 'linux' in sys.platform

RE_BOOST_LIB_VERSION = re.compile('#define\s+BOOST_LIB_VERSION\s+([^\s]+)')

KEY = 'BOOST'


class BoostLibNotDetected(Warnings.WarningOnByDefault):
    pass


#-----------------------------------------------------------------------------
# Determine 32 or 64 bits

import platform

bits, name = platform.architecture()

if bits != '64bit':
    raise RuntimeError("64-bit is required, detected %s" % bits)


if ON_WINDOWS:

    SEARCH_INCLUDE_DIRS = [
        'C:/local',
        'C:/',
    ]

    SEARCH_INCLUDE_DIRS = [x.replace('/', '\\') for x in SEARCH_INCLUDE_DIRS]


elif ON_LINUX:

    SEARCH_INCLUDE_DIRS = [
        "/usr/local/include",
        "/usr/include",
    ]

    SEARCH_LIB_DIRS = [
        "/usr/local/lib",
        "/usr/lib",
    ]


def win32_search_for_include_dir(env):
    '''
    Search for a directory called 'boost_{TAG}', in each prefix, if not found
    search for a directory called 'boost' in each prefix.
    '''

    search_include_dirs = env.get('BOOST_SEARCH_INCLUDE_DIRS', SEARCH_INCLUDE_DIRS)

    include_dirs = []

    for prefix in search_include_dirs:

        list_ = glob.glob(os.path.join(prefix, 'boost_*'))

        for d in list_:

            inc_dir = os.path.join(d, 'boost')

            if os.path.isdir(inc_dir):

                include_dirs.append(inc_dir)

    for prefix in search_include_dirs:

        list_ = glob.glob(os.path.join(prefix, 'boost'))

        if len(list_):
            include_dirs.extend(list_)

    return include_dirs


def linux_search_for_include_dir(env):
    '''
    Search for a directory called 'boost_{TAG}', in each prefix, if not found
    search for a directory called 'boost' in each prefix.
    '''

    search_include_dirs = env.get('BOOST_SEARCH_INCLUDE_DIRS', SEARCH_INCLUDE_DIRS)

    include_dirs = []

    for prefix in search_include_dirs:

        pattern = os.path.join(prefix, 'boost_*')

        if VERBOSE:
            print("    Looking for include dirs: %s" % pattern)

        list_ = glob.glob(pattern)

        if len(list_):

            if VERBOSE:
                for d in list_:
                    print("    %s" % d)

            include_dirs.extend(list_)

    for prefix in search_include_dirs:

        pattern = os.path.join(prefix, 'boost')

        if VERBOSE:
            print("    Looking for include dirs: %s" % pattern)

        list_ = glob.glob(pattern)

        if len(list_):

            if VERBOSE:
                for d in list_:
                    print("    %s" % d)

            include_dirs.extend(list_)

    if VERBOSE:

        print("    len(include_dirs) = %d" % len(include_dirs))

        for d in include_dirs:
            print("    %s" % d)

    return include_dirs


def search_for_include_dir(env):

    if ON_WINDOWS:
        include_dirs = win32_search_for_include_dir(env)

    else:
        include_dirs = linux_search_for_include_dir(env)

    scan_for_version_hpp(env, include_dirs)


def get_boost_tag(version_hpp):
    '''
    Reads the contents of version.hpp and scans for the version number.
    '''

    with open(version_hpp, 'r') as fd:
        text = fd.read()

    m = RE_BOOST_LIB_VERSION.search(text)

    if m is None:
        raise RuntimeError("Failed to find BOOST_LIB_VERSION in file: %s" % (
            version_hpp))

    tag = m.groups()[0].replace('"', '')

    return tag


def scan_for_version_hpp(env, include_dirs):
    '''
    Search for the file config.hpp in each directory.
    '''

    if VERBOSE and len(include_dirs) == 0:
        sys.stderr.write("    No include dirs!\n")

    file_list = []

    for d in include_dirs:

        fn = os.path.join(d, "version.hpp")

        if VERBOSE:
            sys.stdout.write("    Searching for version.hpp in %s" % d)

        if os.path.isfile(fn):
            file_list.append(fn)

            if VERBOSE:
                print(" ok")

        elif VERBOSE:
            print(" NOT FOUND")

    versions = []

    for f in file_list:

        tag = get_boost_tag(f)

        ver = float(tag.replace('_', '.'))

        versions.append((ver, tag, f))

    versions = sorted(versions, key = lambda x: x[0], reverse = True)

    if VERBOSE:
        for ver, _, ver_hpp in versions:
            print("    Found: %.2f in %s" % (ver, ver_hpp))

    if len(versions) == 0:
        return

    # Pick the largest one and return.

    ver, tag, version_hpp = versions[0]

    include_dir = os.path.dirname(os.path.dirname(version_hpp))

    d = dict(
        version = ver,
        tag = tag,
        include_dir = include_dir,
    )

    env['HAVE_BOOST'] = True
    env[KEY].update(d)


def win32_search_for_lib_dir(env):
    '''
    Assumes Visual Studio!

    On Windows, the lib dir is always off of the include dir:

        {INCLUDE_DIR}\lib64-msvc-{MSVC_VERSION}

    Search inside the libdir for the file:

        boost_system-{toolset}-*-{TAG}.dll

    '''

    try:
        compiler_info = env['DETECTED_COMPILER']
    except KeyError:
        raise RuntimeError("Compiler hasn't been detected yet!")

    toolset = compiler_info['toolset']
    msvc_version = compiler_info['msvc_version']

    search_lib_dirs = env.get(
        'BOOST_SEARCH_LIB_DIRS', [env[KEY]['include_dir']])

    prefix = 'lib64-msvc-' + msvc_version

    pattern = 'boost_system-{toolset}-*-{tag}.dll'.format(
        toolset = toolset,
        tag = env[KEY]['tag'])

    for d in search_lib_dirs:

        d = os.path.join(d, prefix)

        pattern = os.path.join(d, pattern)

        libs = glob.glob(pattern)

        if len(libs) == 0:
            d = None
            d['HAVE_BOOST'] = False

        env[KEY]['lib_dir'] = d

        break


def linux_search_for_lib_dir(env):
    '''
    Search for libboost_system.so.{VER}.*
    '''

    search_lib_dirs = env.get('BOOST_SEARCH_LIB_DIRS', SEARCH_LIB_DIRS)

    for d in search_lib_dirs:

        pattern = os.path.join(d, 'libboost_system.so.%.2f.*' % (
            env[KEY]['version']))

        libs = glob.glob(pattern)

        if len(libs) == 0:
            raise RuntimeError("Failed to find Boost library: %s" % pattern)

        env[KEY]['lib_dir'] = d
        break


def search_for_lib_dirs(env):
    '''
    Given that the include directory was found, search for the lib dir.
    '''

    if ON_WINDOWS:
        win32_search_for_lib_dir(env)

    else:
        linux_search_for_lib_dir(env)


def win32_add_boost_flags(env):

#~    print "BEFORE = ", repr(env['CPPDEFINES'])

    env.AppendUnique(
        CPPDEFINES = ['BOOST_ALL_DYN_LINK', '_WIN32_WINNT=0x0501'],
        CXXFLAGS = ["-I%s" % env[KEY]['include_dir'], "/MD"],
        LIBPATH = [env[KEY]['lib_dir']],
    )

#~    print "AFTER = ", repr(env['CPPDEFINES'])

    #--------------------------------------------------------------------------
    # Fix a scons BUG: CPPDEFINES can turn into a list of tupples of strings

    defs = []

    for x in env['CPPDEFINES']:
        if isinstance(x, tuple):
            defs.append(x[0])
        else:
            defs.append(x)

    env['CPPDEFINES'] = defs


def linux_add_boost_flags(env):

    env.AppendUnique(
        CPPDEFINES = ['BOOST_ALL_DYN_LINK'],
        CXXFLAGS = ['-I%s' % env[KEY]['include_dir']],
        LIBPATH = [env[KEY]['lib_dir']],
        LINKFLAGS = ["-Wl,--rpath=%s" % env[KEY]['lib_dir']],
    )


def add_boost_flags(env, *args, **kwargs):

    '''
    Appends the following environment variables:

        CPPDEFINES
        CXXFLAGS
        LIBPATH
        LIBS
        LINKFLAGS
    '''

    if ON_WINDOWS:
        win32_add_boost_flags(env)

    else:
        linux_add_boost_flags(env)


def win32_link_boost_lib(env, args, **kwargs):
    '''
    Do nothing, auto linking is enabled.
    '''

    env['boost_dll_libs'] = []

    linking_dll = kwargs.get('linking_dll', False)

    if linking_dll:

        compiler_info = env['DETECTED_COMPILER']

        toolset = compiler_info['toolset']

        libs = []

        for l in args:

            libname = '{lib}-{toolset}-mt-{tag}.lib'.format(
                lib = l,
                toolset = toolset,
                tag = env[KEY]['tag'])

            libs.append(libname)

        env.AppendUnique(LIBS = libs)


def linux_link_boost_lib(env, libs):

    env.Append(
        LIBS = libs,
    )


def link_boost_lib(env, _, libs, **kwargs):

    if ON_WINDOWS:
        win32_link_boost_lib(env, libs, **kwargs)

    else:
        linux_link_boost_lib(env, libs)


def require_boost_version_ge(env, *args):

    _, version = args

    version = float(version[0])

    if not env.have_boost():
        raise RuntimeError("Boost >= %.2f wasn't detected!" % version)

    if env[KEY]['version'] < version:
        raise RuntimeError(
            "Detected boost version %.2f is less than required %.2f" % (
                env[KEY]['version'],
                version))


def have_boost(env, *args):
    return env['HAVE_BOOST']


def detect_boost(*args):

    env, _, _ = args

    if VERBOSE:
        print env['DETECTBOOSTCOM']

    env['HAVE_BOOST'] = False
    env[KEY] = {}

    search_for_include_dir(env)

    if env['HAVE_BOOST']:
        search_for_lib_dirs(env)

        if VERBOSE:
            for k, v in env[KEY].iteritems():
                print '    env["{KEY}"]["{k}"] = {v}'.format(
                    KEY = KEY, k = k, v = v)


def generate(env):
    """
    Add a builder and construction variable for detected boost.
    """

    env['BUILDERS']['DetectBoost'] = detect_boost
    env['BUILDERS']['add_boost_flags'] = add_boost_flags
    env['BUILDERS']['have_boost'] = have_boost
    env['BUILDERS']['link_boost_lib'] = link_boost_lib
    env['BUILDERS']['require_boost_version_ge'] = require_boost_version_ge

    comstr = "Detecting Boost"

    env['DETECTBOOSTCOM'] = comstr
    env['DETECTBOOSTCOMSTR'] = comstr


def exists(env):
    return env.Detect(env['CXX'])
