*******************************************************************************
Building and installing Nsound
*******************************************************************************

===============================================================================
Binary installers
===============================================================================

There are currently no binary installers.

===============================================================================
Building The C++ Library From Source
===============================================================================

A general overview of building Nsound from source is given here, with detailed
instructions for specific platforms given separately.

-------------------------------------------------------------------------------
Prerequisites
-------------------------------------------------------------------------------

Building Nsound requires the following software installed:

1) Python 3.7 or later

    On Debian and derivative (Ubuntu): python

    On Windows: the official 64-bit Python installer at
    `www.python.org <http://www.python.org>`_ is enough

    On Mac OSX: I highly recommend the MacPorts package manager to install
    the latest versions of Python 3.7, Numpy and Matplotlib
    `www.macports.org <http://www.macports.org/install.php>`_ .

2) Scons

    Nsound uses this execellent build system,  `www.scons.org <http://www.scons.org>`_ .

    python3 -m pip install scons

    and install it by running the `dist/scons*.exe` file.

3) A C++ compiler

    To build Nsound you will need a C++ compiler.

    On Windows: you'll need to install Visual Studio.

    On Mac OS X: you'll need to install XCode command line tools.

-------------------------------------------------------------------------------
Optional Prerequisites
-------------------------------------------------------------------------------

1) Matplotlib

    For creating pretty plots, Matplotlib is used.

    Numpy and python-dateutil are a prerequisites for Matplotlib.

    python3 -m pip install matplotlib

2) PortAudio

    To enable audio playback through the soundcard, Nsound can use portaudio.

    PortAudio: `www.portaudio.com <http://www.portaudio.com>`_

    On Windows, portaudio have already been statically built and provided.

3) SWIG

    To build the Python module, SWIG generates the Python wrapper code.

    `www.swig.org <http://www.swig.org>`_

    On Windows: you'll download and unzip to ``C:\``.

..   a comment ``

^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Testing that all Matplotlib dependancies are met
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Usually this is only a problem for the Windows platform, since Linux and
MacPorts download and install prerequisite automatically.

1) Start a Python Shell and import matplotlib::

    >>> import matplotlib
    >>> # Try to import pyplot.
    >>> from matplotlib import pyplot
    >>> # Try to plot something
    >>> pyplot.plot([1,2,3,4], "bo-")
    >>> pyplot.show()

===============================================================================
Compiling The C++ Library
===============================================================================

.. pyexec::

    from Nsound import PACKAGE_RELEASE
    template = """
    1) Uncompress the Nsound archive::

        $ tar xfz %s.tar.gz""" % PACKAGE_RELEASE

.. pyexec::

    from Nsound import PACKAGE_RELEASE
    template = """
    2) Execute the SCons tool::

        cd %s
        scons""" % PACKAGE_RELEASE

SCons will query your computer for specific information about the build
environment and generate a few files.  It will then proceed to build the Nsound
library and examples.

.. pyexec::

    import subprocess
    output = subprocess.check_output(["scons", "-u", "--help"]).decode('ascii')
    template = "To get help with scons, do::\n\n"
    n = -1
    for line in output.split("\n"):
        n += 1
        if n == 0:
            template += "    $ scons --help\n"
            template += "    scons: Reading SConscript files ...\n"
        else:
            template += "    %s\n" % line

-------------------------------------------------------------------------------
On Windows: Compiling Nsound From The Command Line
-------------------------------------------------------------------------------

On Windows, rather than muck around with the System's PATH environment
variables, I recommend creating a .BAT file that will insert paths into the
environment.

1) Save this file to your doesktop :download:`windows_env64.bat <../../../../windows_env64.bat>`

2) Create a shortcut to the file ``windows_env64.bat``, edit the shortcut so the target line reads like this::

    cmd /k 'C:\Users\USERNAME\Desktop\windows_env64.bat'

 replacing `USERNAME` with your username.

3) Modify the shortcut startup directory to be located in the nsound directory, this way you won't have to cd to it all the time.

4) Double click the shortcut to open a DOS box with Visual Studio, Python, and SCons in the path, so building Nsound becomes as simple as::

    C:\Users\USERNAME\code\nsound> scons

-------------------------------------------------------------------------------
On Windows: Compiling Nsound C++ library Using Visual Studio
-------------------------------------------------------------------------------

Before trying to compile Nsound with Vistual Studio, ensure that Python and
SCons have been installed.  There are Visual Studio build steps that will execute
scons to generate some files.

Next, examine the file ``msvs\properties.props`` to ensure the path to Python
is correct, the default is ``C:\Python310``.

A Visual Studio 2010+ solution file is provide in ``msvs\nsound.sln``.  Debug
builds won't link with Python because the debug library ``python310_d.lib`` is
**not** provided, but isn't needed if you're debugging Nound C++ code.

.. note::

    If you see this error:

        LINK : fatal error LNK1181: cannot open input file 'winmm.lib'

    it may mean you are using "Visual Studio Express for Windows".  This error
    will go away if you install "Visual Studio Express for Windows Desktop".

-------------------------------------------------------------------------------
Cygwin Notes
-------------------------------------------------------------------------------

If you run into an error message that looks something like::

    python 6140 C:\cygwin\bin\python.exe: *** fatal error - unable to remap \\?\C:\cygwin\lib\python2.6\lib-dynload\time.dll to same address as parent: 0x2B0000 != 0x360000

You will need to "rebase" your Cygwin installation.  To do so follow these steps:

1) Close all Cygwin programs that may be using the cygwin.dll
2) Ensure the Cygwin package called rebase is installed using the setup.exe program
3) Start a Windows DOS box (not a Cygwin shell)
4) Start an ASH shell, then run the rebaseall command by executing::

    C:\> C:\cygwin\bin\ash.exe
    $ cd /bin
    $ ./rebaseall

===============================================================================
Compiling the Python Module
===============================================================================

.. note::

    SWIG must be installed to generate the Python interface.

1) Try compiling the C++ library first to ensure SCons and the C++ compiler are working (see step 2 above in `Compiling The C++ Library`).

2) Build, install and test the Python module::

    scons --pytest

-------------------------------------------------------------------------------
Mac OSX Notes - Use MacPorts (TODO: Update for python 3.7+)
-------------------------------------------------------------------------------

To enable real-time playback and plottting, install these prerequisites using
the mac port project::

    sudo port install swig scons py27-matplotlib portaudio swig-python py27-game py27-pip

Select the macport version of python and pip the defaults::

    sudo port select python python27
    sudo port select pip pip27

===============================================================================
Linking With A Python Installed In A Non-Standard Location
===============================================================================

Sometimes a custom built Python is installed in a non-standard location, for
example::

    /usr/local/python2.7

You will need to install scons to this non-stanard location, then just run
scons with this non-standard python::

    /usr/local/python2.7/bin/scons

The SCons build environment will ask Python's ``distutils`` module for the
location of the include directory and dynamic library paths.  This should get
done automatically.  A SCons tool was added to brute-force check the
``distutils`` build variables to locate these paths,
``site_scons/site_tools/ImportPythonConfig.py``.  To enable some extra
debug messages, try using the ``--config-debug`` switch with scons::

    scons --config-debug

Please report any problems you are having to Nick.

-------------------------------------------------------------------------------
Python Errors
-------------------------------------------------------------------------------

Sometime everything above goes well, SCons finds the libraries, compiles and
links everything fine, but at runtime you may run into other errors.

^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Matplotlib Backend Not Set
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

You may see a message like::

    Traceback (most recent call last):
        File "/usr/local/python2.7/lib/python2.7/site-packages/matplotlib/backends/__init__.py", line 35, in do_nothing_show
            fname = frame.f_back.f_code.co_filename
    AttributeError: 'NoneType' object has no attribute 'f_code'
    src/Nsound/Plotter.cc:171: failed to call python pylab.show()

This is due to the Matplotlib Backend not being set.  So set a default backend
edit the file `matplotlibrc`, and set the following::

    backend : BACKEND

and replace BACKEND with something like GTKAgg, GTKCairo, CocoaAgg, MacOSX,
QtAgg, WXAgg.  The choice depends on the backends that are installed.  To
test if a specific backend is installed do::

    >>> import matplotlib
    >>> matplotlib.use("GTKAgg")  # Specify specific backend
    >>> import matplotlib.pylab
    >>> pylab.plot([1,2,3,4,3,2,1])
    >>> pylab.show()  # Plot will appear if you selected a working GUI backend
    >>> pylab.savefig("somefile.png") # Replace png with pdf or svg to
    ...                               # to test a file only backend

You should either see a figure window show up after the `show()` call or a
filename written to the local directory after the `savefig()` call.

===============================================================================
Installing The Nsound Python Module Via PIP
===============================================================================

All the prerequisites still apply from above.  With pip you won't have to
download the Nsound source code yourself.

From a shell with your build environment setup, do::

    python3 -m pip install Nsound

