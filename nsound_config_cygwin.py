##############################################################################
#
# $Id: nsound_config_cygwin.py 867 2014-08-23 22:42:55Z weegreenblobbie $
#
# NsoundConfig.py
#
# A baseclass for configuring Nsound for different computing platforms.
#
##############################################################################

import os

# Scons
from SCons.Script import Dir

# Nsound imports
from nsound_config import NsoundConfig

class NsoundConfigCygwin(NsoundConfig):

    def _customize_environment(self):

        self.env['NS_ON_CYGWIN'] = True
        self.env['NSOUND_PLATFORM_OS'] = "NSOUND_PLATFORM_OS_CYGWIN"
        self.env['NS_BUILD_STATIC'] = True

        CXXFLAGS = []

        if self.env['NS_DEBUG_BUILD']:
            CXXFLAGS.append("-g")

        else:
            CXXFLAGS.extend(
                ["-fno-strict-aliasing", "-fwrapv", "-O2", ])

        import platform
        cpu = platform.machine().upper()

        if cpu in ['X86_64', 'AMD64']:
            cpu = 'amd64'

        CPPDEFINES = []
        CPPPATH = [Dir('external/include')]
        LIBPATH = [Dir('external/cygwin/lib/%s' % cpu)]
        LIBS = []

        self.env.AppendUnique(
            CPPDEFINES = CPPDEFINES,
            CPPPATH = CPPPATH,
            LIBPATH = LIBPATH,
            LIBS = LIBS)

    def add_custom_compiler_flags(self):

        if not self.env['NS_COMPILER']: return

        CXXFLAGS = []

        d = self.env['NS_COMPILER']

        if 'gcc' in d['compiler']:

            longver = d['longver']

            if longver >= 40600 and longver < 40700:
                CXXFLAGS = ["-std=c++0x"]

            elif longver >= 40700:
                CXXFLAGS = ["-std=c++11"]

        self.env.AppendUnique(CXXFLAGS = CXXFLAGS)

    def on_cygwin(self):
        return True
