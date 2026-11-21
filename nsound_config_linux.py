"""

$Id: nsound_config_linux.py 875 2014-09-27 22:25:13Z weegreenblobbie $

"""

import os

# Nsound imports
from nsound_config import NsoundConfig

class NsoundConfigLinux(NsoundConfig):

    def _customize_environment(self):

        self.env['NS_ON_LINUX'] = True
        self.env['NSOUND_PLATFORM_OS'] = "NSOUND_PLATFORM_OS_LINUX"

        if self.env['NS_DEBUG_BUILD']:
            self.env.AppendUnique(CXXFLAGS = ["-g"])

        else:
            self.env.AppendUnique(
                CXXFLAGS = ["-fno-strict-aliasing", "-fwrapv", "-O2", '-Wall'])

    def add_custom_compiler_flags(self):

        if not self.env['NS_COMPILER']: return

        CXXFLAGS = []

        d = self.env['NS_COMPILER']

        if 'gcc' in d['compiler'] or 'clang' in d['compiler']:
            CXXFLAGS = ["-std=c++20"]

        self.env.AppendUnique(CXXFLAGS = CXXFLAGS)

    def on_linux(self):
        return True
