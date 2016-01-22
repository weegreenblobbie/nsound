#! /usr/bin/env python

###############################################################################
#
#  $Id: NsoundGimpHighLowPass.py 491 2010-06-20 21:55:54Z weegreenblobbie $
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
import re
import struct
import sys

import gimp
import gimpfu

from Nsound import Buffer
from Nsound import FFTransform
from Nsound import FFTChunk

from NsoundGimpUtils import *

###############################################################################
def run(img, layer, name, domain, axis):

    if axis == "X":
        (X, Y) = (layer.width, layer.height)
    else:
        (Y, X) = (layer.width, layer.height)

    mag_layer = None
    pha_layer = None
    type = None

    new_layers = []

    gimp.context_push()
    img.undo_group_start()

    if img.base_type == gimpfu.RGB:
        type = gimpfu.RGBA_IMAGE
    else:
        type = gimpfu.GRAY_IMAGE

    if domain == "FORWARD":

        w = X

        X = FFTransform.roundUp2(X)

        new_layers.append(
            gimp.Layer(img, name + " Phase %d" % w, X/2, Y, type, 100.0,
                gimpfu.NORMAL_MODE))

        new_layers.append(
            gimp.Layer(img, name + " Magnitude %d" % w, X/2, Y, type, 100.0,
                gimpfu.NORMAL_MODE))

    elif domain == "REVERSE":

        # Search the image for the highest layers that are labeled with Phase
        # and Magnitude

        for l in img.layers:

            if " Magnitude " in l.name:
                mag_layer = l

            elif " Phase " in l.name:
                pha_layer = l

            if mag_layer != None and pha_layer != None:
                break

        if mag_layer == None:

            raise Exception("Could not locate 'FFT Magnitude X' layer!")

        elif pha_layer == None:

            raise Exception("Could not locate 'FFT Phase X' layer!")

        # Okay, we have both layers, read the width in the name

        X = None

        t1 = re.search(" [0-9]+", mag_layer.name).group(0)
        t2 = re.search(" [0-9]+", pha_layer.name).group(0)

        if t1 == t2:
            X = int(t1)

        if X == None:
            raise Exception("Could not determing layer's original size!")

        new_layers.append(
            gimp.Layer(img, name + " Reverse", X, Y, type, 100.0,
                gimpfu.NORMAL_MODE))


    for i in range (len(new_layers)):

        new_layers[i].set_offsets(layer.offsets[0],layer.offsets[1])

        new_layers[i].fill(gimpfu.TRANSPARENT_FILL)

        img.add_layer(new_layers[i], 0)

        # New mask
        mask = new_layers[i].create_mask(0)
        new_layers[i].add_mask(mask)

    gimp.progress_init("Transforming pixels ...")

    transform = FFTransform(X)

    if domain == "FORWARD":

        for y in range(Y):

            gimp.progress_update(float(y) / float(Y))

            pixels = getRow(layer, y, axis)

            output_pixels = [ [], [] ]

            for color in range(len(pixels)):

                buf = pixels[color]

                buf.normalize()

                # Transform into frequency domain
                if domain == "FORWARD":

                    # Perform over sampled FFT
                    fdomain = transform.fft(buf, X, 0)

                    mag = fdomain[0].getMagnitude()
                    pha = fdomain[0].getPhase()

                    mag.normalize()
                    pha.normalize()

                    # Scale to pixel max
                    mag *= 255.0
                    pha *= 255.0

                    # Stuff into the output pixels
                    output_pixels[0].append(pha)
                    output_pixels[1].append(mag)

            for i in range(len(output_pixels)):
                setRow(new_layers[i], y, output_pixels[i], axis)

    elif domain == "REVERSE":

        for y in range(Y):

            gimp.progress_update(float(y) / float(Y))

            pixels = {}

            pixels["mag"] = getRow(mag_layer, y, axis)
            pixels["pha"] = getRow(pha_layer, y, axis)

            output_pixels = []

            for color in range(len(pixels["mag"])):

                mag = pixels["mag"][color]
                pha = pixels["pha"][color]

                mag.normalize()

                # Center the phase about 0.0 and scale by PI
                pha -= 0.5 * pha.getMean()
                pha.normalize()

                pha *= math.pi

                ch = FFTChunk(X)

                ch.setPolar(mag,pha)

                tdomain = transform.ifft([ch])
                tdomain.normalize()

                output_pixels.append(255.0 * tdomain)

            setRow(new_layers[0], y, output_pixels, axis)

    gimp.progress_update(1.0)

    # Apply changes
    for i in range(len(new_layers)):
        new_layers[i].remove_mask(gimpfu.MASK_APPLY)

    img.undo_group_end()
    gimp.context_pop()

###############################################################################
gimpfu.register(
    "nsound_fft",                                     # procedure name
    "Transforms Pixels To The Frequency Domain",      # blurb
    "Transforms Pixels To The Frequency Domain",      # help
    "Nick Hilton",                                    # author
    "Nick Hilton",                                    # copyright
    "2010",                                           # date/year
    "FFT",                                            # label
    "RGB*, GRAY*",                                    # imagetypes

    # input paramaters
    [
        (gimpfu.PF_IMAGE,    "image",     "Input image",     None),
        (gimpfu.PF_DRAWABLE, "layer",     "Input drawable",  None),
        (gimpfu.PF_STRING,   "name",      "Layer name",     "FFT"),
        (gimpfu.PF_RADIO,    "domain",    "Direction: {Forward, Reverse}",
            "FORWARD", (("Forward","FORWARD"), ("Reverse","REVERSE"))),

        (gimpfu.PF_RADIO,    "axis",    "Axis: {X, Y}",
            "X", (("X","X"), ("Y","Y"))),
    ],
    [],                                 # output result
    run,                                # function
    menu="<Image>/Filters/Nsound",      # menu location
#~    domain=None,
#~    on_query=None,
#~    on_run=None
    )

gimpfu.main()

