*******************************************************************************
Building and installing Nsound
*******************************************************************************

===============================================================================
The Python Module
===============================================================================

The Python module needs Python 3.10 or newer and a C++20 compiler: gcc or
clang on Linux, Xcode's clang on macOS, or the Visual Studio C++ Build Tools
on Windows.  pip compiles the C++ library and the module's pybind11 bindings
together.

From PyPI::

    python3 -m pip install Nsound

From a source checkout, in the nsound folder::

    python3 -m pip install .

Plotting needs matplotlib, Nsound works without it::

    python3 -m pip install matplotlib

To run the Python unit tests from the nsound folder, with numpy installed::

    python3 -m unittest discover -s src/test -t .

Or check the whole install: this installs the checkout into a new venv and
runs the Python tests there::

    python3 tools/test_pip_install.py

===============================================================================
The C++ Library With Bazel
===============================================================================

The C++ library, programs, examples and unit tests build with
`Bazel <https://bazel.build>`_ on Linux, macOS and Windows.  Use
`Bazelisk <https://github.com/bazelbuild/bazelisk>`_ as ``bazel``, it fetches
the Bazel version pinned in ``.bazelversion``.

On Windows, run ``.\setup-windows.ps1`` from a PowerShell window in the
nsound folder, it installs Bazelisk and the Visual Studio C++ Build Tools,
and turns on Developer Mode and long paths, which the Python tests need for
their symlinks and deep paths.

Then, in the nsound folder::

    bazel build //...       # the library, programs and examples
    bazel test //...        # the C++ and Python unit tests

Bazel builds the Python module too, ``//src/python:Nsound``, with its own
Python.  README.rst lists the compiler, sanitizer and coverage configs and
the optional features.

===============================================================================
Python Errors
===============================================================================

Sometimes Nsound builds and installs fine, but you run into errors at runtime.


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

