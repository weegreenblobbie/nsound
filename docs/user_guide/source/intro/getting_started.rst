******************************
Getting Started Using Nsound
******************************

Using The Nsound Python Module
==============================

After installing the Nsound Python module, you can start playing with the
examples.  Download the Nsound source code or check out the git tag::

    $ git clone https://github.com/weegreenblobbie/nsound.git

Next, modify one of the examples in src/examples, then execute the example::

    $ python example1.py

and listen to the wavefile written out or look at the plots created.

You can also use the module in Python interactively::

    >>> import Nsound as ns
    >>> b = ns.Buffer()
    >>> b << 0 << 1 << 0.9 << 0.8 << 0.7 << 0.6 << 0.5 << 0.4 << 0.3 << 0.2 << 0.1 << 0
    >>> # Other magical lines ...
    >>> b.plot("Nsound")
    >>> ns.Plotter.show()

.. plot::

    import Nsound as ns

    N = ns.Buffer()
    s = ns.Buffer()
    o = ns.Buffer()
    u = ns.Buffer()
    n = ns.Buffer()
    d = ns.Buffer()

    spacer = ns.Buffer()

    spacer << 0 << 0 << 0 << 0 << 0 << 0 << 0 << 0 << 0 << 0

    N << 0 << 1 << 0.9 << 0.8 << 0.7 << 0.6 << 0.5 << 0.4 << 0.3 << 0.2 << 0.1 << 0
    N << 1.0 << 0.0

    s << 0.05 << 0.10 << 0.15 << 0.20 << 0.25 << 0.30 << 0.35 << 0.40 << 0.45 << 0.50 << 0.0

    o << 0.5 << 0.5 << 0.5 << 0.5 << 0.5 << 0.5 << 0.5 << 0.5 << 0.5 << 0.5 << 0.5

    u << 0.5 << 0 << 0 << 0 << 0 << 0 << 0 << 0 << 0 << 0 << 0.5

    n << 0.5 << 0.45 << 0.40 << 0.35 << 0.30 << 0.25 << 0.20 << 0.15 << 0.10 << 0.05 << 0.0 << 0.5 << 0.0

    d << 0.5 << 0.5 << 0.5 << 0.5 << 0.5 << 0.5 << 0.5 << 0.5 << 0.5 << 1.0 << 0.0

    b = ns.Buffer()

    b << spacer
    b << N << spacer
    b << s << spacer
    b << o << spacer
    b << u << spacer
    b << n << spacer
    b << d << spacer

    b.plot("Nsound")
    ns.Plotter().ylim(-0.5, 1.5)


Using The C++ Library And SCons
===============================

Nsound uses the SCons build system (http://www.scons.org).  SCons is written in
Python and the "Makefiles" are Python scripts.

SCons looks for a special file called SConstruct that serves as the master
"Makefile" that tells SCons how to build everything.  Nsound's SConstruct file
is located in the root of the archive.

.. pyexec::

    from Nsound import PACKAGE_RELEASE
    template = """
    The Nsound C++ library and examples are compiled by default when you invoke
    scons without any arguments::

        cd %s
        scons
    """ % PACKAGE_RELEASE

Sopose you are modifying one of the examples and wish to recompile it, you could
issue scons in the root and it will recompile everything that needs to be
rebuilt.  Now suppose you want work in the src/example directory.  Running
SCons in this directory results in the following error message::

    scons: *** No SConstruct file found.
    File "/usr/lib/scons/SCons/Script/Main.py", line 858, in _main

This is because Nsound only contains one SConstruct file, and it only lives in
the root of the archive.  This is a common scenario and so the folks that make
SCons include a command line switch to tell it to look else where for the
SConstruct file.  So now try this::

    scons -u

This will tell scons to search up the directory structure to search for the
SConstruct file.

You can also tell SCons to build a specific target.  Suppose you are in the
examples directory and modified example2.cc.  You may have changes in other
example files but you only want to compile example2.cc.  You can specify the
target like so::

    scons -u example2

Or if you are on Windows::

    scons -u example2.exe
