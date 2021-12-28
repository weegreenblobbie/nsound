"""
$Id: nsound_config_windows.py 896 2015-06-07 03:46:27Z weegreenblobbie $

Nsound is a C++ library and Python module for audio synthesis featuring
dynamic digital filters. Nsound lets you easily shape waveforms and write
to disk or plot them. Nsound aims to be as powerful as Csound but easy to
use.

Copyright (c) 2009-Present Nick Hilton
"""

# Python modules
import os
import sys

# Scons
from SCons.SConf import CheckContext
from SCons.Script import Dir

# Custom python scripts
from nsound_config import NsoundConfig

CONFIG_DIR = "msvs"


class NsoundConfigWindows(NsoundConfig):

    def _customize_environment(self):

        self.env['NS_ON_WINDOWS'] = True
        self.env['NSOUND_PLATFORM_OS'] = "NSOUND_PLATFORM_OS_WINDOWS"
        self.env['NS_BUILD_STATIC'] = True

        import platform
        cpu = platform.machine().upper()

        if cpu in ['X86_64', 'AMD64']:
            cpu = 'x64'

        # Defaults
        CPPDEFINES = ["_CRT_SECURE_NO_WARNINGS"]
        CPPPATH = [Dir('external/include')]
        CXXFLAGS = "/nologo /O2 /W3 /EHsc /MD /Gd /TP /Zm256 /std:c++14".split()
        LIBPATH = [Dir('external/win32/lib/%s' % cpu)]
        LIBS = []
        LINKFLAGS = ["/nologo"]

        self.env.AppendUnique(
            CPPDEFINES = CPPDEFINES,
            CPPPATH = CPPPATH,
            CXXFLAGS = CXXFLAGS,
            LIBPATH = LIBPATH,
            LIBS = LIBS,
            LINKFLAGS = LINKFLAGS)

    def on_windows(self):
        return True
