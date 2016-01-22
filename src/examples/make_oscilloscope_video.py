"""

$Id: make_oscilloscope_video.py 877 2014-11-13 03:51:39Z weegreenblobbie $

Nsound is a C++ library and Python module for audio synthesis featuring
dynamic digital filters. Nsound lets you easily shape waveforms and write
to disk or plot them. Nsound aims to be as powerful as Csound but easy to
use.

Copyright (c) 2004 to Present Nick Hilton

weegreenblobbie_at_yahoo_com

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

import argparse
import glob
import os
import sys

from matplotlib import pylab
import Nsound as ns
import numpy as np

#------------------------------------------------------------------------------
# Globals

png_filename = 'oscilloscope_%010d.png'


#------------------------------------------------------------------------------
# utility functions

def get_ydata(sr, signal, t, width):
    '''
    Samples the signal at pos : pos + width.  Then computes the location of the
    peak, and resamples the signal such that the peak is always loacted in the
    same position.
    '''

    pos = int(t * sr + 0.5)

    s0 = pos
    s1 = s0 + width

    if s1 > len(signal):
        s1 = len(signal) - 1
        s0 = s1 - width

    datay = np.array(signal[s0 : s1])

    # get location of the peak

    peak_idx = datay.argmax()

    # Center peak at 33% of windows

    global_index = pos + peak_idx

    s0 = int(global_index - 0.33333 * width)
    s1 = s0 + width

    if s0 >= len(signal):
        return np.zeros(width)

    if s1 >= len(signal):
#~        datay = signal[s0:]
#~        n = width - len(datay)
#~        datay = np.concatenate([datay, np.zeros(n)])
#~        return datay

        return np.zeros(width)

    return signal[s0 : s1]


def main():

    parser = argparse.ArgumentParser(description = 'oscillocope')

    parser.add_argument(
        '-c',
        '--channel',
        dest    = 'channel',
        default = 0,
        type = int,
        help    = "Input channel to use if n_channels > 1")

    parser.add_argument(
        '--fps',
        dest    = 'fps',
        default = 30,
        help    = "Frames-per-second to render")

    parser.add_argument(
        '--freq',
        dest    = 'show_freq',
        default = False,
        action = 'store_true',
        help    = "Display fundamental frequency")

    parser.add_argument(
        '--pk-pk',
        dest    = 'show_pk2pk',
        default = False,
        action = 'store_true',
        help    = "Display peak 2 peak")

    parser.add_argument(
        '-w',
        '--width',
        dest    = 'width',
        default = 0.05,
        type = float,
        help    = "Oscillocope's width in seconds")

    parser.add_argument(
        'filename',
        help    = "Input .wav file to process")

    args = parser.parse_args()

    assert args.width > 0
    assert args.fps > 0

    print "Hello Oscillocope!"

    #--------------------------------------------------------------------------
    # remove exising .png files

    pattern = png_filename.replace('%010d', '*')

    files = glob.glob(pattern)

    for f in files:
        os.remove(f)

    #--------------------------------------------------------------------------
    # read input file

    sys.stdout.write("Reading '%s' ... " % args.filename)
    sys.stdout.flush()

    audio_stream = ns.AudioStream(args.filename)
    sr = audio_stream.getSampleRate()

    data = audio_stream[args.channel]

    print " done"

    #--------------------------------------------------------------------------
    # compute width and step

    width = int(sr * args.width + 0.5)

    step = 1.0 / args.fps

    n_frames = audio_stream.getDuration() / step

    print "fps = ", args.fps
    print "sr = ", sr
    print "width = ", width
    print "step = ", step
    print "n_frames = ", n_frames

    #--------------------------------------------------------------------------
    # Create plot

    fig = pylab.figure()
    pylab.grid(True)
    pylab.xlabel('Time')
    pylab.ylabel('Amplitude')

    ax = fig.add_subplot(111)

    #--------------------------------------------------------------------------
    # Create xdata

    xdata = np.arange(0, width) / sr

    #--------------------------------------------------------------------------
    # Process the data

    if len(data) < width:
        # pad with zeros
        n = width - len(data)
        for i in xrange(n):
            data << 0

    # frame 0

    t = 0.5 * step

    pos = int(t * sr + 0.5)

    ydata = data[pos : pos + width]

    pylab.axis([xdata[0], xdata[-1], -1, 1])

    line, = ax.plot(xdata, ydata)

    # Lable with time

    time_text = ax.text(
        0.98, 0.98,
        '%3.2f sec' % t,
        horizontalalignment ='right',
        verticalalignment   = 'top',
        transform           = ax.transAxes)

    #--------------------------------------------------------------------------
    # Main loop

    update_display = round(n_frames / 200)

    if update_display <= 0:
        update_display = 1

    print "update_display = ", update_display

    dur = audio_stream.getDuration()

    n = 0

    while t < dur:

        if n % update_display == 0:
            pdone = 100.0 * n / n_frames
            sys.stdout.write("             \rProcessing: %6.2f%%" % pdone)
            sys.stdout.flush()

        ydata = get_ydata(sr, data, t, width)

        time_text.set_text('%3.2f sec' % t)

        line.set_ydata(ydata)

        fig.canvas.draw()

        pylab.savefig(png_filename % n)

        n += 1
        t += step

    pdone = 100.0
    sys.stdout.write("             \rProcessing: %6.2f%%" % pdone)
    print ""

    #--------------------------------------------------------------------------
    # encode audio + video

    # Pass 1
    cmd = (
        'ffmpeg -r {fps} -i {pngfiles} -r {sr} -i {audiofile} -pass 1 -vcodec libtheora '
        '-b:v 6000k -bt 6000k -pix_fmt yuv444p -r {fps} -f rawvideo -y /dev/null')

    cmd = cmd.format(
        audiofile = args.filename,
        fps = args.fps,
        pngfiles = png_filename,
        sr = sr,
        )

    os.system(cmd)

    # Pass 2

    cmd = (
        'ffmpeg -r {fps} -i {pngfiles} -r {sr} -i {audiofile} -pass 2 -vcodec libtheora '
        '-b:v 6000k -bt 6000k -pix_fmt yuv444p -r {fps} movie.ogv -y'
        )

    cmd = cmd.format(
        audiofile = args.filename,
        fps = args.fps,
        pngfiles = png_filename,
        sr = sr,
        )

    os.system(cmd)

    print "Goodbye!"

if __name__ == "__main__":
    main()
