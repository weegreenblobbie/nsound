"""

$Id: test_AudioStream.py 927 2015-08-23 18:46:27Z weegreenblobbie $

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

import pickle
import os
import unittest

import numpy as np
import numpy.testing

import Nsound as ns
fn = ns.rel_to_abs

class Test(unittest.TestCase):

    GAMMA = 12

    ns.Wavefile.setDefaultSampleSize(48);

    def _plot_diff(self, b1, b2, title1, title2):

        p = ns.Plotter()

        ax1 = p.subplot(2, 1, 1)

        p.plot(b1, "b-+", 'label="%s"' % title1)
        p.plot(b2, "r-+", 'label="%s"' % title2)

        p.legend()

        ax2 = p.subplot(2, 1, 2, "", ax1, ax1)

        diff = b1 - b2
        diff.abs()
        p.plot(diff, "r-*", 'label="abs difference"')

        p.legend()

        ns.Plotter.show()

    def test_01(self):
        "AudioStream::operator<<(float64)"

        a1 = ns.AudioStream(100, 2)

        i = 0.0
        while  i < 100.0:
            a1 << i
            i += 1.1

        for buf in a1:
            i = 0.0
            for x in buf:
                self.assertAlmostEqual(x, i, self.GAMMA)
                i += 1.1

    def test_02(self):
        "AudioStream copy constructor"

        a1 = ns.AudioStream(100, 1)
        a1 << 1.1 << 2.2 << 3.3 << 4.4

        a2 = ns.AudioStream(a1)

        try:
            self.assertEqual(a1, a2)
            self.assertEqual(a2, a1)
        except:
            self._plot_diff(a1[0], a2[0], "a1", "a2")
            raise

    def test_03(self):
        "AudioStream::operator<<(Buffer)"

        a1 = ns.AudioStream(100, 1)
        b1 = ns.Buffer()

        for i in range(10):
            a1 << i
            b1 << i

        a2 = ns.AudioStream(a1)
        a3 = ns.AudioStream(a2)

        a2 << b1
        a3 << b1

        try:
            self.assertEqual(a2, a3)
        except:
            self._plot_diff(a2[0], a3[0], "a2", "a3")
            raise

    def test_04(self):
        "AudioStream::operator<<(const AudioStream &) ..."

        a1 = ns.AudioStream(100, 1)

        for i in range(10):
            a1 << i

        a2 = ns.AudioStream(a1)
        a3 = ns.AudioStream(a2)

        a2 << a1
        a3 << a1

        try:
            self.assertEqual(a2, a3)
        except:
            self._plot_diff(a2[0], a3[0], "a2", "a3")
            raise

    def test_05(self):
        "AudioStream::operator[](uint32) ..."

        a1 = ns.AudioStream(100, 1)
        a2 = ns.AudioStream(100, 1)

        for i in range(10):
            a1 << i
            a2 << i

        size = a1[0].getLength()

        try:
            for i in range(size):
                self.assertAlmostEqual(a1[0][i], a2[0][i], self.GAMMA)

        except:
            self._plot_diff(a1[0], a2[0], "a1", "a2")
            raise

    def test_06(self):
        "::operator+,-,*,/ ..."

        a1 = ns.AudioStream(100, 1)
        a2 = ns.AudioStream(100, 1)

        for i in range(10):
            a1 << i + 1
            a2 << i + 1

        self.assertEqual(1 + a1 * 2, 2 * a2 + 1)
        self.assertEqual(1 - a1 / 2, -1 * a2 / 2 + 1)

        t1 = 2.0 / a1
        t2 = 4.0 * (0.5 / a2)

        for i in range(t1[0].getLength()):
            self.assertAlmostEqual(t1[0][i], t2[0][i], self.GAMMA)

        a1 = -1.0 * a2
        a2 += a1

        for buf in a2:
            for x in buf:
                self.assertAlmostEqual(x, 0.0, self.GAMMA)

    def test_07(self):
        "AudioStream::abs() ..."

        a1 = ns.AudioStream(100, 2)

        self.assertEqual(0.0, a1.getDuration())

        neg = -1.0
        sum_ = 1.0
        for i in range(100):
            sum_ *= neg
            a1 << sum_

        a1.abs()

        self.assertGreaterEqual(a1.getMin(), 0.0)

    def test_08(self):
        "AudioStream::reverse() ..."

        a1 = ns.AudioStream(100, 2)

        for f in range(100):
            a1 << f

        a1.reverse()

        for buf in a1:
            index = 0;
            for f in range(99, -1, -1):
                self.assertAlmostEqual(float(f), buf[index], self.GAMMA)
                index += 1

    def test_09(self):
        "AudioStream::substream() ..."

        a1 = ns.AudioStream(100, 2)

        chunk = 100

        for i in range(chunk * 2 + 1):
            a1 << i

        test_size = 99 / 100.0

        substream = a1.substream(0, test_size)

        self.assertAlmostEqual(test_size, substream.getDuration(), self.GAMMA/2)

        test_size = 1.0

        substream = a1.substream(0, test_size)

        self.assertAlmostEqual(test_size, substream.getDuration(), self.GAMMA)

        test_size = a1.getDuration() + 1.0

        substream = a1.substream(0, test_size)

        self.assertAlmostEqual(a1.getDuration(), substream.getDuration(), self.GAMMA)

        test_size = (chunk + 10) / 100.0

        for i in range(10):

            substream = a1.substream(i/100.0, test_size)

            self.assertAlmostEqual(test_size, substream.getDuration())

        substream = a1.substream(0, 0.10)

        for i in range(10):
            self.assertAlmostEqual(i, substream[0][i], self.GAMMA)

        substream = a1.substream(0.031, 0.031)

        for i in range(3):
            self.assertAlmostEqual(i + 3, substream[0][i])

        substream = a1.substream(3.1/100.0)

        self.assertEqual(a1[0].getLength() - 3, substream[0].getLength())

        substream = a1.substream(0)

        self.assertEqual(a1[0].getLength(), substream[0].getLength())

        for i in range(10):
            self.assertAlmostEqual(i, substream[0][i], self.GAMMA)

        substream = a1.substream(5.1/100.0,5.1/100.0)

        self.assertEqual(5, substream[0].getLength())

    def test_10(self):
        "AudioStream operators with different lengths ..."

        sine = ns.Sine(100)

        a1 = ns.AudioStream(100, 2)
        a1 << sine.drawLine(1.0, 2.0, 2.0)

        a2 = ns.AudioStream(100, 2)
        a2 << sine.drawLine(2.0, 3.0, 3.0)

        result = a1 * a2

        self.assertNotEqual(a2[0].getLength(), result[0].getLength())

        for x in result[0]:
            self.assertAlmostEqual(6.0, x, self.GAMMA)

        result = a2 * a1

        self.assertEqual(a2[0].getLength(), result[0].getLength())

        for i in range(a1[0].getLength()):
            self.assertAlmostEqual(6.0, result[0][i])

        for i in range(a1[0].getLength(), result[0].getLength()):
            self.assertAlmostEqual(3.0, result[0][i], self.GAMMA)

    def test_11(self):
        "AudioStream slices ..."

        print("\n")

        sine = ns.Sine(100)

        a1 = ns.AudioStream(100, 3)
        a1 << sine.drawLine(1.0, 1.0, 1.0)

        a1[0] += 1.0
        a1[2] += 1.0

        for b in a1[0]:
            self.assertAlmostEqual(2.0, b, self.GAMMA)

        for b in a1[1]:
            self.assertAlmostEqual(1.0, b, self.GAMMA)

        for b in a1[2]:
            self.assertAlmostEqual(2.0, b, self.GAMMA)

        selection = a1.select(0, 2) # selects channels 0,1
        selection -= 1.0

        for b in a1[0]:
            self.assertAlmostEqual(1.0, b, self.GAMMA)

        for b in a1[1]:
            self.assertAlmostEqual(0.0, b, self.GAMMA)

        for b in a1[2]:
            self.assertAlmostEqual(2.0, b, self.GAMMA)


    def test_12(self):
        "AudioStream pickling"

        gold = [1.1, 2.2, 3.3, 4.4, 5.5]

        as1 = ns.AudioStream(2, 2)

        as1 << gold

        fn = 'audio_stream.pkl'

        with open(fn, 'wb') as fd:
            pickle.dump(as1, fd)

        with open(fn, 'rb') as fd:
            obj = pickle.load(fd)

        self.assertEqual(2, int(obj.getSampleRate()))
        self.assertEqual(2, obj.getNChannels())

        for i in range(2):
            data = obj[i].toList()
            np.testing.assert_almost_equal(gold, data)

        if os.path.isfile(fn):
            os.remove(fn)


    def test_13(self):
        "AudioStream buffer assignment and transpose"

        b = ns.Buffer()
        b << 1 << 2 << 3 << 4

        a = ns.AudioStream(1, 2)

        a[0] = b
        a[1] = b.getReverse()

        self.assertEqual(2, a.getNChannels())
        self.assertEqual(4, a.getLength())

        self.assertAlmostEqual(a[0], ns.Buffer([1,2,3,4]), self.GAMMA)
        self.assertAlmostEqual(a[1], ns.Buffer([4,3,2,1]), self.GAMMA)

        a.transpose()

        self.assertEqual(4, a.getNChannels())
        self.assertEqual(2, a.getLength())

        self.assertAlmostEqual(a[0], ns.Buffer([1,4]), self.GAMMA)
        self.assertAlmostEqual(a[1], ns.Buffer([2,3]), self.GAMMA)
        self.assertAlmostEqual(a[2], ns.Buffer([3,2]), self.GAMMA)
        self.assertAlmostEqual(a[3], ns.Buffer([4,1]), self.GAMMA)
