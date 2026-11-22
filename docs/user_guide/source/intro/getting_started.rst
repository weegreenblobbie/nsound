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


Using The C++ Library And Bazel
===============================

Nsound builds with Bazel (https://bazel.build), see
:doc:`build_and_installing`.  The BUILD.bazel files say what each folder
builds, ``//...`` is everything::

    bazel build //...

Suppose you are modifying one of the examples, say src/examples/example2.cc.
Build just that example by naming its target, from anywhere in the nsound
folder::

    bazel build //src/examples:example2

Or build it and run it, the examples read and write their .wav files in
src/examples::

    bazel run //src/examples:example2

Bazel rebuilds only what changed.  Programs using the library depend on
``//src/Nsound:nsound`` in their BUILD.bazel file.
