*******************************************************************************
Nsound Real-Time Audio Playback
*******************************************************************************

-------------------------------------------------------------------------------
Real-Time Playback Status
-------------------------------------------------------------------------------

Current real-time playback status (as of |today|):

+------------+------------------------------+----------------------+--------------------------+
| Platform   | Software Path                | Lo Latency w/ JACK   | Without JACK             |
+============+==============================+======================+==========================+
| Linux      | nsound->portaudio->jack      | YES, works great!    | YES (via ALSA)           |
+------------+------------------------------+----------------------+--------------------------+
| Mac OSX    | nsound->portaudio->jack      | NO `1`_              | YES (via coreaudio) `2`_ |
+------------+------------------------------+----------------------+--------------------------+
| Windows    | nsound->portaudio->jack      | NO `3`_              | YES (via winmm) `4`_     |
+------------+------------------------------+----------------------+--------------------------+
| Cygwin     | nsound->portaudio->???       | `N/A` `5`_           | `N/A` `5`_               |
+------------+------------------------------+----------------------+--------------------------+

.. _1:

| **Note 1**:
| Mac port's ``jackd`` and ``qjackctl`` don't seem to work on my OSX 10.7.5 system.

.. code-block:: console

    $ /opt/local/bin/jackd --version
    jackd version 0.121.3 tmpdir /tmp/ protocol 24

    $ /opt/local/bin/qjackctl --version
    Qt: 4.6.3
    QjackCtl: 0.3.6

Starting the JACK server produces this error, which I don't know how to solve::

    19:43:21.392 Patchbay deactivated.
    19:43:21.426 Statistics reset.
    19:43:39.616 JACK is starting...
    19:43:39.617 /opt/local/bin/jackd -v -t2000 -dcoreaudio -dAppleHDAEngineOutput:1B,0,1,1:0 \
        -r48000 -p128 -o1
    19:43:39.630 JACK was started with PID=1814.
    jackd 0.121.3
    Copyright 2001-2009 Paul Davis, Stephane Letz, Jack O'Quinn, Torben Hohn and others.
    jackd comes with ABSOLUTELY NO WARRANTY
    This is free software, and you are welcome to redistribute it
    under certain conditions; see the file COPYING for details
    getting driver descriptor from /opt/local/lib/jack/jack_coreaudio.so
    getting driver descriptor from /opt/local/lib/jack/jack_dummy.so
    getting driver descriptor from /opt/local/lib/jack/jack_net.so
    JACK compiled with POSIX SHM support.
    server 'default' registered
    loading driver ..
    registered builtin port type 32 bit float mono audio
    registered builtin port type 8 bit raw midi
    clock source = system clock via gettimeofday
    start poll on 3 fd's
    new client: coreaudio, id = 1 type 1 @ 0x100114420 fd = -1
    new buffer size 128
    resizing port buffer segment for type 0, one buffer = 512 bytes
    resizing port buffer segment for type 1, one buffer = 512 bytes
    registered port system:playback_1, offset = 0
    ++ jack_sort_graph
    ++ jack_rechain_graph():
    -- jack_rechain_graph()
    -- jack_sort_graph
    1814 waiting for signals
    load = 0.0563 max usecs: 3.000, spare = 2663.000
    19:43:41.666 Could not connect to JACK server as client. - Overall operation failed. \
        Please check the messages window for more info.
    load = 0.1219 max usecs: 5.000, spare = 2661.000
    load = 0.1360 max usecs: 4.000, spare = 2662.000
    load = 0.1430 max usecs: 4.000, spare = 2662.000
    19:43:44.656 JACK is stopping...
    jack main caught signal 15
    poll failed (Bad file descriptor)
    starting server engine shutdown
    stopping driver
    unloading driver
    freeing shared port segments
    stopping server thread
    last xrun delay: 0.000 usecs
    max delay reported by backend: 0.000 usecs
    freeing engine shared memory
    max usecs: 5.000, engine deleted
    cleaning up shared memory
    cleaning up files
    unregistering server 'default'
    19:43:44.713 JACK was stopped successfully.

If anybody has experience with this please conact me.

.. _2:

| **Note 2**:
| OSX without JACK works reasonably well, OSX has great audio drivers built-in.

.. _3:

| **Note 3**:
| On Windows, I wanted to use the portaudio->jack interface to connect to the
 Windows JACK server.  I failed to build a static portaudio with JACK and/or
 ASIO.  If anybody has experience with this please contact me.

.. _4:

| **Note 4**:
| Windows witout JACK works reasonbly well, adjusting the ``sample_rate`` and
 ``buffer_size_sec`` for the ``AudioPlaybackRt`` class should produce
  glitch-free performance, at the expense of large latencies.

.. _5:

| **Note 5**:
| Cygwin is completly untested in this release.


-------------------------------------------------------------------------------
Linux Real-Time Audio Setup
-------------------------------------------------------------------------------

These instructions are based on Ubuntu Desktop 14.04 64-bit.

^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Install JACK Audio Connection Kit (JACK)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

References:
    - http://www.youtube.com/watch?v=fMz6fDGBnA4

1) On the command line, install ``jackd`` and ``qjackctl``::

    sudo apt-get install jackd qjackctl

.. Note:: IMPORTANT! The installation script will prompt you to enable real-time priorites, please select ``<YES>``.

2) Confirm the the install process created this file: ``/etc/security/limits.d/audio.conf``:

.. code-block:: console

    $ cat /etc/security/limits.d/audio.conf
    # Provided by the jackd package.
    #
    # Changes to this file will be preserved.
    #
    # If you want to enable/disable realtime permissions, run
    #
    #    dpkg-reconfigure -p high jackd

    @audio   -  rtprio     95
    @audio   -  memlock    unlimited
    #@audio   -  nice      -19

3) Confirm you are part of the ``audio`` group by using the ``id`` command:

.. code-block:: console

    $ id
    uid=1000(nhilton) gid=1000(nhilton) groups=1000(nhilton),4(adm),24(cdrom),27(sudo),29(audio),30(dip),46(plugdev),108(lpadmin),123(pulse-access),124(sambashare)

4) Reboot your computer.

5) Configure ``jackd`` with ``qjackctl``, here's an example from my system:

.. image:: /images/jack.png

-------------------------------------------------------------------------------
Real-Time Demos
-------------------------------------------------------------------------------

These demos require ``python-pygame`` to be installed, ``pygame`` is used as
the event system.

^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Bebot
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Bebot is based off the really cool "Bebot" iPhone app.  When executed, a GUI
window will pop up.  If you have JACK working, use the ``--jack`` flag to
connect to the JACK server.

.. code-block:: console

    $ cd src/examples
    $ python bebot.py
    $ python bebot.py --jack   # if you have JACK working

    Hello Bebot!
    Press ESC to quit.

.. image:: /images/bebot.png

Pressing the left mouse button down starts audio playback, releasing the mouse
stops audio playback.  While audio is playing, you can change the sound by
moving the mouse around.

.. only:: html

    .. pyexec::

        import os
        import datetime
        import Nsound as ns

        name = "bebot-rt"

        wavfile = "source/_static/%s.wav" % name
        mp3file = "source/_static/%s.mp3" % name

        # Encode as mp3
        if not os.path.isfile(mp3file):
            os.system(
                "lame -q 2 -b 192 "
                "--tt '%s' --ta Nsound --tl '%s' --ty %d --tc %s %s %s" %(
                name,
                ns.__version__,
                datetime.datetime.now().year,
                "https://github.com/weegreenblobbie/nsound",
                wavfile,
                mp3file))

        template = "Sample here: :download:`%s.mp3. <_static/%s.mp3>`" %(
            name,
            name)

^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Piano
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

A simple piano with the following options:

.. pyexec::

    import subprocess

    cmd = ["python", "../../src/examples/piano.py", "--help"]

    output = subprocess.check_output(cmd).decode("ascii").split('\n')

    s = output[0] + "\n"
    for x in output[1:]:
        s += "        %s\n" % x

    template = """.. code-block:: console

        $ cd src/examples
        $ python piano.py --help
        {output}
    """.format(output = s)

If you have JACK working, use the ``--jack`` flag to connect to the JACK server.
Try running with these options:

.. code-block:: console

    $ python piano.py --saw --chorus 2
    $ python piano.py --saw --chorus 2 --jack    # if you have JACK working

    Hello Piano!
    Press ESC to quit.
    Press [z,s,x,d,c,v,g,b,h,n,j,m] to play a note.
    ++-+--++--++-+--++--+-+-+-+-

.. only:: html

    .. pyexec::

        import os
        import datetime
        import Nsound as ns

        name = "piano-rt"

        wavfile = "source/_static/%s.wav" % name
        mp3file = "source/_static/%s.mp3" % name

        # Encode as mp3
        if not os.path.isfile(mp3file):
            os.system(
                "lame -q 2 -b 192 "
                "--tt '%s' --ta Nsound --tl '%s' --ty %d --tc %s %s %s" %(
                name,
                ns.__version__,
                datetime.datetime.now().year,
                "https://github.com/weegreenblobbie/nsound",
                wavfile,
                mp3file))

        template = "Sample here: :download:`%s.mp3. <_static/%s.mp3>`" %(
            name,
            name)
