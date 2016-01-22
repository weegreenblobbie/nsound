"""

$Id: moogvcf-rt.py 925 2015-08-23 02:41:47Z weegreenblobbie $

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
import sys
import Queue as queue
import multiprocessing
import datetime
now = datetime.datetime.now

import pygame
import matplotlib
from matplotlib import pylab
import numpy as np

import Nsound as ns

#------------------------------------------------------------------------------
# Globals

SR = 48000.0
CHANNELS = 2
N_BUFFERS = 3
LO_T = 0.02
HI_T = 0.10 # Higher latency for non-JACK

WINDOW_SIZE = (400, 200)
FIGSIZE = (5, 3)

DEBUG_RECORD = True


def limit(x, y):

    if x < 0:
        x = 0

    if x >= WINDOW_SIZE[0]:
        x = WINDOW_SIZE[0] - 1

    if y < 0:
        y = 0

    if y >= WINDOW_SIZE[1]:
        y = WINDOW_SIZE[1] - 1

    return x, y


def worker(q, plot, disp_q, signal, sig_dur, t):

    #--------------------------------------------------------------------------
    # Initialize

    # worker state machine
    enum = 0

    WAITING = enum ; enum += 1
    PLAYING = enum ; enum += 1

    try:
        playback = ns.AudioPlaybackRt(SR, CHANNELS, N_BUFFERS, t)
    except:
        q.put(False)
        sys.stderr.write("FAILURE: failed to create audio playback object!\n")
        sys.stderr.flush()
        return

    playback.setBufferUnderrunMode(ns.BUM_NOISE)

    # Circular iterator

    circ = signal.cbegin()

    #--------------------------------------------------------------------------
    # lookup tables

    # Feedback gain

    cutoff_min = 1.0
    cutoff_max = 16000.0

    res_min = 0.000
    res_max = 1.000

    g = ns.Generator(1)

    cutoff_table = g.drawLine(WINDOW_SIZE[0], cutoff_min, cutoff_max)
    res_table    = g.drawLine(WINDOW_SIZE[1], res_min,    res_max)

    lpf = ns.FilterLowPassMoogVcf(SR, 800.0, 0.5)

    gen = ns.Generator(SR)

    if DEBUG_RECORD:
        aout = ns.AudioStream(SR, 1)
        aid = 0

    # Tell the main thread we succeeded in initializing our
    # nsound objects.

    q.put(True)

    while q.full():
        pass

    #--------------------------------------------------------------------------
    # Sound processing loop

    space = ' ' * 48

    state = WAITING

    x0, y0 = None, None
    x1, y1 = None, None

    reset_first = False

    dur = 0.01
    dur_samples = int(SR * dur)

    process_pos = True

    while True:

        try:
            pos = q.get(False)
            process_pos = True
        except queue.Empty:
            process_pos = False

        if process_pos:

            if pos == "QUIT":
                playback.stop()
                return

            elif pos == "STOP":

                gen.reset()
                lpf.reset()

                playback.stop()

                state = WAITING

                disp_q.put("RESET")

                sys.stdout.write("\r%s\r" % space)
                sys.stdout.flush()

                if DEBUG_RECORD:
                    fn = "moogvcf-rt-%02d.wav" % aid
                    aout >> fn

                    print "Wrote %s" % fn

                    aout = ns.AudioStream(SR, 1)
                    aid += 1

            elif state == WAITING:

                x1, y1 = pos

                y1 = WINDOW_SIZE[1] - y1

                x1, y1 = limit(x1, y1)

                if x0 is None:
                    x0, y0 = x1, y1

                state = PLAYING

            elif state == PLAYING:
                x1, y1 = pos
                y1 = WINDOW_SIZE[1] - y1

                x1, y1 = limit(x1, y1)

        if state != PLAYING:
            continue

        cut0 = cutoff_table[x0]
        cut1 = cutoff_table[x1]

        res0 = res_table[y0]
        res1 = res_table[y1]

        cut_avg = (cut0 + cut1) / 2.0
        res_avg = (res0 + res1) / 2.0

        if plot:
            disp_q.put([cut_avg, res_avg])

        fmt = "\r{space}\rcutoff = %5.1f Hz, resonance = %.3f".format(space = space)

        sys.stdout.write(fmt % (cut_avg, res_avg))
        sys.stdout.flush()

        cut = gen.drawLine(dur, cut0, cut1)
        res = gen.drawLine(dur, res0, res1)

        temp = ns.Buffer(len(cut))

        for i in xrange(len(cut)):

            x = float(circ)
            circ += 1

            y = lpf.filter(x, cut[i], res[i])

            temp << y

        temp >> playback

        if DEBUG_RECORD:
            aout << temp

        x0, y0 = x1, y1


def display(q):
    '''
    The worker thread sends filter cutoff and Q for display.
    '''

    pylab.ion()

    fig = pylab.figure(figsize = FIGSIZE)
    fig.subplots_adjust(bottom = 0.2, left = 0.15)

    ax1 = fig.add_subplot(111)
    ax1.grid(True)
    ax1.set_xlabel('Frequency (Hz)')
    ax1.set_ylabel('Filter Response (dB)')

    cutoff = 800.0
    res = 0.5

    lpf = ns.FilterLowPassMoogVcf(SR, cutoff, res)
    lpf.setRealtime(True)

    #--------------------------------------------------------------------------
    # Create filter response plot

    xdata = lpf.getFrequencyAxis()
    ydata = lpf.getFrequencyResponse()
    ydata.dB()

    line1, = ax1.plot(xdata, ydata, rasterized = True)
    line2 = ax1.axvline(cutoff, color="r")

    ax1.set_xscale('log')
    ax1.axis([xdata[0], xdata[-1], -60, 30])

    fig.canvas.draw()

    # cache the backgrounds
    bg1 = fig.canvas.copy_from_bbox(ax1.bbox)

    d0 = now()

    while True:

        try:
            d = q.get(False)
            have_d = True
        except queue.Empty:
            have_d = False

        if not have_d: continue

        if d == "QUIT":
            return

        elif d == "RESET":
            #data = np.zeros(ydata.getLength())
            #line1.set_ydata(data)
            #ax1.draw_artist(ax1.patch)
            #ax1.draw_artist(line1)
            fig.canvas.draw()
            fig.canvas.flush_events()
            continue

        d1 = now()
        dt = d1 - d0

        if dt.total_seconds() < 1.0/2: continue

        # reduce the number of times to redraw the plot to save CPU
        if (
            abs(cutoff - d[0]) < 8.0 and
            abs(res - d[1]) < 0.1
        ):
            continue

        #----------------------------------------------------------------------
        # design new filter

        cutoff, res = d

        lpf.filter(0, cutoff, res)

        ydata = lpf.getFrequencyResponse()
        ydata.dB()

        #----------------------------------------------------------------------
        # Redraw plot

        line1.set_ydata(ydata)
        line2.set_xdata(cutoff)

        # restore background
        fig.canvas.restore_region(bg1)

        ax1.draw_artist(ax1.patch)
        ax1.draw_artist(line1)
        ax1.draw_artist(line2)

        fig.canvas.blit(ax1.bbox)

        # cache the backgrounds
        bg1 = fig.canvas.copy_from_bbox(ax1.bbox)

        d0 = now()


def main():

    parser = argparse.ArgumentParser(description = 'moogvcf-rt')

    parser.add_argument(
        '-j',
        '--jack',
        dest    = 'use_jack',
        default = False,
        action  = 'store_true',
        help    = """Use the JACK portaudio Host API, if the JACK server isn't
running, you will get undefined behavior""")

    parser.add_argument(
        '-p',
        '--plot',
        dest    = 'plot',
        default = False,
        action  = 'store_true',
        help    = """Plots the low pass filter's frequency response""")

    parser.add_argument(
        'input_wave',
        help = "Input wave to process")

    args = parser.parse_args()

    # Low latency

    if args.use_jack:
        print "Will try to use JACK (known to work on Linux)"
        ns.AudioPlaybackRt.use_jack(True)
        t = LO_T

    # High latency
    else:
        print "Using High Latency"
        t = HI_T

    signal = ns.AudioStream(args.input_wave).getMono()
    signal.resample2(SR)
    signal.normalize()
    signal *= 0.666

    q = multiprocessing.Queue(maxsize = 1)
    disp_q = multiprocessing.Queue(maxsize = 1)

    p = multiprocessing.Process(
        target=worker, args = (q, args.plot, disp_q, signal[0], signal.getDuration(), t))
    p.start()

    if args.plot:
        disp_p = multiprocessing.Process(target = display, args = (disp_q,))
        disp_p.start()

    # read from queue that the process initialized and is ready.
    good = q.get()

    if not good:
        sys.exit(1)

    # Init only the display, otherwise when using JACK Audio Connection Kit,
    # pygame won't exit cleanly.
    pygame.display.init()

    # Open window
    window = pygame.display.set_mode((WINDOW_SIZE[0], WINDOW_SIZE[1]))
    pygame.display.set_caption("Moog VCF RT! %s" % ns.__package__)

    # Turn off key repeats
    pygame.key.set_repeat()

    print "\n" * 20

    print "Hello Moog VCF RT!"
    print "Press ESC to quit."


    enum = 0
    IDLE = enum ; enum += 1
    PLAYING = enum ; enum += 1

    state = IDLE

    main_loop = True

    while main_loop:

        event = pygame.event.wait()

        #print "event = ", event

        if state == PLAYING and event.type == pygame.MOUSEMOTION:
            q.put(event.pos)

        elif event.type == pygame.MOUSEBUTTONDOWN:
            q.put(event.pos)
            state = PLAYING

        elif event.type == pygame.MOUSEBUTTONUP:
            q.put("STOP")
            state = IDLE

        elif event.type == pygame.KEYDOWN:

            # Break out of loop
            if event.key == pygame.K_ESCAPE:
                print "Quitting main loop"
                sys.stdout.flush()
                main_loop = False
                break

        elif event.type == pygame.QUIT:
            print "Quitting main loop"
            sys.stdout.flush()
            main_loop = False
            break

    q.put("QUIT")
    p.join()
    if args.plot:
        disp_q.put("QUIT")
        disp_p.join()
    pygame.display.quit()
    print "Goodbye!"


if __name__ == "__main__":
    main()
