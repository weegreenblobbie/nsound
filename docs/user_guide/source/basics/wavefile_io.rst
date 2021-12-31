***********
Wavefile IO
***********

Nsound includes a simple Wavefile class that can read RIFF wavefiles that are
PCM encoded.  This is a very basic wavefile format.  Nsound currently does not
support any other audio formats, but there are plenty of other free tools on
the web that can convert audio formats to PCM wavefiles.

Reading Wavefiles
=================

As seen in previous sections, a wavefile can be read by passing its filename
to the Buffer or AudioStream constructor::

    import Nsound as ns

    b = ns.Buffer("california.wav")
    a = ns.AudioStream("california.wav")

Nsound has also overloaded the left shift operator, concatenation from
wavefiles is easy::

    import Nsound as ns

    b = ns.Buffer()
    b << "california.wav" \
      << "walle.wav"

Writing Wavefiles
=================

.. Warning::

	Values outside the range of -1.0 and 1.0 will get clipped in the output
	wavefile.  To avoid this, call the function normalize() on the Buffer or
	AudioStream.

The Buffer and AudioStream class includes a function to write wavefiles to
disk.

.. py:function:: Buffer.writeWavefile(filename)
.. py:function:: AudioStream.writeWavefile(filename)

The Wavefile class is used by Buffer and AudioStream to perform the actual
writing.  Note: the Buffer class `does not know` the sample rate of the data
contained in it.  The Wavefile class uses a default sample rate setting.
The Wavefile default sample rate and sample size can be changed by calling:

.. py:function:: Wavefile.setDefaultSampleRate(sample_rate)
.. py:function:: Wavefile.setDefaultSampleSize(sample_size)

    Where `sample_size` is 8, 16, 24, 32 or 64.

When an AudioStream writes to disk, the sample rate is known by the AudioStream.
Like the Buffer class, the sample size `is not known`.  Use the
Wavefile.setDefaultSampleSize() to set this option::

    import Nsound as ns

    a = ns.AudioStream("california.wav")

    ns.Wavefile.setDefaultSampleSize(32)

    a.writeWavefile("california-32bit.wav")

Nsound has also overloaded the right shift operator for writing wavefiles, the
example above can be rewritten as::

    import Nsound as ns

    a = ns.AudioStream("california.wav")

    ns.Wavefile.setDefaultSampleSize(32)

    a >> "california-32bit.wav"

Nsound will write wavefiles using integer data types by default.  This can be
changed to floating point.

Floating Point Wavefiles
========================

.. warning::

    Few programs know how to read the IEEE floating point format.  Audacity
    1.3.12 is able to read `mono` floating point formats, but does not seem
    to be able to read multi channel files.

.. note::

	Since the data is stored as floating point, there is no need to normalize
	the data first.

Nsound can read and write wavefiles that store their samples in IEEE floating
point format.  To write files using this format, call:

.. py:function:: Wavefile.setIEEEFloat(flag)

And set the flag to True.  The Wavefile sample size must be set to 32 or 64.

Resampling Wavefiles
====================

Sometimes a wavefile won't be at the sample rate that we wish it were.  For
example, you downloaded a sample sound but it is not at the sample rate of
your project.

With Nsound it's easy to resample the file to the sample rate you need::

    import Nsound as ns

    a = ns.AudioStream("wav_at_16KHz.wav")

    a.resample2(48000.0)

    a >> "wav_at_48KHz.wav"

The Python script below can be used on the command line to change a wavefile's
sample rate::

    #! /usr/bin/env python

    from argparse import ArgumentParser

    import Nsound as ns

    parser = ArgumentParser(
        usage = "resample target_sample_rate input.wav output.wav")
    parser.add_argument("target_sample_rate", type=int)
    parser.add_argument("input_wave", type=str)
    parser.add_argument("output_wave", type=str)

    args = parser.parse_args()

    target = float(args.target_sample_rate)
    f1 = args.input_wave
    f2 = args.output_wave

    print("Reading %s" % f1)

    a1 = ns.AudioStream(f1)
    source = a1.getSampleRate()

    print("source: %d" % source)
    print("target: %d" % target)

    ratio = target / source

    print("ratio:  %f" % ratio)

    print("Resampling ...")

    a2 = a1.getResample(ratio)
    a2.setSampleRate(int(target))

    print("Writing %s" % f2)

    a2 >> f2
