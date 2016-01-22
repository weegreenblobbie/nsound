******************
Nsound Instruments
******************

Instruments are classes that provide a common API, their purpose is to provide
a foundation of common musical elements for everyone to use.

All Nsound Instruments will provide the following functions:

.. py:function:: Nsound.Instrument.play()

Returns:

    *demo* : Nsound.AudioStream
        Returns a demo created by the author of the instrument

.. py:function:: Nsound.Instrument.play(duration, frequency)

Parameters:

    *duration* : float
        The duration in seconds of sound to generate

    *frequency* : float
        The frequency of the sound in Hz

Returns:

    *out* : Nsound.AudioStream
        Returns the sound of the instrument at the given ``frequency`` for
        ``duration`` seconds

Bass Guitar
-----------

Based on a physical model written in Csound by Hans Mikelson.

.. py:function:: Nsound.GuitarBass(sample_rate)

*sample_rate* : float
    The number of samples per seconds

.. only:: html

    .. pyexec::

        import os
        import datetime
        import Nsound as ns

        sr = 44100.0

        a = ns.AudioStream(sr, 1)

        a << ns.GuitarBass(sr).play() # Plays the demo.

        name = "GuitarBass"

        wavfile = "source/_static/%s.wav" % name
        mp3file = "source/_static/%s.mp3" % name

        a >> wavfile

        # Encode as mp3
        if not os.path.isfile(mp3file):
            os.system(
                "lame -q 2 -b 192 "
                "--tt '%s' --ta Nsound --tl '%s' --ty %d --tc %s %s %s" %(
                name,
                ns.__version__,
                datetime.datetime.now().year,
                "http://nsound.sourceforge.net",
                wavfile,
                mp3file))

        template = "Sample here: :download:`%s.mp3. <_static/%s.mp3>`" %(
            name,
            name)

Clarinet
--------

Based on a physical model written in Csound by Hans Mikelson which was
originally based on Perry Cook's physical model.

.. py:function:: Nsound.Clarinet(sample_rate)

*sample_rate* : float
    The number of samples per seconds

.. only:: html

    .. pyexec::

        import os
        import datetime
        import Nsound as ns

        sr = 44100.0

        a = ns.AudioStream(sr, 1)

        a << ns.Clarinet(sr).play() # Plays the demo.

        name = "Clarinet"

        wavfile = "source/_static/%s.wav" % name
        mp3file = "source/_static/%s.mp3" % name

        a >> wavfile

        # Encode as mp3
        if not os.path.isfile(mp3file):
            os.system(
                "lame -q 2 -b 192 "
                "--tt '%s' --ta Nsound --tl '%s' --ty %d --tc %s %s %s" %(
                name,
                ns.__version__,
                datetime.datetime.now().year,
                "http://nsound.sourceforge.net",
                wavfile,
                mp3file))

        template = "Sample here: :download:`%s.mp3. <_static/%s.mp3>`" %(
            name,
            name)

Kick Bass Drum
--------------

.. py:function:: Nsound.DrumKickBass(sample_rate, high_frequency, low_frequency)

*sample_rate* : float
    The number of samples per seconds

*high_frequency* : float
    The starting frequency of the drum

*low_frequency* : float
    The stopping frequency of the drum as its response decays

.. only:: html

    .. pyexec::

        import datetime
        import os
        import Nsound as ns

        sr = 44100.0

        a = ns.AudioStream(sr, 1)

        a << ns.DrumKickBass(sr, 400.0, 0.0).play() # Plays the demo.

        # Firefox plugin won't play short clip, so add some silence
        g = ns.Generator(sr)
        a << g.silence(1.0)

        name = "DrumKickBass"

        wavfile = "source/_static/%s.wav" % name
        mp3file = "source/_static/%s.mp3" % name

        a >> wavfile

        # Encode as mp3
        if not os.path.isfile(mp3file):
            os.system(
                "lame -q 2 -b 192 "
                "--tt '%s' --ta Nsound --tl '%s' --ty %d --tc %s %s %s" %(
                name,
                ns.__version__,
                datetime.datetime.now().year,
                "http://nsound.sourceforge.net",
                wavfile,
                mp3file))

        template = "Sample here: :download:`%s.mp3. <_static/%s.mp3>`" %(
            name,
            name)

BD01 Drum
---------

Simulates a bass drum.  Based on a Csound drum.

source: http://www.csounds.com/istvan/html/drums.html

.. py:function:: Nsound.DrumBD01(sample_rate)

*sample_rate* : float
    The number of samples per seconds

.. only:: html

    .. pyexec::

        import os
        import datetime
        import Nsound as ns

        sr = 44100.0

        a = ns.AudioStream(sr, 1)

        a << ns.DrumBD01(sr).play() # Plays the demo.

        # Firefox plugin won't play short clip, so add some silence
        g = ns.Generator(sr)
        a << g.silence(1.0)

        name = "DrumBD01"

        wavfile = "source/_static/%s.wav" % name
        mp3file = "source/_static/%s.mp3" % name

        a >> wavfile

        # Encode as mp3
        if not os.path.isfile(mp3file):
            os.system(
                "lame -q 2 -b 192 "
                "--tt '%s' --ta Nsound --tl '%s' --ty %d --tc %s %s %s" %(
                name,
                ns.__version__,
                datetime.datetime.now().year,
                "http://nsound.sourceforge.net",
                wavfile,
                mp3file))

        template = "Sample here: :download:`%s.mp3. <_static/%s.mp3>`" %(
            name,
            name)

Hat
---

Simulates a Hat hit, based on a Csound Hat instrument by Steven Cook.

.. py:function:: Nsound.Hat(sample_rate)

*sample_rate* : float
    The number of samples per seconds

.. only:: html

    .. pyexec::

        import os
        import datetime
        import Nsound as ns

        sr = 44100.0

        a = ns.AudioStream(sr, 1)

        a << ns.Hat(sr).play() # Plays the demo.

        name = "Hat"

        wavfile = "source/_static/%s.wav" % name
        mp3file = "source/_static/%s.mp3" % name

        a >> wavfile

        # Encode as mp3
        if not os.path.isfile(mp3file):
            os.system(
                "lame -q 2 -b 192 "
                "--tt '%s' --ta Nsound --tl '%s' --ty %d --tc %s %s %s" %(
                name,
                ns.__version__,
                datetime.datetime.now().year,
                "http://nsound.sourceforge.net",
                wavfile,
                mp3file))

        template = "Sample here: :download:`%s.mp3. <_static/%s.mp3>`" %(
            name,
            name)

Pipe Organ
----------

Based on a Csound Pipe Organ by Hons Mikelson.

.. py:function:: Nsound.OrganPipe(sample_rate)

*sample_rate* : float
    The number of samples per seconds

.. only:: html

    .. pyexec::

        import os
        import datetime
        import Nsound as ns

        version = ns.__version__

        sr = 44100.0

        a = ns.AudioStream(sr, 1)

        # Plays the demo.
        a << ns.OrganPipe(sr).play()

        name = "OrganPipe"

        wavfile = "source/_static/%s.wav" % name
        mp3file = "source/_static/%s.mp3" % name

        a >> wavfile

        # Encode as mp3
        if not os.path.isfile(mp3file):
            os.system(
                "lame -q 2 -b 192 "
                "--tt '%s' --ta Nsound --tl '%s' --ty %d --tc %s %s %s" %(
                name,
                ns.__version__,
                datetime.datetime.now().year,
                "http://nsound.sourceforge.net",
                wavfile,
                mp3file))

        template = "Sample here: :download:`%s.mp3. <_static/%s.mp3>`" %(
            name,
            name)

Slide Flute
-----------

Based on a physical model written in Csound by Hans Mikelson which was
originally based on Perry Cook's physical model.

.. py:function:: Nsound.FluteSlide(sample_rate)

*sample_rate* : float
    The number of samples per seconds

.. only:: html

    .. pyexec::

        import os
        import datetime
        import Nsound as ns

        sr = 44100.0

        a = ns.AudioStream(sr, 1)

        a << ns.FluteSlide(sr).play() # Plays the demo.

        name = "FluteSlide"

        wavfile = "source/_static/%s.wav" % name
        mp3file = "source/_static/%s.mp3" % name

        a >> wavfile

        # Encode as mp3
        if not os.path.isfile(mp3file):
            os.system(
                "lame -q 2 -b 192 "
                "--tt '%s' --ta Nsound --tl '%s' --ty %d --tc %s %s %s" %(
                name,
                ns.__version__,
                datetime.datetime.now().year,
                "http://nsound.sourceforge.net",
                wavfile,
                mp3file))

        template = "Sample here: :download:`%s.mp3. <_static/%s.mp3>`" %(
            name,
            name)
