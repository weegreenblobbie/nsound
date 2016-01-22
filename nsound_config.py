"""
$Id: nsound_config.py 913 2015-08-08 16:41:22Z weegreenblobbie $

Nsound is a C++ library and Python module for audio synthesis featuring
dynamic digital filters. Nsound lets you easily shape waveforms and write
to disk or plot them. Nsound aims to be as powerful as Csound but easy to
use.

Copyright (c) 2009-Present Nick Hilton

A baseclass for configuring Nsound for different computing platforms.
"""

# Python imports
import distutils.sysconfig
import os
import platform
import sys

# Scons imports
from SCons.SConf import CheckContext
from SCons.SConf import SConfBase
from SCons.Script import COMMAND_LINE_TARGETS
from SCons.Script import Environment
from SCons.Script import GetOption


#------------------------------------------------------------------------------
# Globals & constants

SUCCESS = 1
FAILURE = 0
BUILD_FAILED = -999999999

class NsoundConfig(SConfBase):
    """
    A baseclass for configure tests with SCons.
    """

    def __init__(self):

        #----------------------------------------------------------------------
        # Setup default environment, checking command line options

        env = Environment(
            ENV = os.environ,
            LIBPATH = [],
            LIBS = [])

        env['NS_ON_CYGWIN'] = False
        env['NS_ON_LINUX'] = False
        env['NS_ON_MAC'] = False
        env['NS_ON_WINDOWS'] = False

        # Check options
        env['NS_BUILD_STATIC'] = GetOption("build_static")
        env['NS_COMPILER'] = GetOption("compiler")
        env['NS_CONFIG_DEBUG'] = GetOption("config_debug")
        env['NS_DEBUG_BUILD'] = GetOption("debug_build")
        env['NS_DISABLE_64'] = GetOption("disable_64")
        env['NS_DISABLE_CUDA'] = not GetOption("enable_cuda")
        env['NS_DISABLE_LIBAO'] = GetOption("disable_libao")
        env['NS_DISABLE_LIBPORTAUDIO'] = GetOption("disable_libportaudio")
        env['NS_DISABLE_OPENMP'] = GetOption("disable_openmp")
        env['NS_DISABLE_PYTHON'] = GetOption("disable_python")
        env['NS_EXTRA_WARNINGS'] = GetOption("extra_warnings")

        # There was a bug in either openMP or matplotlib that caused
        # segmentation faults when used together, last tried in 2010.
        if "setup_builder.py" in COMMAND_LINE_TARGETS:
            env['NS_DISABLE_OPENMP'] = True

        env['NS_HAVE_BOOST']        = False
        env['NS_HAVE_CPP11']        = False
        env['NS_HAVE_CUDA']         = False
        env['NS_HAVE_LIBAO']        = False
        env['NS_HAVE_LIBPORTAUDIO'] = False
        env['NS_HAVE_OPENMP']       = False
        env['NS_HAVE_PYLAB_C_API']  = False
        env['NS_HAVE_PYTHON_H']     = False

        env['NSOUND_PLATFORM_OS'] = 'Unknown'

        # Boost header
        boost_prefix = GetOption("boost_prefix")

        if boost_prefix:
            if os.path.isdir(boost_prefix):
                env.AppendUnique(CPPPATH = [boost_prefix])

        #----------------------------------------------------------------------
        # Setup installer prefixes

        prefix = GetOption("prefix")

        if prefix is None:
            prefix = os.path.dirname(os.path.realpath(__file__))

        env['NS_BINDIR'] = os.path.join(prefix, "bin")
        env['NS_LIBDIR'] = os.path.join(prefix, "lib")

        #----------------------------------------------------------------------
        # Add local tools

        env.Tool('ImportPythonConfig')
        env.Tool('AcGenerateFile')

        if env['NS_COMPILER']:
            env['NS_COMPILER'] = env['NS_COMPILER'].lower()

            compiler = env['NS_COMPILER']

            if compiler in ['gcc', 'g++']:
                env.Tool(compiler)

            elif compiler in ['mingw']:
                env.Tool(compiler)

            elif compiler in ['clang', 'clang++']:
                env.Tool('g++')
                env['CXX'] = 'clang++'

        #----------------------------------------------------------------------
        # Supress command output?

        if not GetOption('verbose'):
            env['ARCOMSTR']          = "Archiving $TARGET"
            env['CCCOMSTR']          = "Compiling $SOURCE"
            env['CXXCOMSTR']         = env['CCCOMSTR']
            env['LINKCOMSTR']        = "Linking   $TARGET"
            env['PYTHONSETUPCOMSTR'] = "Building Python Extension with $SOURCE"
            env['RANLIBCOMSTR']      = "Indexing  $TARGET"
            env['SHCCCOMSTR']        = env['CCCOMSTR']
            env['SHCXXCOMSTR']       = env['CCCOMSTR']
            env['SHLINKCOMSTR']      = env['LINKCOMSTR']
            env['LDMODULECOMSTR']    = env['LINKCOMSTR']
            env['SWIGGENCOMSTR']     = "Generating SWIG interface $TARGET"

        else:
            print ""
            print "CXXFLAGS = %s" % env['CXXFLAGS']
            print "LINKFLAGS = %s" % env['LINKFLAGS']
            print "LIBPATH = %s" % env['LIBPATH']
            print "LIBS = %s" % env['LIBS']
            print "LINKFLAGS = %s" % env['LINKFLAGS']
            print ""

        self.env = env
        self._orig_env = env

        #----------------------------------------------------------------------
        # Platform specific customization

        self._customize_environment()

        #----------------------------------------------------------------------
        # Insert python config into environment
        if self.env['HAVE_PYTHON_CONFIG']:

            cpppath = self.env['PYTHON_CONFIG']['CPPPATH']
            libpath = self.env['PYTHON_CONFIG']['LIBPATH']
            libs = self.env['PYTHON_CONFIG']['LIBS']

            self.env.AppendUnique(
                CPPPATH = cpppath,
                LIBPATH = libpath,
                LIBS = libs)

            self.add_to_rpath(libpath)

        SConfBase.__init__(self, env)

    def _make_check_context(self):
        """
        We backup the scons environment object so test are executed in an
        environment object that we don't have to fix if the test fails.
        """

        self._orig_env = self.env
        self.env = self.env.Clone()

        return CheckContext(self)

    def _restore_env(self):
        """
        After a context test, we restore the environt back to the original.
        """
        self.env = self._orig_env

    def _get_result(self, results, split = True):

#~        print "results = ", results

        if results is None or results == 0:
            return BUILD_FAILED, ''

        elif results == 1:
            return SUCCESS, ''

        elif results[0]:

            stdout = results[1].strip()

            if split:
                stdout = stdout.split()

                if len(stdout):
                    stdout = stdout[0]

            return SUCCESS, stdout

        else:
            return FAILURE, results[1].strip()

    def on_cygwin(self):
        return self.env['NS_ON_CYGWIN']

    def on_linux(self):
        return self.env['NS_ON_LINUX']

    def on_mac(self):
        return self.env['NS_ON_MAC']

    def on_windows(self):
        return self.env['NS_ON_WINDOWS']

    def add_custom_compiler_flags(self):
        """
        After detecting the compiler version, adjust flags as necessary.
        """
        pass

    def add_to_rpath(self, paths):
        """
        Adds the path to the dynamic library link flags to encode the full
        path name to the dynamic library.  If the path does not exist, it will
        get created.
        """

        if self.on_windows():
            return

        if not isinstance(paths, list):
            paths = [paths]

        linkflags = []

        for p in paths:

            linkflags.append("-Wl,-rpath=%s" % p)

        self.env.MergeFlags({'LINKFLAGS' : linkflags})

    def check_compiler_works(self):
        context = self._make_check_context()
        context.Message("checking if compiler works ... ")

        r = context.TryRun("""
            #include <stdio.h>

            int main(void)
            {
                printf("good");

                return 0;
            }

        """, ".cpp")

        ecode, stdout = self._get_result(r)

        if ecode == SUCCESS and stdout == "good":
            context.Result("good")
            return "good"

        context.Result("bad, check config.log for details")
        return "bad"

    def check_endian(self):
        context = self._make_check_context()

        context.Message("checking endianess ... ")

        import struct

        array = struct.pack('cccc', '\x01', '\x02', '\x03', '\x04')

        i = struct.unpack('i', array)

        key = 'NS_ENDIAN_IS_LITTLE'

        # Little Endian
        if i == struct.unpack('<i', array):
            context.Result("little")
            self.env[key] = True
            return "little"

        # Big Endian
        elif i == struct.unpack('>i', array):
            context.Result("big")
            self.env[key] = False
            return "big"

        context.Result("unknown, assuming little")
        self.env[key] = True

        return "unknown"

    def check_for_boost(self):

        context = self._make_check_context()

        context.Message("checking for boost unit_test headers ... ")

        key = 'NS_HAVE_BOOST'

#~        r = context.TryRun("""

#~        #include <boost/test/included/unit_test.hpp>

#~        #include <iostream>

#~        BOOST_AUTO_TEST_CASE( test1 )
#~        {
#~            BOOST_CHECK_EQUAL( 1, 1 );

#~            std::cout << "yes" << std::endl;
#~        }

#~        bool init_func() { return true; }

#~        int
#~        main(int argc, char ** argv)
#~        {
#~            return boost::unit_test::unit_test_main(
#~                reinterpret_cast<boost::unit_test::init_unit_test_func>(
#~                    &init_func),
#~                argc,
#~                argv);
#~        }

#~        """, ".cpp")

        r = context.TryRun("""
            #define BOOST_TEST_ALTERNATIVE_INIT_API 1

            #include <boost/test/unit_test.hpp>

            #include <iostream>

            void reduction_test()
            {
                std::cout << "yes";
            }

            bool init_unit_test()
            {
                boost::unit_test::framework::master_test_suite().add(
                    BOOST_TEST_CASE(&reduction_test ));
                return true;
            }

            int main( int argc, char* argv[] )
            {
              return ::boost::unit_test::unit_test_main( &init_unit_test, argc,
                                                                        argv );
            }
        """, ".cpp")

        ecode, stdout = self._get_result(r)

        if ecode == SUCCESS and stdout == "yes":
            context.Result(stdout)
            self.env[key] = True
            return True

        elif ecode == SUCCESS:
           context.Result("no")
           self.env[key] = False
           return False

        context.Result("error, check config.log for details!")
        self.env[key] = False
        return False

    def detect_compile_version(self):

        context = self._make_check_context()

        context.Message("detecting compiler version ... ")

        key = 'NS_COMPILER'

        r = context.TryRun(r"""
            #include <stdio.h>

            int main(void)
            {
                //-------------------------------------------------------------
                // Clang - check for clang first since it also defines __GNUC__
                #if defined(__clang__)

                    printf("clang %d %d %d\n",
                        __GNUC__,
                        __GNUC_MINOR__,
                        __GNUC_PATCHLEVEL__);

                //-------------------------------------------------------------
                // GCC

                #elif !defined(__CUDACC__) && defined(__GNUC__)

                    printf("gcc %d %d %d\n",
                        __GNUC__,
                        __GNUC_MINOR__,
                        __GNUC_PATCHLEVEL__);

                #elif !defined(__MINGW32__) && defined(_MSC_VER)

                    printf("msvc %d ", _MSC_VER);

                    #if (_MSC_VER >= 1600) && (_MSC_VER < 1700)
                        printf("vc100 ");
                    #elif (_MSC_VER < 1800)
                        printf("vc110 ");
                    #elif (_MSC_VER < 1900)
                        printf("vc120 ");
                    #elif (_MSC_VER < 2000)
                        printf("vc140 ");
                    #else
                        #error "Unknown msvc compiler version!"
                    #endif

                    printf("%d\n", _MSC_FULL_VER);

                #else
                    #error "Unknown compiler!"
                #endif

                return 0;
            }

        """, ".cpp")

        ecode, stdout = self._get_result(r, False)

        if ecode == SUCCESS:

            d = {}

            tokens = stdout.split()

            compiler = tokens[0]

            d['compiler'] = compiler

            if compiler == 'msvc':

                d['version'] = int(tokens[1])
                d['toolset'] = tokens[2]
                d['longver'] = int(tokens[3])

            else:

                ver = [int(x) for x in tokens[1:]]

                d['version'] = ver
                d['toolset'] = '%s%d.%d' % (d['compiler'], ver[0], ver[1])
                d['longver'] = ver[0] * 10000 + ver[1] * 100 + ver[2]

            self.env[key] = d
            context.Result(stdout)
            return True

        context.Result("error, check config.log for details!")
        self.env[key] = None
        return False

    def check_for_pi_macro(self):

        context = self._make_check_context()

        context.Message("checking if math.h defines M_PI ... ")

        key = 'NS_HAVE_M_PI'

        r = context.TryRun("""
            #include <stdio.h>
            #include <math.h>

            int main(void)
            {
                #ifdef M_PI
                    printf("yes");
                #else
                    printf("no");
                #endif

            return 0;
            }

        """, ".cpp")

        ecode, stdout = self._get_result(r)

        if ecode == SUCCESS and stdout == "yes":
            context.Result(stdout)
            self.env[key] = True
            return True

        elif ecode == SUCCESS:
           context.Result("no")
           self.env[key] = False
           return False

        context.Result("error, check config.log for details!")
        self.env[key] = False
        return False

    def check_for_cpp11(self):

        context = self._make_check_context()

        context.Message("checking for C++11 features ... ")

        key = 'NS_HAVE_CPP11'

        r = context.TryRun("""
            #include <iostream>

            #include <atomic>
            #include <chrono>
            #include <thread>
            #include <vector>

            void my_atomic_init(std::atomic_uint & a, unsigned int v)
            {
                // Windows
                #ifdef __WIN32__
                    std::atomic_init(&a, v);
                #else
                    a = v;
                #endif
            }

            int main(void)
            {
                // sleep for 0.100 seconds
                std::chrono::milliseconds duration( 100 );

                std::this_thread::sleep_for( duration );

                std::atomic_uint a_count;

                my_atomic_init(a_count, 0u);

                a_count.fetch_add(1u);

                std::cout << "yes" << std::endl;
                std::cout << "count = " << a_count.load();

                std::vector<int> v = {1,2,3,4,5};

                for(auto x : v)
                {
                    std::cout << "x = " << x << std::endl;
                }

                return 0;
            }

        """, ".cpp")

        ecode, stdout = self._get_result(r)

        if ecode == SUCCESS and stdout == "yes":
            context.Result(stdout)
            self.env[key] = True
            return True

        elif ecode == SUCCESS:
           context.Result("no")
           self.env[key] = False
           return False

        context.Result("error, check config.log for details!")
        self.env[key] = False
        return False

    def check_for_matplotlib(self):
        context = self._make_check_context()

        context.Message("checking for matplotlib.pylab ... ")

        key = 'NS_HAVE_MATPLOTLIB'

        if self.is_python_disabled():
            context.Result("no, Python disabled on command line")
            self.env[key] = False

        try:
            import matplotlib
        except ImportError:
            context.Result(
                'no, matplotlib is missing, no plotting is possible')
            self.env[key] = False
            return False

        try:
            import matplotlib.pylab
        except ImportError:
            context.Result(
                'no, matplotlib.pylab is missing, no plotting is possible')
            self.env[key] = False
            return False

        context.Result("yes")
        self.env[key] = True
        return True

    def check_for_matplotlib_c_api(self):

        context = self._make_check_context()

        context.Message("checking for Matplotlib C API ... ")

        key = 'NS_HAVE_MATPLOTLIB_C_API'

        if self.is_python_disabled():
            context.Result("no, Python disabled on command line")
            self.env[key] = False
            return False

        elif not self.env['NS_HAVE_PYTHON_H']:
            context.Result("no, missing Python.h")
            self.env[key] = False
            return False

        elif not self.env['NS_HAVE_MATPLOTLIB']:
            context.Result("no, missing matplotlib.pylab module")
            self.env[key] = False
            return False

        elif not self.env['HAVE_PYTHON_CONFIG']:
            context.Result(
                "no, ImportPythonConfig faild to detect Python development "
                "environment")
            self.env[key] = False
            return False

        context = self._make_check_context()

        kwargs = {
            'CPPPATH' : self.env['PYTHON_CONFIG']['CPPPATH'],
            'LIBPATH' : self.env['PYTHON_CONFIG']['LIBPATH'],
            'LIBS'    : self.env['PYTHON_CONFIG']['LIBS'],
            }

        self.env.AppendUnique(**kwargs)

        r = context.TryRun(text = """
            #include <Python.h>
            #include <stdio.h>

            int main(void)
            {
                PyObject * ptr;

                Py_Initialize();

                ptr = PyImport_ImportModule("matplotlib");

                if(ptr == NULL)
                {
                    printf("no");
                    PyErr_Print();
                    PyErr_Clear();
                }
                else
                {
                    ptr = PyImport_ImportModule("matplotlib.pylab");

                    if(ptr == NULL)
                    {
                        printf("no");
                        PyErr_Print();
                        PyErr_Clear();
                    }
                    else
                    {
                        printf("yes");
                        Py_DECREF(ptr);
                    }
                }

                Py_Finalize();

                return 0;
            }

        """, extension = ".cpp")

        ecode, stdout = self._get_result(r)

        if ecode == SUCCESS and stdout == "yes":
            context.Result("yes")
            self.env[key] = True
            return True

        # On any failure, restore the build environment.
        self._restore_env()

        if ecode == SUCCESS and stdout == "no":
            context.Result("no, plotting will be disabled.")
            self.env[key] = False
            return False

        context.Result("no, error occured, check config.log")
        self.env[key] = False
        return False

    def check_for_portaudio_h_v18(self):

        context = self._make_check_context()

        context.Message("checking for portaudio.h V18 ... ")

        key = 'NS_HAVE_PORTAUDIO_H_V18'

        if self.env['NS_DISABLE_LIBPORTAUDIO']:
            context.Result("no, disabled on command line")
            self.env[key] = False
            return False

        elif not self.env['NS_HAVE_PORTAUDIO_H']:
            context.Result("no, missing portaudio.h")
            self.env[key] = False
            return False

        r = context.TryCompile(text = """
            #include <portaudio.h>
            #include <stdio.h>

            int main(void)
            {
                PaDeviceInfo info;

                int n_sample_rates = info.numSampleRates;

                double sr = 0.0;

                if(n_sample_rates)
                {
                    sr = info.sampleRates[0];
                }

                return 0;
            }

        """, extension = ".cpp")

        ecode, stdout = self._get_result(r)

        if ecode == SUCCESS:
            context.Result("yes")
            self.env[key] = True
            return True

        context.Result("no")
        self.env[key] = False
        return False

    def check_for_portaudio_h_v19(self):

        context = self._make_check_context()

        context.Message("checking for portaudio.h V19 ... ")

        key = 'NS_HAVE_PORTAUDIO_H_V19'

        if self.env['NS_DISABLE_LIBPORTAUDIO']:
            context.Result("no, disabled on command line")
            self.env[key] = False
            return False

        elif not self.env['NS_HAVE_PORTAUDIO_H']:
            context.Result("no, missing portaudio.h")
            self.env[key] = False
            return False

        r = context.TryCompile( text = """
            #include <portaudio.h>
            #include <stdio.h>

            int main(void)
            {
                PaDeviceInfo info;

                double sr = info.defaultSampleRate;

                return 0;
            }

        """, extension = ".cpp")

        ecode, stdout = self._get_result(r)

        if ecode == SUCCESS:
            context.Result("yes")
            self.env[key] = True
            return True

        context.Result("no")
        self.env[key] = False
        return False

    def check_for_libportaudio(self):

        context = self._make_check_context()

        context.Message("checking for libportaudio ... ")

        key = 'NS_HAVE_LIBPORTAUDIO'

        if self.env['NS_DISABLE_LIBPORTAUDIO']:
            context.Result("no, disabled on command line")
            self.env[key] = False
            return False

        elif self.env['NS_HAVE_PORTAUDIO_H_V18']:
            context.Result(
                "no, portaudio.h is V18, please install portaudio19-dev")
            self.env[key] = False
            return False

        elif not self.env['NS_HAVE_PORTAUDIO_H_V19']:
            context.Result("no, missing portaudio.h V19")
            self.env[key] = False
            return False

        kwargs = {'LIBS' : ["portaudio"]}

        if self.env['NS_ON_WINDOWS'] or self.env['NS_ON_CYGWIN']:
            kwargs['LIBS'].append("winmm")

        self.env.AppendUnique(**kwargs)

        r = context.TryRun(text = r"""

            extern "C"
            {
                void ___chkstk_ms(){}
            }

            #include <portaudio.h>
            #include <stdio.h>
            #include <assert.h>

            #include <string>

            using namespace std;

            string
            to_string(PaHostApiTypeId id)
            {
                switch(id)
                {
                    case paInDevelopment:   return "paInDevelopment";
                    case paDirectSound:     return "paDirectSound";
                    case paMME:             return "paMME";
                    case paASIO:            return "paASIO";
                    case paSoundManager:    return "paSoundManager";
                    case paCoreAudio:       return "paCoreAudio";
                    case paOSS:             return "paOSS";
                    case paALSA:            return "paALSA";
                    case paAL:              return "paAL";
                    case paBeOS:            return "paBeOS";
                    case paWDMKS:           return "paWDMKS";
                    case paJACK:            return "paJACK";
                    case paWASAPI:          return "paWASAPI";
                    case paAudioScienceHPI: return "paAudioScienceHPI";
                }
            }

            void
            dump_info(FILE * fd)
            {
                fprintf(fd, "%s(%d): dump_info():\n", __FILE__, __LINE__);

                unsigned default_host_api  = Pa_GetDefaultHostApi();

                unsigned default_device_id = Pa_GetDefaultOutputDevice();

                if(default_device_id == paNoDevice)
                {
                    fprintf(fd, "Pa_GetDefaultOutputDevice() failed, no default "
                        "output device\n");
                }

                fprintf(fd, "libportaudio Device Count:   %d\n", Pa_GetDeviceCount());
                fprintf(fd, "libportaudio Host API Count: %d\n", Pa_GetHostApiCount());

                const PaHostApiInfo * default_api_info = NULL;

                default_api_info = Pa_GetHostApiInfo(default_host_api);

                if(default_api_info == NULL)
                {
                    fprintf(fd, "Pa_GetHostApiInfo(default_host_api) failed!\n");
                }
                else
                {
                    fprintf(fd, "libportaudio Default Host API: %s\n", default_api_info->name);
                }

                fprintf(fd, "libportaduio Default Device Id: %d\n", default_device_id);

                for(unsigned i = 0; i < Pa_GetHostApiCount(); ++i)
                {
                    const PaHostApiInfo * api_info = NULL;

                    api_info = Pa_GetHostApiInfo(i);

                    if(api_info == NULL)
                    {
                        fprintf(fd, "Pa_GetHostApiCount(%d) failed\n", i);
                        continue;
                    }

                    fprintf(fd, "libportaudio Host API Info:\n");
                    fprintf(fd, "    index:               %d", i);

                    if(i == default_host_api) fprintf(fd, " *default API*");

                    fprintf(fd, "\n");

                    fprintf(fd, "    structVersion:       %d\n", api_info->structVersion);
                    fprintf(fd, "    type:                %s\n", to_string(api_info->type).c_str());
                    fprintf(fd, "    name:                %s\n", api_info->name);
                    fprintf(fd, "    deviceCount:         %d\n", api_info->deviceCount);
                    fprintf(fd, "    defaultInputDevice:  %d\n", api_info->defaultInputDevice);
                    fprintf(fd, "    defaultOutputDevice: %d\n", api_info->defaultOutputDevice);

                    for(unsigned j = 0; j < api_info->deviceCount; ++j)
                    {
                        fprintf(fd, "    Device %d:", j);

                        if(j == default_device_id) fprintf(fd, " *default device*");

                        fprintf(fd, "\n");

                        const PaDeviceInfo * dev_info = Pa_GetDeviceInfo(j);

                        if(dev_info == NULL)
                        {
                            fprintf(fd, "        dev_info == NULL!\n");
                            continue;
                        }

                        fprintf(fd, "        structVersion:     %d\n", dev_info->structVersion);
                        fprintf(fd, "        name:              %s\n", dev_info->name);
                        fprintf(fd, "        hostApi:           %d\n", dev_info->hostApi);
                        fprintf(fd, "        maxInputChannels:  %d\n", dev_info->maxInputChannels);
                        fprintf(fd, "        maxOutputChannels: %d\n", dev_info->maxInputChannels);
                        fprintf(fd, "        defaultSampleRate: %.1f\n", dev_info->defaultSampleRate);
                        fprintf(fd, "        defaultLowInputLatency:   %9.6f\n", dev_info->defaultLowInputLatency);
                        fprintf(fd, "        defaultLowOutputLatency:  %9.6f\n", dev_info->defaultLowOutputLatency);
                        fprintf(fd, "        defaultHighInputLatency:  %9.6f\n", dev_info->defaultHighInputLatency);
                        fprintf(fd, "        defaultHighOutputLatency: %9.6f\n", dev_info->defaultHighOutputLatency);
                    }
                }
            }

            #define LOGFILE "portaudio.log"

            int main(void)
            {
                // Zero out logfile.
                FILE * log = fopen(LOGFILE, "w");
                if(log) fclose(log);

                PaError err = Pa_Initialize();

                if(err != paNoError)
                {
                    printf("Pa_Initialize() failed\n");
                    return 1;
                }

                printf("yes\n");

                log = fopen(LOGFILE, "w");

                if(log)
                {
                    dump_info(log);
                    fclose(log);
                }

                return 0;
            }

        """, extension = ".cpp")

        ecode, stdout = self._get_result(r)

        if ecode == SUCCESS and stdout == "yes":
            context.Result("yes")
            self.env[key] = True
            return True

        # On any failure, restore the build environment.
        self._restore_env()

        if ecode == SUCCESS:
            context.Result("no, " + stdout)
            self.env[key] = False
            return False

        context.Result("error, check config.log for details!")
        self.env[key] = False
        return False

    def check_for_libao(self):

        context = self._make_check_context()

        context.Message("checking for libao ... ")

        key = 'NS_HAVE_LIBAO'

        if self.env['NS_DISABLE_LIBAO']:
            context.Result("no, disabled on command line")
            self.env[key] = False
            return False

        elif not self.env['NS_HAVE_AO_AO_H']:
            context.Result("no, missing ao/ao.h")
            self.env[key] = False
            return False

        kwargs = {'LIBS' : ["ao"]}

        self.env.AppendUnique(**kwargs)

        r = context.TryRun(text = """
            extern "C"
            {
                const char * KSDATAFORMAT_SUBTYPE_PCM = "00000001-0000-0010-8000-00aa00389b71";
            }

            #include <ao/ao.h>
            #include <stdio.h>

            int main(void)
            {
                ao_initialize();

                int driver_id = ao_default_driver_id();

                if(driver_id < 0)
                {
                    ao_shutdown();

                    printf("no, ao_default_driver_id() failed\\n");

                    return 1;
                }

                ao_info * info = ao_driver_info(driver_id);

                if(info == NULL)
                {
                    ao_shutdown();

                    printf("no, ao_driver_info() failed\\n");

                    return 1;
                }

                printf("yes\\n");
                printf("libao Default Driver Info:\\n");
                printf("    type:       ");

                switch(info->type)
                {
                    case AO_TYPE_LIVE:
                        printf("AO_TYPE_LIVE\\n");
                        break;

                    case AO_TYPE_FILE:
                        printf("AO_TYPE_FILE\\n");
                        break;

                    default:
                        printf("Unknown!\\n");
                        break;
                }

                printf("    name:       %s\\n", info->name);
                printf("    short_name: %s\\n", info->short_name);
                printf("    comment:    %s\\n", info->comment);
                printf("    preferred_byte_format: ");

                switch(info->preferred_byte_format)
                {
                    case AO_FMT_LITTLE:
                        printf("Little Endian\\n");
                        break;

                    case AO_FMT_BIG:
                        printf("Big Endian\\n");
                        break;

                    case AO_FMT_NATIVE:

                        if(ao_is_big_endian())
                        {
                            printf("Native Big Endian\\n");
                        }
                        else
                        {
                            printf("Native Little Endian\\n");
                        }
                        break;

                    default:
                        printf("Unknown!\\n");
                        break;
                }

                printf("    priority:   %d\\n", info->priority);
                printf("    option count: %d\\n", info->option_count);

                for(int i = 0; i < info->option_count; ++i)
                {
                    printf("    options[%d]: %s\\n", i, info->options[i]);
                }

                ao_shutdown();

                return 0;
            }

        """, extension = ".cpp")

        ecode, stdout = self._get_result(r)

        if ecode == SUCCESS and stdout == "yes":
            context.Result("yes")
            self.env[key] = True
            return True

        # On any failure, restore the build environment.
        self._restore_env()

        if ecode == SUCCESS:
            context.Result("no, " + stdout)
            self.env[key] = False
            return False

        context.Result("error, check config.log for details!")
        self.env[key] = False
        return False

    def check_for_cuda(self):

        # FIXME: create a proper scons tool for the cuda tool kit.

        context = self._make_check_context()

        context.Message("checking for Cuda Toolkit ... ")

        key = 'NS_HAVE_CUDA'

        if self.env['NS_DISABLE_CUDA']:
            context.Result("no, disabled on command line")
            self.env[key] = False
            return False

        # Search prefixes
        home         = os.environ.get('HOME', '')
        programfiles = os.environ.get('PROGRAMFILES', '')
        homedrive    = os.environ.get('HOMEDRIVE', '')

        CUDA_TOOLKIT_PATH = os.getenv("CUDA_TOOLKIT_PATH")
        # Search for it
        if CUDA_TOOLKIT_PATH == None:
            # Set to something logical for search
            CUDA_TOOLKIT_PATH = "/usr/local/cuda"

        PATHS = \
        [
            CUDA_TOOLKIT_PATH,
            '/usr/local/cuda',
            '/usr/local/CUDA',
            '/usr/local/CUDA_TOOLKIT',
            '/usr/local/NVIDIA_CUDA_TOOLKIT',
            '/usr/local/cuda_toolkit',
            home + '/NVIDIA_CUDA_TOOLKIT',
            home + '/Apps/NVIDIA_CUDA_TOOLKIT',
            home + '/Apps/NVIDIA_CUDA_TOOLKIT',
            home + '/Apps/CudaToolkit',
            home + '/Apps/CudaTK',
            '/Developer/NVIDIA CUDA TOOLKIT',
            '/Developer/CUDA TOOLKIT',
            '/Developer/CUDA',
            programfiles + 'NVIDIA Corporation/NVIDIA CUDA TOOLKIT',
            programfiles + 'NVIDIA Corporation/NVIDIA CUDA',
            programfiles + 'NVIDIA Corporation/CUDA TOOLKIT',
            programfiles + 'NVIDIA Corporation/CUDA',
            programfiles + 'NVIDIA/NVIDIA CUDA TOOLKIT',
            programfiles + 'NVIDIA/NVIDIA CUDA',
            programfiles + 'NVIDIA/CUDA TOOLKIT',
            programfiles + 'NVIDIA/CUDA',
            programfiles + 'CUDA TOOLKIT',
            programfiles + 'CUDA',
            homedrive + '/CUDA TOOLKIT',
            homedrive + '/CUDA'
        ]

        cuda_toolkit_path = None

        for path in PATHS:
            if os.path.isdir(path):
                cuda_toolkit_path = path
                break

        if cuda_toolkit_path == None:
            context.Result(
                "no, could not find the Cuda Toolkit, try setting "
                "CUDA_TOOLKIT_PATH environment variable")
            self.env[key] = False
            return False

        # Yes, we found it!

        self.env["CUDA_TOOLKIT_PATH"] = cuda_toolkit_path
        self.env = self.env.Clone(tools = ["CudaTool"])

        r = context.TryRun(text = """
            #include <stdio.h>
            #include <cuda.h>

            #define checkError()                            \\
            {                                               \\
                cudaError_t ecode = cudaGetLastError();     \\
                if(ecode)                                   \\
                {                                           \\
                    printf("%s:%3d: Error: %s\\n",          \\
                        __FILE__,                           \\
                        __LINE__,                           \\
                        cudaGetErrorString(ecode));         \\
                }                                           \\
            }

            // Kernel definition
            __global__
            void
            VecAdd(int * C, const int * A, const int * B)
            {
                int i = threadIdx.x;
                C[i] = A[i] + B[i];
            }

            int main(void)
            {
                int a[3] = {  1,  2,  3};
                int b[3] = { 10, 10, 10};
                int c[3] = {  0,  0,  0};

                int * gpuA = NULL;
                int * gpuB = NULL;
                int * gpuC = NULL;

                const int N_BYTES = sizeof(int) * 3;

                if(CUDA_SUCCESS != cuInit(0))
                {
                    printf("No");
                    checkError();
                }
                else
                {
                    // Allocate the GPU pointers
                    cudaMalloc((void**)&gpuA, N_BYTES); checkError();
                    cudaMalloc((void**)&gpuB, N_BYTES); checkError();
                    cudaMalloc((void**)&gpuC, N_BYTES); checkError();

                    // Copy the data to the device
                    cudaMemcpy(gpuA, a, N_BYTES, cudaMemcpyHostToDevice); checkError();
                    cudaMemcpy(gpuB, b, N_BYTES, cudaMemcpyHostToDevice); checkError();

                    // Kernel invocation with N threads
                    VecAdd<<<1, 3>>>(gpuC, gpuA, gpuB); checkError();

                    // Copy the data back to the host
                    cudaMemcpy(c, gpuC, N_BYTES, cudaMemcpyDeviceToHost); checkError();

                    cudaFree(gpuC); checkError();
                    cudaFree(gpuB); checkError();
                    cudaFree(gpuA); checkError();

                    if( c[0] == 11 &&
                        c[1] == 12 &&
                        c[2] == 13)
                    {
                        printf("yes");
                    }
                    else
                    {
                        printf("no, cuda kernel did not calculate correct"
                            "answer.\\n");
                        printf("%d %d %d ", a[0], a[1], a[2]);
                        printf("%d %d %d ", b[0], b[1], b[2]);
                        printf("%d %d %d",  c[0], c[1], c[2]);
                        printf("\\n");
                    }
                }
                return 0;
            }

        """, extension = ".cu")

        ecode, stdout = self._get_result(r)

        if ecode == SUCCESS and stdout == "yes":
            context.Result("yes")
            self.env[key] = True
            return True

        # On any failure, restore the build environment.
        self._restore_env()

        if ecode == SUCCESS:
            context.Result(stdout)
            self.env[key] = False
            return False

        context.Result("error, check config.log for details!")
        self.env[key] = False
        return False

    def check_for_open_mp(self):

        # FIXME: create proper scons tool for detecting OpenMP

        context = self._make_check_context()

        context.Message("checking if OpenMP works ... ")

        key = 'NS_HAVE_OPENMP'

        if self.env['NS_DISABLE_OPENMP']:
            context.Result("no, disabled on command line")
            self.env[key] = False
            return False

        if self.env['NSOUND_PLATFORM_OS'] == "NSOUND_PLATFORM_OS_WINDOWS":
            # FIXME
            pass
        else:
            self.env.AppendUnique(CXXFLAGS = ["-fopenmp"])
            self.env.AppendUnique(LIBS = ["gomp"])

        r = context.TryRun(text = """
            #include <stdio.h>
            #include <omp.h>

            int main(void)
            {
                int i = 0;

                #pragma omp parallel
                {
                    if( omp_in_parallel() )
                    {
                        if( 0 == omp_get_thread_num() )
                        {
                            printf("yes (%d threads)", omp_get_num_threads());
                        }
                    }
                    else
                    {
                        printf("no");
                    }
                }

                return 0;
            }

        """, extension = ".cpp")

        ecode, stdout = self._get_result(r)

        if ecode == SUCCESS and stdout == "yes":
            context.Result(stdout)
            self.env[key] = True
            return True

        # On any failure, restore the build environment.
        self._restore_env()

        if ecode == SUCCESS and stdout == "no":
            context.Result(
                "no, OpenMp program compiled and executed, but doesn't appear "
                "to spawn threads.")
            self.env[key] = False
            return False

        context.Result("no, error occured, check config.log")
        self.env[key] = False
        return False

    def check_header(self, header, *args, **kwargs):

        keyword = 'NS_HAVE_%s' % header
        keyword = keyword.replace('\\', '/')
        keyword = keyword.replace('/', '_')
        keyword = keyword.replace('.', '_')
        keyword = keyword.upper()

        self.env[keyword] = self.CheckHeader(header, *args, **kwargs)

    def is_static_build(self):
        return self.env['NS_BUILD_STATIC']

    def is_libao_disabled(self):
        return self.env['NS_DISABLE_LIBAO']

    def is_libportaudio_disabled(self):
        return self.env['NS_DISABLE_LIBPORTAUDIO']

    def is_python_disabled(self):
        return self.env['NS_DISABLE_PYTHON']

    # FIXME: move this to a tool in site_scons/site_tools
    @staticmethod
    def CudaTool(env):

        cudaToolkitPath = env['CUDA_TOOLKIT_PATH']

        CUDAScanner = SCons.Scanner.C.CScanner()

        staticObjBuilder, sharedObjBuilder = SCons.Tool.createObjBuilders(env);
        staticObjBuilder.add_action('.cu', '$STATICNVCCCMD')
        sharedObjBuilder.add_action('.cu', '$SHAREDNVCCCMD')
        SourceFileScanner.add_scanner('.cu', CUDAScanner)

        # default compiler
        env['NVCC'] = 'nvcc'

        # default flags for the NVCC compiler
        env['NVCCFLAGS'] = ''
        env['STATICNVCCFLAGS'] = ''
        env['SHAREDNVCCFLAGS'] = ''
        env['ENABLESHAREDNVCCFLAG'] = '-shared'

        # default NVCC commands
        env['STATICNVCCCMD'] = '$NVCC -o $TARGET -c $NVCCFLAGS $STATICNVCCFLAGS $SOURCES'
        env['SHAREDNVCCCMD'] = '$NVCC -o $TARGET -c $NVCCFLAGS $SHAREDNVCCFLAGS $ENABLESHAREDNVCCFLAG $SOURCES'

        # Add nvcc to PATH
        env.PrependENVPath('PATH', cudaToolkitPath + '/bin')

        env.Detect('nvcc')

        # Include path
        env.AppendUnique(NVCCFLAGS=[cudaToolkitPath + "/include"])

        # Library path
        bits = platform.architecture()[0]
        if "64" in bits:
            env.AppendUnique(LIBPATH=[cudaToolkitPath + "/lib64"])
        else:
            env.AppendUnique(LIBPATH=[cudaToolkitPath + "/lib"])

        env.AppendUnique(LIBS=["cuda", "cudart"])

    def Finish(self):
        self.env = SConfBase.Finish(self)

    def GetEncodeLibPath(self):
        return False

