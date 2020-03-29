"""
$Id: SConstruct 926 2015-08-23 17:36:20Z weegreenblobbie $

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
import platform
import os.path
import sys
import subprocess

#-----------------------------------------------------------------------------
# Setup Package Name and Version

PACKAGE_NAME = "Nsound"

VERSION_A = "0"
VERSION_B = "9"
VERSION_C = "5"

DEVELOPMENMT = True

PACKAGE_VERSION = "%s.%s.%s" % (VERSION_A, VERSION_B, VERSION_C)

if DEVELOPMENMT:
    PACKAGE_VERSION += ".dev1"

PACKAGE_RELEASE = PACKAGE_NAME + "-" + PACKAGE_VERSION

Export("PACKAGE_NAME")
Export("PACKAGE_VERSION")

#-----------------------------------------------------------------------------
# command line options

AddOption(
    "--compiler",
    dest = "compiler",
    default = None,
    nargs = 1,
    type = "string",
    help = "Sets the install prefix for programs and libraries")

AddOption(
    "--extra-warnings",
    dest = "extra_warnings",
    action = "store_true",
    default = False,
    help = "Adds extra warning flags to CXXFLAGS")

AddOption(
    "--disable-64",
    dest = "disable_64",
    action = "store_true",
    default = False,
    help = "Disables float64 type, uses float32 instead")

AddOption(
    "--disable-libao",
    dest = "disable_libao",
    action = "store_true",
    default = False,
    help = "Disables the libao AudioBackend")

AddOption(
    "--disable-libportaudio",
    dest = "disable_libportaudio",
    action = "store_true",
    default = False,
    help = "Disables the libportaudio AudioBackend")

AddOption(
    "--enable-cuda",
    dest = "enable_cuda",
    action = "store_true",
    default = False,
    help = "Enables Cuda accelerated functions and linking")

AddOption(
    "--disable-openmp",
    dest = "disable_openmp",
    action = "store_true",
    default = False,
    help = "Disables OpenMp accelerated functions and linking")

AddOption(
    "--disable-python",
    dest = "disable_python",
    action = "store_true",
    default = False,
    help = "Disables the use of matplotlib via Python, only applies to C++ lib")

AddOption(
    "--static",
    dest = "build_static",
    action = "store_true",
    default = False,
    help = "Builds a static C++ lib")

AddOption(
    "--prefix",
    dest = "prefix",
    default = None,
    nargs = 1,
    type = "string",
    help = "Sets the install prefix for programs and libraries")

AddOption(
    "--config-debug",
    dest = "config_debug",
    default = False,
    action = "store_true",
    help = "Prints lots of debug messages during scons configuration tests")

AddOption(
    "--V",
    "--verbose",
    dest = "verbose",
    action = "store_true",
    default = False,
    help = "Shows full compiler command output")

AddOption(
    "--D",
    "--DEBUG",
    dest = "debug_build",
    action = "store_true",
    default = False,
    help = "Compile in debug mode")

AddOption(
    "--with-boost",
    dest = "boost_prefix",
    default = None,
    nargs = 1,
    type = "string",
    help = "Adds the boost include path into the compiler flags")

AddOption(
    "--pytest",
    dest = "pytest",
    default = False,
    action = "store_true",
    help = "Rebuilds python module, then runs unit tests")

AddOption(
    "--unit-test",
    dest = "unit_test",
    default = False,
    action = "store_true",
    help = "Runs the c++ unit tests")


if GetOption("pytest"):

    commands = [
        "scons -c",
        "scons setup_builder.py",
        "python setup_builder.py install --user",
        "python -m unittest discover"]

    for cmd in commands:

        try:
            subprocess.check_output(cmd)

        except:
            raise RuntimeError("FAILURE!\ncmd = %s" % cmd)

    Exit(0)


if GetOption("unit_test"):

    env = Environment()

    target = "Main" + env['PROGSUFFIX']

    os.chdir('src/test')

    commands = [
        "scons -u " + target,
    ]

    if "win32" not in sys.platform:
        commands.append('./' + target)
    else:
        commands.append(target)

    for cmd in commands:
        subprocess.check_call(cmd, shell=True)

    Exit(0)


#------------------------------------------------------------------------------
# Do parallel builds by default

n_jobs = 1

try:
    import multiprocessing
    n_jobs = multiprocessing.cpu_count()
except:
    pass

SetOption('num_jobs', n_jobs)

print("Building with %d threads" % GetOption('num_jobs'))

#------------------------------------------------------------------------------
# Check if building the python module

build_py_module = False

if "setup_builder.py" in COMMAND_LINE_TARGETS:

    build_py_module = True

Export("build_py_module")

# Determin the platform to build on

encode_lib_path = 1
use_disttar = 1
use_distzip = 1

#------------------------------------------------------------------------------
# Platform and architecture

bits, name = platform.architecture()
arch = "x86"

if "64" in bits:
	arch = "AMD64"

if not GetOption("help"):

    print("""
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

Help Nsound, please submit bug reports if things aren't working for you.

https://github.com/weegreenblobbie/nsound

Contact Nick for help: weegreenblobbie2_gmail_com

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
""")

    print("Using Python%s" % platform.python_version())
    print("")
    print("Building on %s, %s" % (sys.platform, arch))

if sys.platform in ["linux", "linux2", "linux3"]:

    from nsound_config_linux import NsoundConfigLinux
    nsound_config = NsoundConfigLinux()

elif sys.platform == "cygwin":

    from nsound_config_cygwin import NsoundConfigCygwin
    nsound_config = NsoundConfigCygwin()

elif sys.platform == "darwin":

    from nsound_config_mac import NsoundConfigMac
    nsound_config = NsoundConfigMac()

elif sys.platform == "win32":

    from nsound_config_windows import NsoundConfigWindows
    nsound_config = NsoundConfigWindows()

else:
    sys.stderr.write("\n")
    sys.stderr.write("ERROR: unsupported platform %s\n" % repr(sys.platform))
    sys.stderr.write("Send email to the nsound develpers for help\n")
    sys.stderr.write("\n")
    Exit()

Export("nsound_config")

nsound_config.add_to_rpath(nsound_config.env['NS_LIBDIR'])

if not nsound_config.env.GetOption("help"):
    print("bindir = %s" % nsound_config.env['NS_BINDIR'])
    print("libdir = %s" % nsound_config.env['NS_LIBDIR'])
    print("")

nsound_h = "src/Nsound/Nsound.h"
doxyfile = "docs/reference/Doxyfile"
setup_builder_py = "setup_builder.py"

have_cuda = False
have_libao = False
have_libportaudio = False

if      not nsound_config.env.GetOption("clean") \
    and not nsound_config.env.GetOption("help"):

    # C++ compiler?

    if not nsound_config.CheckCXX():
        raise RuntimeError(
"""
Failed to compile test program, check config.log for
the compiler statements and errors.
""")

    nsound_config.detect_compile_version()
    nsound_config.add_custom_compiler_flags()

    if not nsound_config.check_compiler_works():
        try:
            sys.stderr.write("grep -i error config.log\n\n")
            os.system("grep -i error config.log")
            sys.stderr.write("\n")
        except:
            pass

        raise RuntimeError(
"""
Failed to compile test program, check config.log for
the compiler statements and errors.
""")

    nsound_config.check_for_cpp11()

    if not nsound_config.env['NS_HAVE_CPP11']:
        raise RuntimeError("Nsound now requires a C++11 compiler!")

    nsound_config.check_endian()
    nsound_config.check_for_pi_macro()

    #--------------------------------------------------------------------------
    # Python matplotlib C API

    nsound_config.check_header("Python.h", language = "C++")
    nsound_config.check_for_matplotlib()
    nsound_config.check_for_matplotlib_c_api()

    #--------------------------------------------------------------------------
    # Audio backends

    # portaudio
    nsound_config.check_header("portaudio.h", language = "C++")
    nsound_config.check_for_portaudio_h_v18()
    nsound_config.check_for_portaudio_h_v19()
    nsound_config.check_for_libportaudio()

    # libao
    nsound_config.check_header("ao/ao.h", language = "C++")
    nsound_config.check_for_libao()

    #--------------------------------------------------------------------------
    # OpenMP and Cuda

    nsound_config.check_for_cuda()
    nsound_config.check_for_open_mp()

    #--------------------------------------------------------------------------
    # Generate Nsound.h

    env = nsound_config.env
    d = {}

    d["PACKAGE_NAME"]      = PACKAGE_NAME
    d["PACKAGE_VERSION"]   = PACKAGE_VERSION
    d["PACKAGE_RELEASE"]   = PACKAGE_RELEASE
    d["PACKAGE_VERSION_A"] = VERSION_A
    d["PACKAGE_VERSION_B"] = VERSION_B
    d["PACKAGE_VERSION_C"] = VERSION_C

    if build_py_module:
        d["NSOUND_IN_PYTHON_MODULE"] = "#define NSOUND_IN_PYTHON_MODULE 1"
    else:
        d["NSOUND_IN_PYTHON_MODULE"] = "// no"

    if env['NS_HAVE_OPENMP']:
        d["NSOUND_OPENMP"] = "#define NSOUND_OPENMP 1"
        d["NSOUND_HAVE_OPENMP"] = "True"
    else:
        d["NSOUND_OPENMP"] = "#undef NSOUND_OPENMP // disabled"
        d["NSOUND_HAVE_OPENMP"] = "False"

    if env['NS_HAVE_CPP11']:
        d["NSOUND_CPP11"] = "#define NSOUND_CPP11 1"
        d["NSOUND_HAVE_CPP11"] = "True"
    else:
        d["NSOUND_CPP11"] = "#undef NSOUND_CPP11 // disabled"
        d["NSOUND_HAVE_CPP11"] = "False"

    if env['NS_HAVE_CUDA']:
        d["NSOUND_CUDA"] = "#define NSOUND_CUDA 1"
        d["NSOUND_HAVE_CUDA"] = "True"
    else:
        d["NSOUND_CUDA"] = "#undef NSOUND_CUDA // disabled"
        d["NSOUND_HAVE_CUDA"] = "False"

    if env['NS_HAVE_MATPLOTLIB_C_API']:
        d["NSOUND_C_PYLAB"] = "#define NSOUND_C_PYLAB 1"
    else:
        d["NSOUND_C_PYLAB"] = "#undef NSOUND_C_PYLAB  // disabled"

    d["NSOUND_PLATFORM_OS"] = env['NSOUND_PLATFORM_OS']
    d["NSOUND_BOOLEAN"] = "bool"

    if nsound_config.CheckTypeSize("char", expect = 1, language = "C++"):
        d["NSOUND_INT_8"]   = "signed char "
        d["NSOUND_U_INT_8"] = "unsigned char"
    else:
        raise RuntimeError("Need to specify compiler's 1 byte integer type.")

    if nsound_config.CheckTypeSize("short", expect = 2, language = "C++"):
        d["NSOUND_INT_16"]   = "signed short"
        d["NSOUND_U_INT_16"] = "unsigned short"
    else:
        raise RuntimeError("Need to specify compiler's 2 byte integer type.")

    if nsound_config.CheckTypeSize("int", expect = 4, language = "C++"):
        d["NSOUND_INT_32"]   = "signed int  "
        d["NSOUND_U_INT_32"] = "unsigned int"
    else:
        raise RuntimeError("Need to specify compiler's 4 byte integer type.")

    if nsound_config.CheckTypeSize("float", expect = 4, language = "C++"):
        d["NSOUND_FLOAT_32"] = "float"
    else:
        raise RuntimeError("Need to specify compiler's 4 byte float type.")

    if nsound_config.CheckTypeSize("long long", expect = 8, language = "C++"):

        d["NSOUND_INT_64"]   = "signed long long"
        d["NSOUND_U_INT_64"] = "unsigned long long"

        d["NSOUND_RAW_INT_64"]   = d["NSOUND_INT_64"]
        d["NSOUND_RAW_U_INT_64"] = d["NSOUND_U_INT_64"]

    else:
        raise RuntimeError("Need to specify compiler's 8 byte integer type.")

    if nsound_config.CheckTypeSize("double", expect = 8, language = "C++"):

        d["NSOUND_FLOAT_64"]     = "double"
        d["NSOUND_RAW_FLOAT_64"] = d["NSOUND_FLOAT_64"]

    else:
        raise RuntimeError("Need to specify compiler's 8 byte float type.")

    d["NSOUND_64_32_BIT"] = "#define NSOUND_64_BIT 1"

    if env['NS_DISABLE_64']:

        d["NSOUND_64_32_BIT"] = "#define NSOUND_32_BIT 1"

        d["NSOUND_INT_64"]   = d["NSOUND_INT_32"]
        d["NSOUND_U_INT_64"] = d["NSOUND_U_INT_32"]
        d["NSOUND_FLOAT_64"] = d["NSOUND_FLOAT_32"]

    if env['NS_ENDIAN_IS_LITTLE']:
        d["NSOUND_ENDIAN"] = "NSOUND_LITTLE_ENDIAN"

    else:
        d["NSOUND_ENDIAN"] = "NSOUND_BIG_ENDIAN"

    if env['NS_HAVE_M_PI']:
        d["NSOUND_M_PI"] = "// M_PI is defined in math.h"

    else:
        d["NSOUND_M_PI"] = "// M_PI is _NOT_ defined in math.h\n"
        d["NSOUND_M_PI"] += "#define M_PI 3.1415926535897932846"

    if env['NS_HAVE_LIBPORTAUDIO']:
        d["NSOUND_LIBPORTAUDIO"] = "#define NSOUND_LIBPORTAUDIO 1"
    else:
        d["NSOUND_LIBPORTAUDIO"] = "#undef NSOUND_LIBPORTAUDIO // disabled"

    if env['NS_HAVE_LIBAO']:
        d["NSOUND_LIBAO"] = "#define NSOUND_LIBAO 1"
    else:
        d["NSOUND_LIBAO"] = "#undef NSOUND_LIBAO // disabled"

    nsound_config.Finish()

    # Generate Nsound.h and Doxyfile.

    nsound_h = env.AcGenerateFile(
        'src/Nsound/Nsound.h',
        'src/Nsound/Nsound.h.in',
        AC_GEN_DICT = d)

    doxyfile = env.AcGenerateFile(
        'docs/reference/Doxyfile',
        'docs/reference/Doxyfile.in',
        AC_GEN_DICT = d)

    env.Default(nsound_h)

#-----------------------------------------------------------------------------
# include libNsound sources

Export("nsound_h")
Export("nsound_config")

if not nsound_config.env.GetOption("help"):

    SConscript(["src/Nsound/SConscript"])
    SConscript(["src/bin/SConscript"])
    SConscript(["src/examples/SConscript"])
    SConscript(["src/test/SConscript"])

    if build_py_module:
        nsound_config.env.Tool("SwigGen")
        SConscript(["swig/SConscript"])

    Import("*")

    #--------------------------------------------------------------------------
    # Extra Cleaning

    env = nsound_config.env

    env.Clean(libNsound, "Nsound.py")
    env.Clean(libNsound, "swig/Nsound.py")
    env.Clean(libNsound, "bin")
    env.Clean(libNsound, "build")
    env.Clean(libNsound, "dist")
    env.Clean(libNsound, "docs/user_guide/build/doctrees")
    env.Clean(libNsound, "docs/user_guide/build/html")
    env.Clean(libNsound, "docs/user_guide/build/plot_directive")
    env.Clean(libNsound, "lib")
    env.Clean(libNsound, "swig/nsound_wrap.cxx")
    env.Clean(libNsound, "swig/nsound_wrap.os")
    env.Clean(libNsound, doxyfile)
    env.Clean(libNsound, glob.glob("docs/user_guide/source/_static/*.mp3"))
    env.Clean(libNsound, glob.glob("docs/user_guide/source/_static/*.wav"))
    env.Clean(libNsound, glob.glob("example*.wav"))
    env.Clean(libNsound, glob.glob("nsound-*.tar.gz"))
    env.Clean(libNsound, glob.glob("nsound-*.zip"))
    env.Clean(libNsound, glob.glob("src/examples/Temperature_*_Shifted_*.wav"))
    env.Clean(libNsound, glob.glob("src/examples/Temperature_out.wav"))
    env.Clean(libNsound, glob.glob("src/examples/example*.wav"))
    env.Clean(libNsound, glob.glob("src/examples/mynameis-*.wav"))
    env.Clean(libNsound, glob.glob("src/test/*.wav"))
    env.Clean(libNsound, nsound_h)
    env.Clean(libNsound, setup_builder_py)
    env.Clean(libNsound, unit_tests)

    # The conditional compiled objects aren't cleaned by default, I'll have to
    # fix this in the future.

    env.Clean(libNsound, glob.glob("src/Nsound/AudioBackendLibao.os"))
    env.Clean(libNsound, glob.glob("src/Nsound/AudioBackendLibportaudio.os"))
    env.Clean(libNsound, glob.glob("src/Nsound/CudaUtils.os"))
    env.Clean(libNsound, glob.glob("src/Nsound/StretcherCuda.os"))

    #--------------------------------------------------------------------------
    # Aliases

    env.Alias("reference", nsound_h)
    env.Alias("reference", doxyfile)

    env.Alias("lib", nsound_h)
    env.Alias("lib", libNsound)

    env.Alias("test", nsound_h)
    env.Alias("test", unit_tests)

    env.Alias("install", [env['NS_BINDIR'], env['NS_LIBDIR']])

    #--------------------------------------------------------------------------
    # Setup Tar and Zip

    EXCLUDE_EXTENSIONS = [
#~        ".a",  need to include mingw32 libs
        ".cache",
        ".cvsignore",
        ".dblite",
        ".dll",
        ".gz",
        ".jpg",
        ".log",
        ".mp3",
        ".o",
        ".os",
        ".png",
        ".pyc",
        ".so",
        ".tar",
        ".tmp",
        ".zip",
        "core",
        "~"
        ]

    EXCLUDE_DIRS = [
        ".svn",
        ".sconf_temp",
        "./bin",
        "./lib",
        "./docs/user_guide/build",
        "./msw"
        ]

    if 'release' in COMMAND_LINE_TARGETS:
        if(use_disttar):
            env['DISTTAR_FORMAT']='gz'

            env.Append(\
                DISTTAR_EXCLUDE_EXTS = EXCLUDE_EXTENSIONS,
                DISTTAR_EXCLUDE_DIRS = EXCLUDE_DIRS)

            tar = env.DistTar(PACKAGE_RELEASE + ".tar.gz", [env.Dir(".")])

            env.Alias("release", tar)


# :mode=python:
