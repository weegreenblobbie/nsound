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

    def __init__(self):
        NsoundConfig.__init__(self)

    def _customize_environment(self):

        self.env['NS_ON_WINDOWS'] = True
        self.env['NSOUND_PLATFORM_OS'] = "NSOUND_PLATFORM_OS_WINDOWS"
        self.env['NS_BUILD_STATIC'] = True

        import platform
        cpu = platform.machine().upper()

        if cpu in ['X86_64', 'AMD64']:
            cpu = 'x64'

        # Defaults
        CPPDEFINES = []
        CPPPATH = [Dir('external/include')]
        CXXFLAGS = "/nologo /O2 /W3 /EHsc /MD /Gd /TP /Zm256".split()
        ENV = os.environ
        LIBPATH = [Dir('external/win32/lib/%s' % cpu)]
        LIBS = []
        LINKFLAGS = ["/nologo"]

        self.env.AppendUnique(
            CPPDEFINES = CPPDEFINES,
            CPPPATH = CPPPATH,
            CXXFLAGS = CXXFLAGS,
            ENV = ENV,
            LIBPATH = LIBPATH,
            LIBS = LIBS,
            LINKFLAGS = LINKFLAGS)

        # Manifest handeling
        self._generate_vc_runtime_manifest()

    def _generate_vc_runtime_manifest(self):
        """
        Since the official Python binaries are compiled with Visual Studio
        2008, we need to embed a manifest to link against the same Vistual
        Studio runtime libraries in order to call Matplotlib plots and not
        crash.
        """

        sys.stdout.write("creating Microsoft VC Rutime Manifest file ... ")
        sys.stdout.flush()

        # Defaults
        name    = "Microsoft.VC90"
        version = "9.0.21022.8"
        key     = "1fc8b3b9a1e18e3b"

        try:
            import msvcrt

            name    = msvcrt.LIBRARIES_ASSEMBLY_NAME_PREFIX
            version = msvcrt.CRT_ASSEMBLY_VERSION
            key     = msvcrt.VC_ASSEMBLY_PUBLICKEYTOKEN

        except:
            pass

        manifest_xml = '''\
        <assembly xmlns="urn:schemas-microsoft-com:asm.v1" manifestVersion="1.0">
          <trustInfo xmlns="urn:schemas-microsoft-com:asm.v3">
            <security>
              <requestedPrivileges>
                <requestedExecutionLevel level="asInvoker" uiAccess="false"></requestedExecutionLevel>
              </requestedPrivileges>
            </security>
          </trustInfo>
          <dependency>
            <dependentAssembly>
              <assemblyIdentity type="win32" name="{name}.CRT" version="{version}" processorArchitecture="*" publicKeyToken="{key}"></assemblyIdentity>
            </dependentAssembly>
          </dependency>
        </assembly>'''.format(name = name, version = version, key = key)

        if not os.path.exists(CONFIG_DIR):
            os.makedirs(CONFIG_DIR)

        manifest_filename = os.path.join(CONFIG_DIR, "msvcrt.manifest")

        # Write out manifest
        fout = open(manifest_filename, "w")
        fout.write(manifest_xml)
        fout.close()

        sys.stdout.write(" done\n")

        self.env['LINKCOM'] = [
            self.env['LINKCOM'],
            'mt.exe -nologo -manifest {filename} '
            '-outputresource:$TARGET;1'.format(filename = manifest_filename)]

        self.env['SHLINKCOM'] = [
            self.env['SHLINKCOM'],
            'mt.exe -nologo -manifest {filename} '
            '-outputresource:$TARGET;2'.format(filename = manifest_filename)]

    def on_windows(self):
        return True
