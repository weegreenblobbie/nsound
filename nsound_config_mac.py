##############################################################################
#
# $Id: nsound_config_mac.py 854 2014-06-30 00:29:02Z weegreenblobbie $
#
# NsoundConfig_Mac.py
#
# A baseclass for configuring Nsound for different computing platforms.
#
##############################################################################

# Python imports
import os
import sys

# Nsound imports
from nsound_config import NsoundConfig

class NsoundConfigMac(NsoundConfig):

    def __init__(self):
        NsoundConfig.__init__(self)

    def _customize_environment(self):

        self.env['NS_ON_MAC'] = True
        self.env['NSUOND_PLATFORM_OS'] = "NSOUND_PLATFORM_OS_MAC"

        if self.env['NS_DEBUG_BUILD']:
            self.env.AppenduUnique(CXXFLAGS = ["-g"])

        else:
            self.env.AppendUnique(CXXFLAGS = ["-O2"])


	# if using G++, enbale c++11
	self.env.AppendUnique(CXXFLAGS = ['-std=c++11'])

        #---------------------------------------------------------
        # rpath Hack, g++ on OSX doesn't use -Wl,-rpath=somepath
        #
        # The trick here is to specify an abspath when linking libNsound.dylib

        self.env.AppendUnique(
            SHLINKFLAGS =
                [
                    '-install_name',
                    '%s/${TARGET.filebase}${TARGET.suffix}' % self.env['NS_LIBDIR']
                ])

        #----------------------------------------------------------
        # look for portaudio.h

        PATH_LIST = ["/usr", "/usr/local", "/opt", "/opt/local"]

        for path in PATH_LIST:

            portaudio_h = os.path.join(path, "include", "portaudio.h")

            if os.path.isfile(portaudio_h):
                cpppath = os.path.join(path, "include")
                libpath = os.path.join(path, "lib")

                self.env.AppendUnique(
                    CPPPATH = [cpppath],
                    LIBPATH = [libpath])

    def add_to_rpath(self, path):
        """
        -Wl,-rpath not available on Mac.
        """
        #self.env.Append(
        #    LINKFLAGS = ['-Xlinker', '-rpath', '-Xlinker', path])

    def on_mac(self):
        return True

