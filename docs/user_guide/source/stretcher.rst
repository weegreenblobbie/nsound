****************
Nsound Stretcher
****************

Nsound implements pitch and time shifting using the Stretcher class.  The
algorithm is based on the paper "An Overlap-Add Technique Based On Wavefrom
Similarity (WSOLA) For High Quality Time-Scale Modification Of Speech" by
Werner Verhelst and Marc Roelands.

With the Stretcher class, you can change the time of the singal without
modifying the frequency content.  Or, you can also scale the
frequency content without modifying the time.

Please visit the Nsound
`examples <http://nsound.sourceforge.net/examples/index.html>`_ webpage to
listen to a 10 seconds stereo recording that was dynamically time and pitch
shifted.

Some of the basic documentation:

.. py:function:: Stretcher(sample_rate)

.. py:function:: Stretcher.pitchShift(x, factor)

    `x`
        The input Buffer or AudioStream.

    `factor`
        A percent change factor.

.. py:function:: Stretcher.timeShift(x, factor)

    `x`
        The input Buffer or AudioStream.

    `factor`
        A percent change factor.

The example below creates a signal and shifts it in time, but preserves the
frequency content.  The results are put into the frequency domain and plotted.

.. plot::
    :include-source:

    import Nsound as ns

    sample_rate = 1000.0

    stretch = ns.Stretcher(sample_rate)
    stretch.showProgress(True)

    saw = ns.Sawtooth(sample_rate, 4)

    fft = ns.FFTransform(sample_rate)
    fft.setWindow(ns.HANNING)

    a = ns.AudioStream(sample_rate, 1)
    a << saw.generate(1.0, 100.0)
    a.plot("Original Time Domain")
    fft.fft(a[0], 1024)[0].plot("Original Frequency Domain")

    # Time shifts

    a2 = stretch.timeShift(a, 0.50)
    a2.plot("50% Time Shift")

    a3 = stretch.timeShift(a, 2.0)
    a3.plot("200% Time Shift")

    # Frequency Scaling

    a4 = stretch.pitchShift(a, 0.50)
    fft.fft(a4[0], 1024)[0].plot("50% Pitch shift")

    a5 = stretch.pitchShift(a, 2.00)
    fft.fft(a5[0], 1024)[0].plot("200% Pitch Shift")

