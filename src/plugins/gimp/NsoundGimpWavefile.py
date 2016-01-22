#! /usr/bin/env python

###############################################################################
#
#  $Id: NsoundGimpWavefile.py 494 2010-06-22 03:44:58Z weegreenblobbie $
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
import gtk

from Nsound import Buffer, ID3v1Tag

from NsoundGimpUtils import *

###############################################################################
def load_wavefile(filename, raw_filename):

    gimp.progress_init("Importing Wavefile ...")

    progress = 0.0
    progress_offset = 0.05

    # Read the file.
    buf = Buffer(filename)

    buf -= buf.getMean()
    buf.normalize()

    n_pixels = buf.getLength()

    # Read ID3v1 tag if available.
    t = ID3v1Tag()

    w = None
    h = None

    if t.read(filename, False) and t.title == "GIMP+Python+Nsound":

        try:
            temp = re.search("w=[0-9]+", t.comment).group(0)

            w = int(temp[2:])

            temp = re.search("h=[0-9]+", t.comment).group(0)

            h = int(temp[2:])

        except:

            w = None
            h = None

    # If we didn't read the tag, create some dimensions that are 4:3

    if w == None:
        ratio = 4.0 / 3.0

        w = 4.0
        h = 3.0

        while w * h < n_pixels:

            r = w / h

            if r < ratio:
                w += 1.0
            else:
                h += 1.0

            if w >= 1600:
                break

        w = int(w)
        h = int(h)

    gimp.progress_update(progress_offset)

    img = gimp.Image(w, h, gimpfu.GRAY)

    layer = gimp.Layer(img, "Wavefile", w, h, gimpfu.GRAYA_IMAGE,
        100.0, gimpfu.NORMAL_MODE)

    layer.fill(gimpfu.TRANSPARENT_FILL)

    img.add_layer(layer, 0)

    # New mask
    mask = layer.create_mask(0)
    layer.add_mask(mask)

    (X, Y) = (layer.width, layer.height)

    progress_step = (1.0 - progress_offset) / float(Y)

    k = 0
    for y in range(Y):

        gimp.progress_update(progress_offset + float(y) / float(Y))

        # Allocate memory
        pixels = Buffer(Y)

        for x in range(X):
            pixels << 127.0 * buf[k] + 127.0
            k += 1

            if k >= n_pixels:
                while x < X:
                    pixels << 127.0
                    x += 1
                break

        setRow(layer, y, [pixels])

        if k >= n_pixels:
            break

    gimp.progress_update(1.0)

    # Apply changes
    layer.remove_mask(gimpfu.MASK_APPLY)

    return img

###############################################################################
def save_wavefile(img, layer, filename, raw_filename):

    gimp.progress_init("Exporting Wavefile ...")

    progress = 0.0
    progress_offset = 0.05

    # Create Buffer
    # Read the file.
    buf = Buffer(layer.width * layer.height)

    gimp.progress_update(progress_offset)

    (X, Y) = (layer.width, layer.height)

    progress_step = (1.0 - progress_offset) / float(Y)

    for y in range(Y):

        gimp.progress_update(-progress_offset + float(y) / float(Y))

        pixels = getRow(layer, y)

        values = None

        # Convert color to gray
        if len(pixels) == 3:

            values = pixels[0] * pixels[0] \
                   + pixels[1] * pixels[1] \
                   + pixels[2] * pixels[2]

            values.sqrt()

        elif len(pixels) == 1:
            values = pixels[0]

        buf << values

    # All pixels are now in the Buffer normalize and center about zero.
    mmax = buf.getMax()
    distance = mmax - buf.getMin()
    l = mmax - 0.5 * distance
    buf -= l
    buf.normalize()

    # Scale a bit so it's not loud.
    buf *= 0.66

    # Write out to file.
    buf >> filename

    # Add an ID3v1 tag
    t = ID3v1Tag()
    t.title = "GIMP+Python+Nsound"
    t.comment = "w=%d h=%d" %(X,Y)

    t.write(filename)

    gimp.progress_update(1.0)

###############################################################################
def register_load_handlers():
    gimp.register_load_handler('file-wavefile-load', 'wav', '')

###############################################################################
def register_save_handlers():
    gimp.register_save_handler('file-wavefile-save', 'wav', '')

###############################################################################
# Save
gimpfu.register(
    "file-wavefile-save",
    "Saves the image as a wavefile (.wav)",    # Description
    "Saves the image as a wavefile (.wav)",
    "Nick Hilton",                          # Author
    "Nick Hilton",                          # Copyright
    "2010",                                 # Year
    "Wavefile",
    "*",
    [   #input args. Format (type, name, description, default [, extra])
        (gimpfu.PF_IMAGE,    "img",          "Input image", None),
        (gimpfu.PF_DRAWABLE, "layer",        "Input drawable", None),
        (gimpfu.PF_STRING,   "filename",     "The name of the file", None),
        (gimpfu.PF_STRING,   "raw_filename", "The name of the file", None),
    ],
    [], #results. Format (type, name, description)
    save_wavefile,
    on_query = register_save_handlers,
    menu = "<Save>",
    domain=("gimp20-python", gimp.locale_directory)
    )

###############################################################################
# Load
gimpfu.register(
    "file-wavefile-load",                   # Name
    "Loads wavefile (.wav) as an image",    # Description
    "Loads wavefile (.wav) as an image",
    "Nick Hilton",                          # Author
    "Nick Hilton",                          # Copyright
    "2010",                                 # Year
    "Wavefile",
    None,
    # input args. Format (type, name, description, default [, extra])
    [
        (gimpfu.PF_STRING, 'filename', 'The name of the file to load', None),
        (gimpfu.PF_STRING, 'raw_filename', 'The name entered', None),
    ],
    # output args
    [(gimpfu.PF_IMAGE, 'image', 'Output image')], #results. Format (type, name, description)
    load_wavefile,
    on_query = register_load_handlers,
    menu = "<Load>",
    domain=("gimp20-python", gimp.locale_directory)
    )

gimpfu.main()
