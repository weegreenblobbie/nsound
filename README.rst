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


Building with Bazel
-------------------

The C++ library, programs and unit tests can be built with
`Bazel <https://bazel.build>`_ on Linux, macOS and Windows.  Use
`Bazelisk <https://github.com/bazelbuild/bazelisk>`_, it reads the pinned
version from ``.bazelversion``.  The SCons build still works as before.

On Windows, run ``.\setup-windows.ps1`` from a PowerShell window in the
nsound folder.  It installs Bazelisk as ``bazel`` and the Visual Studio C++
Build Tools, then ``bazel build //...`` and ``bazel test //...`` build with
MSVC.  The compiler, sanitizer and coverage configs below are Linux only.

.. code-block:: bash

    bazel build //...                   # library and programs, gcc
    bazel test //...                    # C++ unit tests
    bazel build --config=clang //...    # build with clang
    bazel test --config=asan //...      # AddressSanitizer
    bazel test --config=ubsan //...     # UndefinedBehaviorSanitizer
    bazel build --config=debug //...    # -c dbg
    bazel build --config=opt //...      # -c opt
    tools/coverage.sh                   # coverage report in coverage_html/

Main targets:

================================  ==========================================
Target                            Description
================================  ==========================================
``//src/Nsound:nsound``           The Nsound C++ library
``//src/bin:ns_readwaveheader``   Prints a wave file header
``//src/bin:ns_vocoder``          Vocoder command line program
``//src/test:unit_tests``         The C++ unit tests
``//src/test:test_*``             Test programs, run by hand (tagged manual)
``//src/examples:*``              Examples, e.g.
                                  ``bazel run //src/examples:mynameis``,
                                  their .wav files are in src/examples
================================  ==========================================

Optional features are set with flags:

=================================  =======  ===================================
Flag                               Default  Description
=================================  =======  ===================================
``--//bazel:libao``                off      libao audio backend, links -lao
``--//bazel:portaudio``            off      portaudio audio backend, links
                                            -lportaudio
``--//bazel:openmp``               on       OpenMP in Stretcher, -fopenmp
                                            (libgomp with gcc, libomp with
                                            clang, /openmp with MSVC).  Off
                                            on macOS, where Apple clang
                                            needs Homebrew's libomp
``--//bazel:c_pylab``              off      Plotter uses the matplotlib C API,
                                            does not build yet: Bazel has no
                                            target for the numpy headers
=================================  =======  ===================================

For example, ``bazel build --//bazel:libao --//bazel:portaudio //...``.

The CUDA sources (``*.cu``) are not built with Bazel.  ``Nsound.h`` is
generated from ``src/Nsound/Nsound.h.in`` with the version from
``version.bzl``.  Local options can go in ``user.bazelrc``.
