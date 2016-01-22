#! /usr/bin/env python

###############################################################################
#
#  $Id: NsoundGimpHighLowPass.py 494 2010-06-22 03:44:58Z weegreenblobbie $
#
#  Nsound is a C++ library and Python module for audio synthesis featuring
#  dynamic digital filters. Nsound lets you easily shape waveforms and write
#  to disk or plot them. Nsound aims to be as powerful as Csound but easy to
#  use.
#
#  Copyright (c) 2004, 2005 Nick Hilton
#
#  weegreenblobbie_at_yahoo_com
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

import math
import struct
import sys

import gimp
import gimpfu

from Nsound import Buffer
from Nsound import FilterLowPassIIR
from Nsound import FilterLowPassFIR
from Nsound import FilterHighPassIIR
from Nsound import FilterHighPassFIR

from NsoundGimpUtils import *

###############################################################################
def run(img, layer, name, hi_lo, ftype, order, frequency, direction):

    gimp.context_push()
    img.undo_group_start()

    is_iir = None

    if ftype == "IIR":
        is_iir = True
    else:
        is_iir = False

    if img.base_type == gimpfu.RGB:
        type = gimpfu.RGBA_IMAGE
    else:
        type = gimpfu.GRAY_IMAGE

    new_layer = gimp.Layer(img, name, layer.width, layer.height, type,
            100.0, gimpfu.NORMAL_MODE)

    new_layer.set_offsets(layer.offsets[0],layer.offsets[1])

    new_layer.fill(gimpfu.TRANSPARENT_FILL)

    img.add_layer(new_layer, 0)

    # New mask
    mask = new_layer.create_mask(0)
    new_layer.add_mask(mask)

    f = None
    sample_rate = float(layer.width)

    if is_iir:
        if hi_lo == "LP":
            f = FilterLowPassIIR(sample_rate, order, frequency, 0.0)
        else:
            f = FilterHighPassIIR(sample_rate, order, frequency, 0.0)

        # Estimate filter delay

        pulse = Buffer(2*order)
        pulse << 1.0
        for i in range(0,order):
            pulse << 0.0

        result = Buffer(2*order)
        result << f.filter(pulse)

        # Find the peak
        mmax = result.getMax()

        delay = None

        for i in range(0,result.getLength()):
            if result[i] == mmax:
                delay = i + 1
                break

    else:
        if hi_lo == "LP":
            f = FilterLowPassFIR(sample_rate, order, frequency)
        else:
            f = FilterHighPassFIR(sample_rate, order, frequency)

        delay = (f.getKernelSize() - 1) / 2


    gimp.progress_init("Filtering pixels ...")

    done_factor = 1.0
    offset      = 0.0
    directions  = [direction]

    if direction == "Both":
        done_factor = 0.5
        directions = ["X","Y"]

    for axis in directions:

        if "X" == axis:
            (X, Y) = (layer.width, layer.height)
        else:
            (Y, X) = (layer.width, layer.height)

        for y in range(0, Y):

            gimp.progress_update( offset + done_factor * float(y) / float(Y))

            pixels = getRow(layer, y, axis)

            for i in range(len(pixels)):

                buf = pixels[i]

                f.reset()

                # Prime the filter
                for j in range(delay):
                    f.filter(buf[0])

                last = buf[-1]
                k = 0
                for j in range(buf.getLength()):

                    # while j is < delay, throw out samples
                    if j < delay:
                        f.filter(buf[j])
                    else:
                        buf[k] = f.filter(buf[j])
                        k += 1

                for j in range(delay):
                    buf[k] = f.filter(last)
                    k += 1

                pixels[i] = buf

            setRow(new_layer, y, pixels, axis)

        offset += 0.5
        layer = new_layer

    gimp.progress_update(1.0)

    # Apply changes
    new_layer.remove_mask(gimpfu.MASK_APPLY)

    img.undo_group_end()
    gimp.context_pop()

###############################################################################
gimpfu.register(
    "nsound_high_low_pass_filter",          # procedure name
    "High/Low Pass Filters Pixels",         # blurb
    "High/Low Pass Filters Pixels",         # help
    "Nick Hilton",                          # author
    "Nick Hilton",                          # copyright
    "2010",                                 # date/year
    "High/Low Pass Filter",                 # label
    "RGB*, GRAY*",                          # imagetypes

    # input paramaters
    [
        (gimpfu.PF_IMAGE,    "image",     "Input image",         None),
        (gimpfu.PF_DRAWABLE, "layer",     "Input drawable",      None),
        (gimpfu.PF_STRING,   "name",      "Layer name",         "Filtered"),
        (gimpfu.PF_RADIO,    "hi_lo",     "Type: {Low, High}",
            "LP", (("Low Pass","LP"), ("High Pass","HP"))),

        (gimpfu.PF_RADIO,    "ftype",     "Response: {FIR, IIR}",
            "FIR", (("FIR","FIR"), ("IIR","IIR"))),

        (gimpfu.PF_INT,      "order",     "Order",            16),
        (gimpfu.PF_FLOAT,    "frequency", "Freq",           15.0),
        (gimpfu.PF_RADIO,    "direction", "Axis: {X, Y, Both}",
            "Both", (("X","X"), ("Y","Y"), ("Both", "Both"))),
    ],
    [],                             # output result
    run,                            # function
    "<Image>/Filters/Nsound",       # menu location
#~    domain=None,
#~    on_query=None,
#~    on_run=None
    )

gimpfu.main()

