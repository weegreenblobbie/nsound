"""
$Id: test_filters.py 846 2014-05-27 21:56:19Z weegreenblobbie $

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

    @classmethod
    def setUpClass(cls):

        cls.SR = 8000.0
        cls.GAMMA = 5

        ns.Wavefile.setDefaultSampleSize(32)
        ns.Wavefile.setIEEEFloat(True)
        ns.Wavefile.setDefaultSampleRate(cls.SR)

        sin = ns.Sine(cls.SR)

        freqs_sweep = sin.drawLine(5.0, 0.0, 1720.0)

        cls._input = sin.generate(5.0, freqs_sweep)

#~        cls._input >> fn("filters_input.wav")

        cls._freqs_lo = 200 * sin.generate(5.0, 1.0) + freqs_sweep
        cls._freqs_hi = 200 + cls._freqs_lo

    @classmethod
    def tearDownClass(cls):
    	ns.Wavefile.setIEEEFloat(False)
        ns.Plotter.show()

    def _plot_diff(self, b1, b2, title1, title2):

        p = ns.Plotter()
        p.figure()

        ax1 = p.subplot(2, 1, 1)

        p.plot(b1, "r", 'label="%s"' % title1)
        p.plot(b2, "b", 'label="%s"' % title2)

        p.legend()
        p.grid(True)

        ax2 = p.subplot(2, 1, 2, "", ax1, ax1)

        diff = b1 - b2
        diff.abs()
        p.plot(diff, "r", 'label="abs difference"')
        p.grid(True)
        p.legend()

    def _compare_buffers(self, gold_fn, data, save_gold = False):

        if save_gold:
            print "SAVING GOLD FILE: %s" % gold_fn
            data >> gold_fn

        gold = ns.Buffer(gold_fn)

        diff = gold - data
        diff.abs()

        try:
            self.assertAlmostEqual(0.0, diff.getMax(), self.GAMMA)
        except:
            self._plot_diff(gold, data, "gold", "data")
            raise

    def test_01(self):
        "FilterLowPassFIR"

        gold_fn = fn("gold/filter_fir_low_pass.wav")

        f1 = ns.FilterLowPassFIR(self.SR, 256, 880.0)

        data = f1.filter(self._input)

        self._compare_buffers(gold_fn, data)

        gold_fn = fn("gold/filter_fir_low_pass_dynamic.wav")

        data = f1.filter(self._input, self._freqs_lo)

        self._compare_buffers(gold_fn, data)

    def test_02(self):
        "FilterLowPassIIR"

        gold_fn = fn("gold/filter_iir_low_pass.wav")

        f2 = ns.FilterLowPassIIR(self.SR, 4, 880.0, 0.01)

        data = f2.filter(self._input)

        self._compare_buffers(gold_fn, data)

        gold_fn = fn("gold/filter_iir_low_pass_dynamic.wav")

        data = f2.filter(self._input, self._freqs_lo)

        self._compare_buffers(gold_fn, data)

    def test_03(self):
        "FilterHighPassFIR"

        gold_fn = fn("gold/filter_fir_high_pass.wav")

        f3 = ns.FilterHighPassFIR(self.SR, 256, 880.0)

        data = f3.filter(self._input)

        self._compare_buffers(gold_fn, data)

        gold_fn = fn("gold/filter_fir_high_pass_dynamic.wav")

        data = f3.filter(self._input, self._freqs_hi)

        self._compare_buffers(gold_fn, data)

    def test_04(self):
        "FilterHighPassIIR ... "

        gold_fn = fn("gold/filter_iir_high_pass.wav")

        f4 = ns.FilterHighPassIIR(self.SR, 4, 880.0, 0.01)

        data = f4.filter(self._input)

        self._compare_buffers(gold_fn, data)

        gold_fn = fn("gold/filter_iir_high_pass_dynamic.wav")

        data = f4.filter(self._input, self._freqs_hi)

        self._compare_buffers(gold_fn, data)

    def test_05(self):
        "FilterBandRejectFIR"

        gold_fn = fn("gold/filter_fir_band_reject.wav")

        f5 = ns.FilterBandRejectFIR(self.SR, 512, 450.0, 870.0)

        data = f5.filter(self._input)

        self._compare_buffers(gold_fn, data)

        gold_fn = fn("gold/filter_fir_band_reject_dynamic.wav")

        data = f5.filter(self._input, self._freqs_lo, self._freqs_hi)

        self._compare_buffers(gold_fn, data)

    def test_06(self):
        "FilterBandRejectIIR"

        gold_fn = fn("gold/filter_iir_band_reject.wav")

        f6 = ns.FilterBandRejectIIR(self.SR, 4, 440.0, 880.0, 0.01)

        data = f6.filter(self._input)

        self._compare_buffers(gold_fn, data)

        gold_fn = fn("gold/filter_iir_band_reject_dynamic.wav")

        data = f6.filter(self._input, self._freqs_lo, self._freqs_hi)

        self._compare_buffers(gold_fn, data)

    def test_07(self):
        "FilterBandPassFIR"

        gold_fn = fn("gold/filter_fir_band_pass.wav")

        f7 = ns.FilterBandPassFIR(self.SR, 512, 440, 880)

        data = f7.filter(self._input)

        self._compare_buffers(gold_fn, data)

        gold_fn = fn("gold/filter_fir_band_pass_dynamic.wav")

        data = f7.filter(self._input, self._freqs_lo, self._freqs_hi)

        self._compare_buffers(gold_fn, data)

    def test_08(self):
        "FilterBandPassIIR"

        gold_fn = fn("gold/filter_iir_band_pass.wav")

        f8 = ns.FilterBandPassIIR(self.SR, 4, 440.0, 880.0, 0.01)

        data = f8.filter(self._input)

        self._compare_buffers(gold_fn, data)

        gold_fn = fn("gold/filter_iir_band_pass_dynamic.wav")

        data = f8.filter(self._input, self._freqs_lo, self._freqs_hi)

        self._compare_buffers(gold_fn, data)
