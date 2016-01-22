********************
AudioStream Creation
********************

An AudioStream is a container that holds Buffers, it also stores a sample rate,
providing information about time.  It is meant to easily manipulate multiple
channels of audio at the same time.  Most Nsound functions that operate on
Buffers also operate on AudioStreams.

There are 3 general ways to create an AudioStream:

1) Creating an empty AudioStream
2) Call ones(), rand() or zeros()
3) Reading a wavefile from disk

Creating An Empty AudioStream
=============================

Call the constructor::

    import Nsound as ns
    a = ns.AudioStream()

The default constructor sets the sample rate to 44100.0 and a single channel.
The new AudioStream `a` is empty.  Calling the getLength() method will return 0.
Calling the getDuration() method will return 0.0.

To specify the sample rate::

    a = ns.AudioStream(44100.0)

This will create a new AudioStream with a sample rate of 44.1 kHz and a single
channel.  To create a stereo AudioStream::

    a = ns.AudioStream(44100.0, 2)

The is no limit to the number of channels an AudioStream can have.  In practice,
the number of channels and their duration will be limited to the amount of
memory your computer has.

The underlying data structure that is held by the AudioStream class is a
std::vector of Buffer objects.  One can preallocate memory when creating an
AudioStream by specify the number of samples to preallocate::

    a = ns.AudioStream(44100.0, 2, 1024)

The new AudioStream `a` is empty, even though memory was preallocated.  Calling
the getLength() method will return 0, calling getDuration() will return 0.0.

In general, you don't need to worry about preallocating memory.  It is meant
to be useful when implementing new features in Nsound when the size of
AudioStreams are already known.

Call Ones, Rand or Zeros
=========================

The Buffer class includes some convience functions for creating Buffers that
are filled with oness, random numbers or zeros:

.. py:function:: AudioStream.ones(sample_rate, n_channels, n_samples)
.. py:function:: AudioStream.rand(sample_rate, n_channels, n_samples)
.. py:function:: AudioStream.zeros(sample_rate, n_channels, n_samples)

Example usage::

    import Nsound as ns
    a1 = ns.AudioStream.ones(44100.0, 2, 1.0)
    a2 = ns.AudioStream.rand(44100.0, 2, 1.0)
    a3 = ns.AudioStream.zeros(44100.0, 2, 1.0)

In the example above, 44100 samples (1 second) were stored in 2 channels.

Reading A Wavefile From Disk
============================

An AudioStream can be created from a wavefile::

    a = ns.AudioStream("california.wav")

The new AudioStream `a` will contain all the samples in "california.wav".  If
the wavefile has more than one channel, `all channels` are read and stored
in the new AudioStream.  The wavefile's sample rate is also read and stored in
the AudioStream.

The wavefile's data will be converted into float64 with a range of
(-1.0, 1.0.).

