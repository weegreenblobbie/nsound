"""

$Id: bebot-disp.py 912 2015-07-26 00:50:29Z weegreenblobbie $

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
import queue as queue
import multiprocessing
import datetime
now = datetime.datetime.now

import matplotlib
#~matplotlib.use('TkAgg') # not antigrain smoothing for faster render times
from matplotlib import pylab

import pygame
import Nsound as ns
import numpy as np


#------------------------------------------------------------------------------
# Globals

SR = 48000.0
CHANNELS = 2
N_BUFFERS = 3
LO_T = 0.02
HI_T = 0.05 # Higher latency for non-JACK

WINDOW_SIZE = 400

DEBUG_RECORD = False

#-------------------------------------------
# oscilliscope display settings

DISP_WIDTH = 0.05

# spectrogram settings
SPEC_WINDOW = 0.100    # seconds
SPEC_MAX_FREQ = 16000.0 # Hz


def limit(x, y):

    if x < 0:
        x = 0

    if x >= WINDOW_SIZE:
        x = WINDOW_SIZE - 1

    if y < 0:
        y = 0

    if y >= WINDOW_SIZE:
        y = WINDOW_SIZE - 1

    return x, y


def worker(q, disp_q, t):

    #--------------------------------------------------------------------------
    # Initialize

    # worker state machine
    enum = 0

    WAITING = enum ; enum += 1
    PLAYING = enum ; enum += 1
    RELEASING = enum; enum += 1

    try:
        playback = ns.AudioPlaybackRt(SR, CHANNELS, N_BUFFERS, t)
    except:
        q.put(False)
        sys.stderr.write("FAILURE: failed to create audio playback object!\n")
        sys.stderr.flush()
        return

    gen = ns.Sine(SR)
    gen.setRealtime(True)

    lpf = ns.FilterLowPassIIR(SR, 6, 80, 0.00)
    lpf.setRealtime(True)

    gent = ns.Triangle(SR, 0.02, 0.02)
    gent.setRealtime(True)

    gend = ns.GeneratorDecay(SR)
    gend.setRealtime(True)

    gend.addSlaveSync(gen)

    # Tell the main thread we succeeded in initializing our
    # nsound objects.

    q.put(True)

    while q.full():
        pass

    #--------------------------------------------------------------------------
    # lookup tables

    freq_min = 40
    freq_max = 650

    ks_min = 200
    ks_max = 8000

    alpha_min = 6
    alpha_max = 36

    f_cut_min = 1000
    f_cut_max = 7000

    # Make tables
    g = ns.Generator(1)

    freq_table  = g.drawLine(WINDOW_SIZE, freq_min,  freq_max)
    ks_table    = g.drawLine(WINDOW_SIZE, ks_min,    ks_max)
    alpha_table = g.drawLine(WINDOW_SIZE, alpha_min, alpha_max)
    f_cut_table = g.drawLine(WINDOW_SIZE, f_cut_min, f_cut_max)

    state = WAITING

    x0, y0 = None, None
    x1, y1 = None, None

    reset_first = False

    if DEBUG_RECORD:
        aout = ns.AudioStream(SR, 1)
        aid = 0

    #--------------------------------------------------------------------------
    # Sound processing loop

    dur = 0.01

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
                gend.reset()
                gent.reset()
                lpf.reset()

                playback.stop()

                state = WAITING

                disp_q.put("RESET")

                if DEBUG_RECORD:
                    fn = "bebot-%02d.wav" % aid
                    aout >> fn

                    print("Wrote %s" % fn)

                    aout = ns.AudioStream(SR, 1)
                    aid += 1

            elif state == WAITING:

                x1, y1 = pos

                y1 = WINDOW_SIZE - y1

                x1, y1 = limit(x1, y1)

                if x0 is None:
                    x0, y0 = x1, y1

                state = PLAYING

            elif state == PLAYING:
                x1, y1 = pos
                y1 = WINDOW_SIZE - y1

                x1, y1 = limit(x1, y1)

        if state != PLAYING:
            continue

        f0 = freq_table[x0]
        f1 = freq_table[x1]

        k0 = ks_table[y0]
        k1 = ks_table[y1]

        a0 = alpha_table[y0]
        a1 = alpha_table[y1]

        fc0 = f_cut_table[y0]
        fc1 = f_cut_table[y1]

        freqs = gen.drawLine(dur, f0, f1)
        ks    = gen.drawLine(dur, k0, k1)
        alpha = gen.drawLine(dur, a0, a1)
        f_cut = gen.drawLine(dur, fc0, fc1)

        temp = gend.generate2(dur, freqs, alpha)
        temp *= gen.generate(dur, ks)

        temp += 1.0

        temp *= gent.generate(dur, freqs)

        temp /= 2.0

        temp = 0.333 * lpf.filter(temp, f_cut)

        disp_q.put(temp)
        temp >> playback

        if DEBUG_RECORD:
            aout << temp

        x0, y0 = x1, y1


def display(q):
    '''
    The worker thread sends audio data this display thread for real-time
    oscilliscope display.
    '''

    pylab.ion()

    fig = pylab.figure(figsize=(12,6))

    ax1 = fig.add_subplot(211)
    ax1.grid(True)
    ax1.set_xlabel('Time')
    ax1.set_ylabel('Amplitude')

    ax2 = fig.add_subplot(212)
    ax2.grid(True)
    ax2.set_xlabel('Frequency (Hz)')
    ax2.set_ylabel('Amplitude')

    #--------------------------------------------------------------------------
    # Create oscilliscope data and plot axes

    xdata = np.arange(int(DISP_WIDTH * SR + 0.5)) / float(SR)

    n_samples = len(xdata)

    ydata = ns.Buffer.zeros(n_samples)

    ax1.axis([xdata[0], xdata[-1], -0.5, 0.5])

    line1, = ax1.plot(xdata, ydata, rasterized = True)

    # Create spectrogram data and plot axes

    spec_args = (SR, SPEC_WINDOW, 0.010, ns.NUTTALL)

    spec = ns.Spectrogram(ydata, *spec_args)

    spec_xaxis = np.array(spec.getFrequencyAxis())
    spec_yzeros = np.array(spec.computeMagnitude(ydata)[1:])

    line2, = ax2.plot(spec_xaxis, spec_yzeros, rasterized = True)

    ax2.set_xlim(20, SPEC_MAX_FREQ)
    ax2.set_xscale('log')

    ax2.set_ylim(-90, 60)

    fig.canvas.draw()

    # cache the backgrounds
    bg1 = fig.canvas.copy_from_bbox(ax1.bbox)
    bg2 = fig.canvas.copy_from_bbox(ax2.bbox)

    cbuf = ns.CircularBuffer(2*n_samples)

    d0 = now()

    dcount = 0

    while True:

        try:
            d = q.get(False)
            have_d = True
        except queue.Empty:
            have_d = False

        if not have_d: continue

        if d == "QUIT":
            print("dcount = ", dcount)
            return

        elif d == "RESET":
            data = np.zeros(n_samples)
            line1.set_ydata(data)
            line2.set_ydata(spec_yzeros)
            ax1.draw_artist(ax1.patch)
            ax1.draw_artist(line1)
            ax2.draw_artist(ax2.patch)
            ax2.draw_artist(line2)
            fig.canvas.draw()
            fig.canvas.flush_events()
            cbuf.write(ns.Buffer.zeros(2*n_samples))
            continue

        cbuf.write(d)

        d1 = now()
        dt = d1 - d0

        if dt.total_seconds() >= (1.0/30):

            dcount += 1

            data = cbuf.read()

            #------------------------------------------------------------------
            # oscilliscope

            # Search the middle part of the data for the peak value

            n = len(data)

            i0 = n / 4
            i1 = i0 + n_samples/2

            middle = data[i0 : i1]

            imax = i0 + middle.argmax()

            # keep peak at 25 % into the window

            i0 = imax - n_samples/4
            i1 = i0 + n_samples

            ydata = data[i0 : i1]

            #------------------------------------------------------------------
            # spectrogram

            spec_ydata = spec.computeMagnitude(data).getdB()[1:]

            #------------------------------------------------------------------
            # Redraw plot

            line1.set_ydata(ydata)
            line2.set_ydata(spec_ydata)

            # restore background
            fig.canvas.restore_region(bg1)
            fig.canvas.restore_region(bg2)

            ax1.draw_artist(ax1.patch)
            ax1.draw_artist(line1)
            ax2.draw_artist(ax2.patch)
            ax2.draw_artist(line2)

            fig.canvas.blit(ax1.bbox)
            fig.canvas.blit(ax2.bbox)

            # cache the backgrounds
            bg1 = fig.canvas.copy_from_bbox(ax1.bbox)
            bg2 = fig.canvas.copy_from_bbox(ax2.bbox)

            d0 = now()

def main():

    parser = argparse.ArgumentParser(description = 'bebot')

    parser.add_argument(
        '-j',
        '--jack',
        dest    = 'use_jack',
        default = False,
        action  = 'store_true',
        help    = """Use the JACK portaudio Host API, if the JACK server isn't
running, you will get undefined behavior""")

    args = parser.parse_args()

    # Low latency

    if args.use_jack:
        print("Will try to use JACK (known to work on Linux)")
        ns.AudioPlaybackRt.use_jack(True)
        t = LO_T

    # High latency
    else:
        print("Using High Latency")
        t = HI_T

    print("Hello Bebot!")
    print("Press ESC to quit.")

    q = multiprocessing.Queue(maxsize = 1)
    disp_q = multiprocessing.Queue()

    p = multiprocessing.Process(target = worker, args = (q, disp_q, t))
    p.start()

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
    window = pygame.display.set_mode((WINDOW_SIZE, WINDOW_SIZE))
    pygame.display.set_caption("Bebot! %s" % ns.__package__)

    # Turn off key repeats
    pygame.key.set_repeat()

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
                print("Quitting main loop")
                sys.stdout.flush()
                main_loop = False
                break

        elif event.type == pygame.QUIT:
            print("Quitting main loop")
            sys.stdout.flush()
            main_loop = False
            break

    q.put("QUIT")
    p.join()
    disp_q.put("QUIT")
    disp_p.join()
    pygame.display.quit()
    print("Goodbye!")


if __name__ == "__main__":
    main()
