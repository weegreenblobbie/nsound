###############################################################################
#
#  $Id: FilterLowPassIIR_UnitTest.py 361 2009-09-10 00:37:56Z weegreenblobbie $
#
#  Copyright (c) 2009 to Present Nick Hilton
#
#  weegreenblobbie_yahoo_com (replace '_' with '@' and '.')
#
###############################################################################

###############################################################################
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU Library General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
#
###############################################################################

import sys

from Nsound import *

THIS_FILE = "FilterLowPassIIR_UnitTest.py"

GAMMA = 1.5e-10

###############################################################################
def plot_diff(b1, b2, title1, title2):
    diff = b1 - b2
    diff.plot("%s - %s" %(title1, title2))
    b1.plot(title1)
    b2.plot(title2)

    import pylab
    pylab.show()

###############################################################################
def FilterLowPassIIR_UnitTest():

    print ""
    print "%s" % THIS_FILE
    print ""

    Wavefile.setDefaultSampleSize(48)

    f = FilterLowPassIIR(1000.0, 4, 120.0)

    noise = Buffer()

    noise << "gold/Filter_noise.wav"

    ###########################################################################
    sys.stdout.write("Testing FilterLowPassIIR::filter(input) ...")
    sys.stdout.flush()

    data = Buffer()

    data = f.filter(noise)
    data.normalize()

    gold = Buffer()

    gold << "gold/FilterLowPassIIR_out1.wav"

    diff = data - gold

    if diff.getAbs().getMax() > GAMMA:

        sys.stderr.write(" Output did not match expected values!\n")

        plot_diff(data, gold, "data", "gold")

        raise ValueError

    print " SUCCESS"

    ###########################################################################
    sys.stdout.write("Testing FilterLowPassIIR::filter(input, freqs) ...")
    sys.stdout.flush()

    sin = Sine(100.0)

    freqs = Buffer()
    freqs << sin.drawLine(1.0, 6.0, 50.0)

    data = f.filter(noise, freqs)
    data.normalize()

    gold = Buffer()

    gold << "gold/FilterLowPassIIR_out2.wav"

    diff = data - gold

    if diff.getAbs().getMax() > GAMMA:
        sys.stderr.write(" Output did not match expected values!\n")

        plot_diff(data, gold, "data", "gold")

        raise ValueError

    print " SUCCESS"

    ###########################################################################
    # Repeat to test that reset() is being called.
    sys.stdout.write("Testing FilterLowPassIIR::filter(input) ...")
    sys.stdout.flush()

    data = f.filter(noise)
    data.normalize()

    gold = Buffer()

    gold << "gold/FilterLowPassIIR_out1.wav"

    diff = data - gold

    if diff.getAbs().getMax() > GAMMA:

        sys.stderr.write(" Output did not match expected values!\n")

        plot_diff(data, gold, "data", "gold")

        raise ValueError

    print " SUCCESS"

    ###########################################################################
    # Repeat to test that reset() is being called.
    sys.stdout.write("Testing FilterLowPassIIR::filter(input, freqs) ...")
    sys.stdout.flush()

    freqs = Buffer()
    freqs << sin.drawLine(1.0, 6.0, 50.0)

    data = f.filter(noise, freqs)
    data.normalize()

    gold = Buffer()

    gold << "gold/FilterLowPassIIR_out2.wav"

    diff = data - gold

    if diff.getAbs().getMax() > GAMMA:
        sys.stderr.write(" Output did not match expected values!\n")

        plot_diff(data, gold, "data", "gold")

        raise ValueError

    print " SUCCESS"

