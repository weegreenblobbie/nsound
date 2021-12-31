***************
What is Nsound?
***************

Nsound is a C++ library and Python module for audio synthesis featuring
dynamic digital filters. Nsound lets you easily shape waveforms and write
to disk or plot them.

In Nsound, all audio data is represented by 64-bit floating point numbers
between -1.0 and 1.0.  It is easy to scale the data and adjust the volume by
multiplying the audio data by a percentage, a number between 0.0 and 1.0.  The
audio data is is only converted to 8-bit, 16-bit, 24-bit, 32-bit or 64-bit when
it is written to the disk with the Wavefile class.

The core of Nsound is the Buffer class.  Memory is dynamically allocated and
frees the user from worrying about buffer sizes.  All the mathematical
operators are overloaded to allow intuitive expressions.

Generators produce oscillations of the waveform stored in them.  Generators
can produce a single frequency of the waveform held in them, or can dynamicly
change frequency, just draw a line that represents frequency and pass it into
the generator.

A collection of IIR & FIR filters are also included.  When creating filters,
the cutoff frequency is specified in Hz.  The filters are also real-time,
meaning they keep track of their internal state and for each audio sample
passed in, an audio sample is returned.

Beautiful plots can be easily created if your platform has the Python
Matplotlib package installed.  Most of the Nsound classes have their
own plot methods.

This User's Guide will specify code snippets written in Python.  The C++
code is nearly identical.  For specific C++ reference, view the Doxygen
generated documentation available from
`here <https://weegreenblobbie.com/nsound/doxygen/index.html>`_.
