"""
$Id: test_Wavefile.py 831 2014-04-27 20:28:44Z weegreenblobbie $

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

import unittest

import Nsound as ns
fn = ns.rel_to_abs

class Test(unittest.TestCase):

    GAMMA = 1.5e-14

    def test_1(self):
        "Test 1"

        fn_gold = fn("gold/Wavefile_out1.wav")
        fn_data = fn("test_wavefile1.wav")

        sr = 100
        n_channels = 3

        ns.Wavefile.setDefaultSampleSize(48)

        sin = ns.Sine(sr)

        data = ns.AudioStream(sr, n_channels)

        data << sin.generate(1.0, 13.0)

        data[1] *= 0.666666666666666
        data[2] *= 0.333333333333333

        # Write data to disk, read it back in.

        data >> fn_data

#~        # Create gold file
#~        data >> fn_gold

        data = ns.AudioStream(fn_data)
        gold = ns.AudioStream(fn_gold)

        diff = data - gold

        abs_diff = diff.getAbs()

        if abs_diff.getMax() > self.GAMMA:

            abs_diff.plot("Absolute diff (data - gold)")

            ns.Plotter.show()

            raise ValueError("Broken!")

    def test_2(self):
        "Test 2"

        fn_gold_in  = fn("gold/Wavefile_out1.wav")
        fn_gold_out = fn("gold/Wavefile_out2.wav")
        fn_data     = fn("test_wavefile2.wav")

        data = ns.AudioStream(fn_gold_in)

        ns.Wavefile.setDefaultSampleSize(24)

        data >> fn_data

        data = ns.AudioStream(fn_data)

#~        # Create gold file
#~        data >> fn_gold_out

        gold = ns.AudioStream(fn_gold_out)

        diff = data - gold

        abs_diff = diff.getAbs()

        if abs_diff.getMax() > self.GAMMA:

            abs_diff.plot("Absolute diff (data - gold)")
            ns.Plotter.show()

            raise ValueError("Broken!")
