"""

$Id: test_Buffer.py 916 2015-08-22 16:31:39Z weegreenblobbie $

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

import cPickle as pickle
import os
import unittest

import numpy as np
import numpy.testing

import Nsound as ns
fn = ns.rel_to_abs

class Test(unittest.TestCase):

    GAMMA = 12

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
        "Basic buffer stuff"

        ns.Wavefile.setDefaultSampleSize(48)

        b1 = ns.Buffer(29)

        i = 0.0
        while  i < 100.0:
            b1 << i
            i += 1.1

        i = 0.0
        for x in b1:
            self.assertAlmostEqual(x, i, self.GAMMA)
            i += 1.1

    def test_02(self):
        "Buffer copy constructor"

        b1 = ns.Buffer()
        b1 << 1.1 << 2.2 << 3.3 << 4.4

        b2 = ns.Buffer(b1)

        try:
            self.assertEqual(b1, b2)
            self.assertEqual(b2, b1)

        except:
            self._plot_diff(b1, b2, "b1", "b2")
            raise

        b3 = ns.Buffer(b2)
        b4 = ns.Buffer(b3)

        b4 << b1
        b3 << b1

        try:
            self.assertEqual(b3, b4)
            self.assertEqual(b4, b3)

        except:
            self._plot_diff(b3, b4, "b1", "b2")
            raise

        size = b4.getLength()

        for i in xrange(size):

            try:
                self.assertAlmostEqual(b4[i], b3[i], self.GAMMA)

            except:
                self._plot_diff(b3, b4, "b3", "b4")
                raise

        self.assertEqual(1 + b4 * 2, 2 * b3 + 1)
        self.assertEqual(1 - b4 / 2, -1 * b3 / 2 + 1)
        self.assertEqual(2.0 / b4, 4.0 * (0.5 / b3))

        b3 = -1.0 * b4

        b4 += b3

        for i in xrange(b4.getLength()):
            self.assertAlmostEqual(b4[i], 0.0, self.GAMMA)

    def test_03(self):
        "Buffer::abs()"

        b = ns.Buffer()

        self.assertEqual(b.getLength(), 0)

        neg = -1.0
        sum_ = 1.0
        for i in xrange(100):
            sum_ *= neg
            b << sum_

        b.abs()

        self.assertGreaterEqual(b.getMin(), 0.0)

    def test_04(self):
        "Buffer::reverse()"

        b = ns.Buffer()
        b << 1.1 << 2.2 << 3.3 << 4.4
        b.reverse()

        gold = ns.Buffer()
        gold << 4.4 << 3.3 << 2.2 << 1.1

        for i in xrange(b.getLength()):
            self.assertAlmostEqual(gold[i], b[i], self.GAMMA)

    def test_05(self):
        "Buffer::subbuffer()"

        b = ns.Buffer()

        chunk = 100

        for i in xrange(chunk * 2 + 1):
            b << i

        test_size = 99

        subbuf = b.subbuffer(0, test_size)

        self.assertEqual(test_size, subbuf.getLength())

        subbuf = b[0:test_size]

        self.assertEqual(test_size, subbuf.getLength())

        test_size = chunk

        subbuf = b.subbuffer(0, test_size)

        self.assertEqual(test_size, subbuf.getLength())

        subbuf = b[0:test_size]

        self.assertEqual(test_size, subbuf.getLength())

        subbuf = b.subbuffer(0, test_size + 10)

        self.assertEqual(test_size + 10, subbuf.getLength())

        subbuf = b[0:test_size + 10]

        self.assertEqual(test_size + 10, subbuf.getLength())

        test_size = chunk + 10

        for i in xrange(10):

            subbuf = b.subbuffer(i, test_size)

            self.assertEqual(test_size, subbuf.getLength())

            subbuf = b[i:test_size + i]

            self.assertEqual(test_size, subbuf.getLength())

        subbuf = b.subbuffer(0,10)

        for i in xrange(10):
            self.assertEqual(i, int(subbuf[i]))

        subbuf = b[0:10]

        for i in xrange(10):
            self.assertEqual(i, int(subbuf[i]))

        subbuf = b.subbuffer(3,3)

        for i in xrange(3):
            self.assertEqual(i + 3, int(subbuf[i]))

        subbuf = b[3:6]

        for i in xrange(3):
            self.assertEqual(i + 3, int(subbuf[i]))

        subbuf = b.subbuffer(3)

        self.assertEqual(b.getLength() - 3, subbuf.getLength())

        subbuf = b.subbuffer(0)

        self.assertEqual(b.getLength(), subbuf.getLength())

        for i in xrange(10):
            self.assertEqual(i, int(subbuf[i]))

        subbuf = b.subbuffer(5,5)

        self.assertEqual(5, subbuf.getLength())

        subbuf = b[5:10]

        self.assertEqual(5, subbuf.getLength())

        for i in xrange(5):
            self.assertEqual(i + 5, int(subbuf[i]))

    def test_06(self):
        "Buffer operators with different lengths"

        sine = ns.Sine(100)

        b1 = ns.Buffer(100)
        b1 = sine.drawLine(1.0, 2.0, 2.0)

        b2 = ns.Buffer(100)
        b2 = sine.drawLine(2.0, 3.0, 3.0)

        result = b1 * b2

        self.assertEqual(b1.getLength(), result.getLength())

        for x in result:
            self.assertAlmostEqual(6.0, x, self.GAMMA)

        result = b2 * b1

        self.assertEqual(b2.getLength(), result.getLength())

        for i in xrange(b1.getLength()):
            self.assertAlmostEqual(6.0, result[i], self.GAMMA)

        for i in xrange(b1.getLength(), result.getLength()):
            self.assertAlmostEqual(3.0, result[i], self.GAMMA)

    def test_07(self):
        "Buffer advanced operator"

        sine = ns.Sine(100)
        b = ns.Buffer(100)
        b << sine.generate(1.0, 2.0)

        data = ns.Buffer(b)
        data(data > 0.5).set(0.5);
        data(data < -0.5).set(-0.5);

        gold_fn = fn("gold/Buffer_out1.wav")

        gold = ns.Buffer(gold_fn)

        diff = gold - data;

        try:
            self.assertLess(diff.getAbs().getMax(), self.GAMMA)
        except:
            self._plot_diff(gold, data, "gold", "data")
            raise

        data = ns.Buffer(b)

        bs1 = data(data > 0.5)
        bs2 = data(data < -0.5)

        bs1 *= 0.1
        bs2 *= 0.1

        bs1 += 0.45
        bs2 -= 0.45

        gold_fn = fn("gold/Buffer_out2.wav")

        gold = ns.Buffer(gold_fn)

        diff = gold - data;

        try:
            self.assertLess(diff.getAbs().getMax(), self.GAMMA)
        except:
            self._plot_diff(gold, data, "gold", "data")
            raise

    def test_08(self):
        "Buffer advanced slicing"

        b1 = ns.Buffer([1,2,3,4,5])

        b2 = b1[1:]

        self.assertEqual(b2, ns.Buffer([2,3,4,5]))

        b2 = b1[:1]

        self.assertEqual(b2, ns.Buffer([1.0]))

        b2 = b1[1:2]

        self.assertEqual(b2, ns.Buffer([2.0]))

        b2 = b1[2:1]

        self.assertEqual(b2, ns.Buffer([]))

        b2 = b1[-1:]

        self.assertEqual(b2, ns.Buffer([5.0]))

        b2 = b1[-1:-2]

        self.assertEqual(b2, ns.Buffer([]))

        b2 = b1[-2:-1]

        self.assertEqual(b2, ns.Buffer([4]))


    def test_09(self):
        "Buffer pickling"

        gold = [1.1, 2.2, 3.3, 4.4, 5.5]

        b1 = ns.Buffer(gold)

        fn = 'buffer.pkl'

        with open(fn, 'w') as fd:
            pickle.dump(b1, fd)

        with open(fn, 'r') as fd:
            obj = pickle.load(fd)

        data = obj.toList()

        np.testing.assert_almost_equal(gold, data)

        if os.path.isfile(fn):
            os.remove(fn)


    def test_10(self):
        "Buffer::circular_iterator forward"

        b1 = ns.Buffer([1,2,3])

        c = b1.cbegin()

        b2 = ns.Buffer()

        for i in xrange(10):
            b2 << float(c)
            c += 1

        gold = [1,2,3,1,2,3,1,2,3,1]

        np.testing.assert_almost_equal(gold, b2)


    def test_10(self):
        "Buffer::circular_iterator backward"

        b1 = ns.Buffer([1,2,3])

        c = b1.cbegin()

        b2 = ns.Buffer()

        for i in xrange(10):
            c -= 1

            b2 << float(c)

        gold = [3,2,1,3,2,1,3,2,1,3]

        np.testing.assert_almost_equal(gold, b2)


