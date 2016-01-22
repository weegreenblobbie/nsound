*********************
Nsound Audio Playback
*********************

As of Nsound-0.8.1, playback through the soundcard is now supported on
platforms that have libao or libportaudio.

.. image:: /images/audioplayback_interface.*
    :height: 278px
    :alt: Nsound AudioPlayback Interface
    :align: center

Audio playback can be as simple as::

    import Nsound as ns
    a = ns.AudioStream("california.wav")
    a >> ns.AudioPlayback(a.getSamplerate(), a.getNChannels(), 16)

The :class:`Nsound.AudioPlayback` constructor:

.. py:function:: Nsound.AudioPlayback(sample_rate, n_channels, bits_per_sample)

*sample_rate* : float
    The number of samples per seconds

*n_channels* : int
    The number of channels, currently on 1 or 2 is supported

*bits_per_sample* : int
    The sample size to convert to before playing it on the sound card, currently
    only 16 and 32 are supported

Not all combinations of ``sample_rate``, ``n_channels`` and ``bits_per_sample``
will work.  Most platforms should work with

- ``sample_rate = 44100``
- ``n_channels = 1``, ``n_channels = 2``
- ``bits_per_sample = 16``, ``bits_per_sample = 32``

Other combinations have not been tested.

Audio Backends
--------------

The audio backend to use can be selected.  By default, the audio backend is
automatically seleceted based on the backends that were available at compile
time in the following order:

1. PortAudio
#. AO
#. None

Following the Matplotlib convention, a call to :class:`Nsound.use()` can select
the audio backend to use.

.. py:function:: Nsound.use(backend)

*backend* : str
    The backend to use; currently recognized backends:

        1. "portaudio" or "libportaudio"
        #. "ao" or "libao"

The backend must be selected before initializing the AudioPlayback class::

    ns.use("portaudio")

    pb = ns.AudioPlayback(44100.0, 2, 16)

    buffer_or_audio_stream >> pb

See ``src/examples/example1.py`` and ``src/examples/example4.py`` for
AudioPlayback example usage.

Limitations
-----------

The :class:`Nsound.AudioPlayback` class is **not** thread safe, so only one thread
can use the class at a time.  Only blocking calls are currently implemented,
with memory being allocated and floating point numbers being casted to integer
types on a call by call basis.


Know Problems
-------------

If you are on Linux and you are using libportaudio, you may see a message
like::

    bt_audio_service_open: connect() failed: Connection refused (111)
    bt_audio_service_open: connect() failed: Connection refused (111)

Audio will still be played to the sound card (at least on Nick's Ubuntu
10.04 box.)  The error happens because Bluetooth is disabled but ALSA is still
configured to use Bluetooth services, to fix this, remove the ``bluez-alsa``
package::

    $ sudo apt-get purge bluez-alsa

The error messsage should now go away, but be careful with this solution if
you use Bluetooth devices for audio.
