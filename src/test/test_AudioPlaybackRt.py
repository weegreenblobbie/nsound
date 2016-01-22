"""
$Id: test_AudioPlaybackRt.py 899 2015-06-07 18:27:06Z weegreenblobbie $

Copyright (c) 2009 to Present Nick Hilton

weegreenblobbie_yahoo_com (replace '_' with '@' and '.')

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

import time
import unittest
import sys

import Nsound as ns
fn = ns.rel_to_abs

skip = False

d = dir(ns)

if 'NSOUND_CPP11' not in d or 'NSOUND_LIBPORTAUDIO' not in d:
    skip = True


class Test(unittest.TestCase):

    @unittest.skipIf(skip, "AudioPlaybackRt not compiled in!")
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

        for i in xrange(N / 2):
            pb.play(0)

        info = pb.get_debug_info()

        self.assertEqual(0, info.rd_ptr)
        self.assertEqual(0, info.wr_ptr)
        self.assertEqual(N/2, info.wr_index)
        self.assertEqual(0, info.n_ready)
        self.assertEqual(False, info.is_streaming)

        # write 2/2 buffer size to pb

        for i in xrange(N / 2):
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

        for i in xrange(N - 2):
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

        for i in xrange(N):
            pb.play(0)

        time.sleep(0.2)

        info = pb.get_debug_info()

        self.assertEqual(0, info.rd_ptr)
        self.assertEqual(0, info.wr_ptr)
        self.assertEqual(1, info.wr_index)
        self.assertEqual(3, info.n_ready)
        self.assertEqual(False, info.is_streaming)

        # playing starts

        for i in xrange(N):
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

    @unittest.skipIf(skip, "AudioPlaybackRt not compiled in!")
    def test_2(self):
        "Test 2"

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

        # Play some samples

        for i in xrange( 10 * N):
            pb.play(0)

        # Note: not sleepping, trying to capture n_ready from 10 to 0

        info = pb.get_debug_info()

        self.assertEqual(False, info.is_streaming)
        self.assertEqual(10, info.n_ready)

        x = set()

        for i in xrange(1 << 10):
            pb.play(0)
            info = pb.get_debug_info()
            if info.is_streaming: break

        self.assertEqual(True, info.is_streaming)

        while info.n_ready != 0:
            self.assertEqual(True, info.is_streaming)
            info = pb.get_debug_info()
            x.add(info.n_ready)

        gold = set(range(11))

        # print "gold = ", gold
        # print "x    = ", x

        self.assertEqual(gold, x)

    @unittest.skipIf(skip, "AudioPlaybackRt not compiled in!")
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
