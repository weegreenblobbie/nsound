.. image:: https://github.com/weegreenblobbie/nsound/actions/workflows/linux.yaml/badge.svg?branch=master
   :target: https://github.com/weegreenblobbie/nsound/actions/workflows/linux.yaml?query=workflow:linux+branch:master
   :alt: Github Actions

.. image:: https://github.com/weegreenblobbie/nsound/actions/workflows/macos.yaml/badge.svg?branch=master
   :target: https://github.com/weegreenblobbie/nsound/actions/workflows/macos.yaml?query=workflow:macos+branch:master
   :alt: Github Actions

.. image:: https://github.com/weegreenblobbie/nsound/actions/workflows/windows.yaml/badge.svg?query=branch:master
   :target: https://github.com/weegreenblobbie/nsound/actions/workflows/windows.yaml?query=workflow:windows+branch:master
   :alt: Github Actions

Nsound, a C++ framework for synthesizing sound
==============================================

.. hyper link references

.. _`Nsound Homepage`: https://github.com/weegreenblobbie/nsound
.. _`Nsound User's Guide`: https://weegreenblobbie.com/nsound/user_guide/index.html
.. _`Nsound Doxygen Reference`: https://weegreenblobbie.com/nsound/doxygen/index.html

.. _`Introduction & Bebot`: https://www.youtube.com/watch?v=j4uLvl68PtI
.. _`bebot.py`: https://github.com/weegreenblobbie/nsound/blob/master/src/examples/bebot.py

.. _`Oscilloscope`: https://www.youtube.com/watch?v=ORnQVFpdSoA
.. _`make_oscilloscope_video.py`:  https://github.com/weegreenblobbie/nsound/blob/master/src/examples/make_oscilloscope_video.py

.. _`Moog VCF & Delayline`: https://www.youtube.com/watch?v=9m3KA6JBDH8
.. _`moogvcf-rt.py`: https://github.com/weegreenblobbie/nsound/blob/master/src/examples/moogvcf-rt.py
.. _`delayline-rt.py`: https://github.com/weegreenblobbie/nsound/blob/master/src/examples/delayline-rt.py

.. _`Tape Deck`: https://www.youtube.com/watch?v=Al6aHlwyEuA
.. _`tapedeck-rt.py`: https://github.com/weegreenblobbie/nsound/blob/master/src/examples/tapedeck-rt.py


Important Links
---------------

- `Nsound Homepage`_
- `Nsound User's Guide`_
- `Nsound Doxygen Reference`_

YouTube Videos & Source Code
----------------------------

=========================  ===================================
YouTube Video              Source Code
=========================  ===================================
`Introduction & Bebot`_    `bebot.py`_
`Oscilloscope`_            `make_oscilloscope_video.py`_
`Moog VCF & Delayline`_    `moogvcf-rt.py`_ , `delayline-rt.py`_
`Tape Deck`_               `tapedeck-rt.py`_
=========================  ===================================

Installing Python Module
------------------------

If you're on Windows, OS X, or Linux:

.. code-block:: bash

    python3 -m pip install Nsound

To build it from a source checkout, with Python 3.10 or newer and a C++20
compiler:

.. code-block:: bash

    python3 -m pip install .

``tools/test_pip_install.py`` checks that: it installs the checkout into a
new venv and runs the Python tests there.

macOS versions
~~~~~~~~~~~~~~

Building from source needs no macOS settings: the build targets the macOS it
is built on.

macOS wheel builds
~~~~~~~~~~~~~~~~~~

This is only for building wheels to publish on PyPI.

A wheel is tagged with the macOS it targets and doesn't install on an older
one.  To build a wheel for Macs older than the one building it, set
``MACOSX_DEPLOYMENT_TARGET`` to the oldest macOS the wheel supports:

.. code-block:: bash

    MACOSX_DEPLOYMENT_TARGET=11.0 python3 -m pip wheel --no-deps -w dist .

The target also sets how ``AudioPlaybackRt`` waits while its playback buffers
are full.  Targeting macOS 11 or newer it sleeps, targeting an older macOS it
keeps a CPU core busy, it spins.

For example, building on macOS 15:

============================  ================  ==============  =============
``MACOSX_DEPLOYMENT_TARGET``  Wheel's tag       Installs on     While waiting
============================  ================  ==============  =============
not set                       ``macosx_15_0``   15 or newer     Sleeps
``11.0``                      ``macosx_11_0``   11 or newer     Sleeps
``10.14``, Intel only         ``macosx_10_14``  10.14 or newer  Spins
============================  ================  ==============  =============

The tags end in the processor type, ``x86_64`` (Intel) or ``arm64`` (Apple
Silicon).  To support every Mac, publish three wheels, ``pip`` installs the
newest one the Mac supports:

- ``macosx_11_0_arm64`` for Apple Silicon.
- ``macosx_11_0_x86_64`` for Intel with macOS 11 or newer.
- ``macosx_10_14_x86_64`` for Intel with macOS 10.14 or 10.15.

Check the tag in each wheel's file name.


Building with Bazel
-------------------

The C++ library, programs, unit tests and the Python module can be built
with `Bazel <https://bazel.build>`_ on Linux, macOS and Windows.  Use
`Bazelisk <https://github.com/bazelbuild/bazelisk>`_, it reads the pinned
version from ``.bazelversion``.  Bazel fetches its own Python and its
packages, matplotlib and numpy.

On Windows, run ``.\setup-windows.ps1`` from a PowerShell window in the
nsound folder.  It installs Bazelisk as ``bazel`` and the Visual Studio C++
Build Tools, and turns on Developer Mode and long paths, which the Python
tests need for their symlinks and deep paths.  Then ``bazel build //...`` and ``bazel test //...`` build
with MSVC.  The compiler, sanitizer and coverage configs below are Linux only.

.. code-block:: bash

    bazel build //...                   # library and programs, gcc
    bazel test //...                    # C++ and Python unit tests
    bazel build --config=clang //...    # build with clang
    bazel test --config=asan //...      # AddressSanitizer, C++ tests only
    bazel test --config=ubsan //...     # UndefinedBehaviorSanitizer
    bazel build --config=debug //...    # -c dbg
    bazel build --config=opt //...      # -c opt
    tools/coverage.sh                   # coverage report in coverage_html/,
                                        # fails below the minimum

Main targets:

================================  ==========================================
Target                            Description
================================  ==========================================
``//src/Nsound:nsound``           The Nsound C++ library
``//src/python:Nsound``           The Python module, pybind11 bindings
``//src/bin:ns_playback``         Lists the audio host APIs, plays a
                                  test sweep through one, e.g.
                                  ``bazel run //src/bin:ns_playback --
                                  "Windows WASAPI"``
``//src/bin:ns_readwaveheader``   Prints a wave file header
``//src/bin:ns_vocoder``          Vocoder command line program
``//src/test:<Class>_test``       Catch2 unit tests, one per class
``//src/test:py_test_*``          Python unit tests, src/test/test_*.py
``//src/test:test_*``             Test programs, run by hand (tagged manual)
``//src/examples:*``              Examples, e.g.
                                  ``bazel run //src/examples:mynameis``,
                                  their .wav files are in src/examples
================================  ==========================================

Optional features are set with flags:

=================================  =======  ===================================
Flag                               Default  Description
=================================  =======  ===================================
``--//bazel:portaudio``            on       Audio playback with portaudio,
                                            which Bazel builds: ALSA on
                                            Linux, CoreAudio on macOS, WMME
                                            on Windows
``--//bazel:openmp``               on       OpenMP in Stretcher, -fopenmp
                                            (libgomp with gcc, libomp with
                                            clang, /openmp with MSVC).  Off
                                            on macOS, where Apple clang
                                            needs Homebrew's libomp
``--//bazel:c_pylab``              on       Plotter plots with matplotlib in
                                            Bazel's Python, which C++
                                            programs link and start.  Off,
                                            Plotter does nothing.  The
                                            Python module always plots
=================================  =======  ===================================

For example, ``bazel build --//bazel:portaudio=false //...``.

C++ programs plot with ``bazel run``, e.g.
``bazel run //src/examples:cepstral_pitch_estimate``, or run from
``bazel-bin``: the first ``Plotter`` starts Bazel's Python from the
program's runfiles.  A program copied away from its runfiles prints why it
can't plot and runs without plotting.  The C++ tests plot with matplotlib's
Agg backend, which opens no windows.

The CUDA sources (``*.cu``) are not built with Bazel.  ``Nsound.h`` is
generated from ``src/Nsound/Nsound.h.in`` with the version from
``version.bzl``.  Local options can go in ``user.bazelrc``.  The Python
packages are pinned in ``requirements_lock.txt``, made from
``requirements.in`` by the ``uv`` command at its top.

``tools/coverage.sh`` measures the line coverage of ``src/Nsound`` and the
``linux / bazel / coverage`` check fails if it drops below
``MINIMUM_PERCENT`` in ``tools/check_coverage.py``; raise it as tests are
added.  The playback tests play into a file on Linux: their ALSA
configuration, ``src/test/alsa_capture.conf``, makes the default device
write what it plays to a file, so no sound card is needed.  Bazel can't
collect C++ coverage with MSVC, so
on Windows ``tools\coverage.ps1`` only says where to run it instead.
