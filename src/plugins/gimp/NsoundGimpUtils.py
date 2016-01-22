###############################################################################
#
#  $Id: NsoundGimpUtils.py 494 2010-06-22 03:44:58Z weegreenblobbie $
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


###############################################################################
def getRow(layer, row, direction = "X"):

    from Nsound import Buffer

    # Preallocate memory
    red   = Buffer(layer.width)
    green = Buffer(layer.width)
    blue  = Buffer(layer.width)

    pixel_region = None
    raw_data = None
    n_pixels = None

    if direction == "X":
        n_pixels = layer.width
        pixel_region = layer.get_pixel_rgn(0,row, n_pixels, 1)
        raw_data = pixel_region[:,row]
    else:
        n_pixels = layer.height
        pixel_region = layer.get_pixel_rgn(row, 0, 1, n_pixels)
        raw_data = pixel_region[row,:]

    bpp = layer.bpp

    for i in range(0, bpp*n_pixels, bpp):

        r = 0.0
        g = 0.0
        b = 0.0
        a = 0.0

        if bpp == 4:
            (r,g,b,a) = struct.unpack('BBBB', raw_data[i:i+4])
        elif bpp == 3:
            (r,g,b) = struct.unpack('BBB', raw_data[i:i+3])
        elif bpp == 2:
            (r,a) = struct.unpack('BB', raw_data[i:i+2])
        elif bpp == 1:
            r = struct.unpack('B', raw_data[i])[0]

        red   << r
        green << g
        blue  << b

    if bpp >= 3:
        return [red, green, blue]
    elif bpp >= 1:
        return [red]

###############################################################################
def setRow(layer, row, pixels, direction = "X"):

    # Filter out possible NaNs
    for i in range(len(pixels)):
        buf = pixels[i]

        for j in range(buf.getLength()):
            if math.isnan(buf[j]):
                buf[j] = 0.0

        pixels[i] = buf

    n_pixels = None
    if direction == "X":
        n_pixels = layer.width
        pixel_region = layer.get_pixel_rgn(0,row, n_pixels, 1)
    else:
        n_pixels = layer.height
        pixel_region = layer.get_pixel_rgn(row, 0, 1, n_pixels)

    bpp = layer.bpp

    raw_string = ""

    if bpp >= 3:
        (red,green,blue) = (pixels[0], pixels[1], pixels[2])
        green(green > 255).set(255)
        green(green <   0).set(0)
        blue(blue   > 255).set(255)
        blue(blue   <   0).set(0)

    elif bpp >= 1:
        red = pixels[0]

    # limit
    red(red     > 255).set(255)
    red(red     <   0).set(0)

    k = 0
    for i in range(0, bpp*n_pixels, bpp):

        if bpp == 4:
            raw_string += struct.pack("BBBB",
                int(red[k]),
                int(green[k]),
                int(blue[k]),
                255)
        elif bpp == 3:
            raw_string += struct.pack("BBBB",
                int(red[k]),
                int(green[k]),
                int(blue[k]))
        elif bpp == 2:
            raw_string += struct.pack("BB", red[k], 255)
        elif bpp == 1:
            raw_string += struct.pack("B", red[k])

        k += 1

    if direction == "X":
        pixel_region[0:layer.width, row] = raw_string
    else:
        pixel_region[row, 0:layer.height] = raw_string

