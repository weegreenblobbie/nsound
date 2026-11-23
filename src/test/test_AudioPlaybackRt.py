"""
$Id: test_AudioPlaybackRt.py 899 2015-06-07 18:27:06Z weegreenblobbie $

Copyright (c) 2009 to Present Nick Hilton

weegreenblobbie2_gmail_com (replace '_' with '@' and '.')

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Library General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

"""

import array
import os
import tempfile
import time
import unittest
import sys

import Nsound as ns
fn = ns.rel_to_abs

skip = False
skip_reason = ""

d = dir(ns)

if 'NSOUND_LIBPORTAUDIO' not in d:
    skip = True
    skip_reason = "AudioPlaybackRt not compiled in!"

elif not sys.platform.startswith("linux"):
    skip = True
    skip_reason = "Playback is captured through ALSA, on Linux only"

else:
    # portaudio plays through ALSA: alsa_capture.conf makes its default device
    # write to a file, there may be no sound card.
    os.environ["ALSA_CONFIG_PATH"] = fn("alsa_capture.conf")
    os.environ["NSOUND_ALSA_CAPTURE"] = os.path.join(
        os.environ.get("TEST_TMPDIR", tempfile.gettempdir()), "capture.raw")


class Test(unittest.TestCase):

    @unittest.skipIf(skip, skip_reason)
    def test_1(self):
        "Test 1"

        SR = 44100.0
        T = 0.10
        N = int(round(T * SR, 1))

        pb = ns.AudioPlaybackRt(SR, 1, 3, T)

        # init values

        info = pb.get_debug_info()

        self.assertEqual(3, info.pool_size)
        self.assertEqual(N, info.samples_per_buffer)
        self.assertEqual(SR, info.samplerate)

        self.assertEqual(0, info.rd_ptr)
        self.assertEqual(0, info.wr_ptr)
        self.assertEqual(0, info.wr_index)
        self.assertEqual(0, info.n_ready)
        self.assertEqual(False, info.is_streaming)

        # write 1/2 buffer size to pb
        for i in range(N // 2):
            pb.play(0)

        info = pb.get_debug_info()

        self.assertEqual(0, info.rd_ptr)
        self.assertEqual(0, info.wr_ptr)
        self.assertEqual(N/2, info.wr_index)
        self.assertEqual(0, info.n_ready)
        self.assertEqual(False, info.is_streaming)

        # write 2/2 buffer size to pb

        for i in range(N // 2):
            pb.play(0)

        time.sleep(0.2)

        info = pb.get_debug_info()

        self.assertEqual(0, info.rd_ptr)
        self.assertEqual(1, info.wr_ptr)
        self.assertEqual(0, info.wr_index)
        self.assertEqual(1, info.n_ready)
        self.assertEqual(False, info.is_streaming)

        # write 1 sample
        pb.play(0)

        time.sleep(0.2)

        info = pb.get_debug_info()

        self.assertEqual(0, info.rd_ptr)
        self.assertEqual(1, info.wr_ptr)
        self.assertEqual(1, info.wr_index)
        self.assertEqual(1, info.n_ready)
        self.assertEqual(False, info.is_streaming)

        for i in range(N - 2):
            pb.play(0)

        time.sleep(0.2)

        info = pb.get_debug_info()

        self.assertEqual(0, info.rd_ptr)
        self.assertEqual(1, info.wr_ptr)
        self.assertEqual(N - 1, info.wr_index)
        self.assertEqual(1, info.n_ready)
        self.assertEqual(False, info.is_streaming)

        # write 1 sample
        pb.play(0)

        time.sleep(0.2)

        info = pb.get_debug_info()

        self.assertEqual(0, info.rd_ptr)
        self.assertEqual(2, info.wr_ptr)
        self.assertEqual(0, info.wr_index)
        self.assertEqual(2, info.n_ready)
        self.assertEqual(False, info.is_streaming)

        # write 1 sample
        pb.play(0)

        time.sleep(0.2)

        info = pb.get_debug_info()

        self.assertEqual(0, info.rd_ptr)
        self.assertEqual(2, info.wr_ptr)
        self.assertEqual(1, info.wr_index)
        self.assertEqual(2, info.n_ready)
        self.assertEqual(False, info.is_streaming)

        # The last sample fills the 3rd buffer, they all wait to be played:
        # the next sample starts playing and waits for a buffer to be played
        # before it is written.  The capture device plays them all at once.

        for i in range(N):
            pb.play(0)

        time.sleep(0.2)

        info = pb.get_debug_info()

        self.assertEqual(0, info.rd_ptr)
        self.assertEqual(0, info.wr_ptr)
        self.assertEqual(1, info.wr_index)
        self.assertEqual(0, info.n_ready)
        self.assertEqual(True, info.is_streaming)

        # it keeps playing

        for i in range(N):
            pb.play(0)

        time.sleep(0.4)

        info = pb.get_debug_info()

        self.assertEqual(1, info.rd_ptr)
        self.assertEqual(1, info.wr_ptr)
        self.assertEqual(1, info.wr_index)
        self.assertEqual(0, info.n_ready)
        self.assertEqual(True, info.is_streaming)

        time.sleep(0.2)

        info = pb.get_debug_info()

        self.assertEqual(1, info.rd_ptr)
        self.assertEqual(1, info.wr_ptr)
        self.assertEqual(1, info.wr_index)
        self.assertEqual(0, info.n_ready)
        self.assertEqual(True, info.is_streaming)

        pb.stop()

        info = pb.get_debug_info()

        self.assertEqual(0, info.rd_ptr)
        self.assertEqual(0, info.wr_ptr)
        self.assertEqual(0, info.wr_index)
        self.assertEqual(0, info.n_ready)
        self.assertEqual(False, info.is_streaming)

    @unittest.skipIf(skip, skip_reason)
    def test_2(self):
        "Plays every sample written, in order"

        SR = 8000
        T = 0.01
        N = int(round(T * SR, 1))

        # A capture file of its own, ALSA opens it with the stream.
        capture = os.environ["NSOUND_ALSA_CAPTURE"] + ".test_2"
        os.environ["NSOUND_ALSA_CAPTURE"] = capture

        pb = ns.AudioPlaybackRt(SR, 1, 10, T)

        # 10 buffers of distinct samples.
        samples = [((i % 200) - 100) / 128.0 for i in range(10 * N)]

        for x in samples:
            pb.play(x)

        info = pb.get_debug_info()

        self.assertEqual(False, info.is_streaming)
        self.assertEqual(10, info.n_ready)

        # Streaming starts with more samples, silence here.
        for i in range(1 << 10):
            pb.play(0)
            info = pb.get_debug_info()
            if info.is_streaming: break

        self.assertEqual(True, info.is_streaming)

        for i in range(100):
            if pb.get_debug_info().n_ready == 0: break
            time.sleep(0.01)

        pb.stop()

        played = array.array("h")

        with open(capture, "rb") as fd:
            played.frombytes(fd.read())

        # 16 bit samples, truncated like int().
        gold = [int(32767 * x) for x in samples]

        # portaudio starts the stream with silence, the samples follow.
        start = next(i for i, x in enumerate(played) if x != 0)

        self.assertEqual(gold, list(played[start:start + len(gold)]), "silence %d" % start)

    @unittest.skipIf(skip, skip_reason)
    def test_3(self):
        "Test 3"

        SR = 44100
        T = 0.01
        N = int(round(T * SR, 1))

        pb = ns.AudioPlaybackRt(SR, 1, 10, T)

        info = pb.get_debug_info()

        self.assertEqual(10, info.pool_size)
        self.assertEqual(N, info.samples_per_buffer)

        self.assertEqual(0, info.rd_ptr)
        self.assertEqual(0, info.wr_ptr)
        self.assertEqual(0, info.wr_index)
        self.assertEqual(0, info.n_ready)
        self.assertEqual(False, info.is_streaming)

        gen = ns.Sine(SR)

        dur = 1 * T

        buf = gen.silence(dur)

        pb.play(buf)

        time.sleep(dur)

        #info = pb.get_debug_info()
        #print "info = ", info
