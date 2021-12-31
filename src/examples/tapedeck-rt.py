"""

$Id: tapedeck-rt.py 925 2015-08-23 02:41:47Z weegreenblobbie $

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

import pygame
import Nsound as ns

#------------------------------------------------------------------------------
# Globals

SR = 48000.0
CHANNELS = 2
N_BUFFERS = 3
LO_T = 0.01
HI_T = 0.10 # Higher latency for non-JACK

N_THREADS = 1
WINDOW_SIZE = (600, 200)

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


def worker(q, signal, sig_dur, t):

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

    # Circular iterator

    circ = signal.cbegin()
    loc = 0.0 # location in sample

    #--------------------------------------------------------------------------
    # lookup tables

    # rate

    r_min = -3.0
    r_max = 3.0

    g = ns.Generator(1)
    gen = ns.Generator(SR)

    r_table = g.drawLine(WINDOW_SIZE[0], r_min, r_max)

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

    space = ' ' * 32

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

                playback.stop()

                state = WAITING

                sys.stdout.write("\r%s\r" % space)
                sys.stdout.flush()

                # db_info = playback.get_debug_info()
                # print "db_info =", db_info

                if DEBUG_RECORD:
                    fn = "tapedeck-rt-%02d.wav" % aid
                    aout >> fn

                    print("Wrote %s" % fn)

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

        r0 = r_table[x0]
        r1 = r_table[x1]

        r_avg = (r0 + r1) / 2.0

        fmt = "\r{space}\rrate = %5.3fx".format(space = space)

        sys.stdout.write(fmt % r_avg)
        sys.stdout.flush()

        #----------------------------------------------------------------------
        # playback

        rate = gen.drawLine(dur, r0, r1)

        temp = ns.Buffer(dur_samples)

        for i in range(dur_samples):

            x = float(circ)

            r = rate[i]

            loc += r

            loci = int(loc)

            if abs(loci) > 1:
                circ += loci
                loc -= loci

            temp << x

        temp >> playback

        if DEBUG_RECORD:
            aout << temp

        x0, y0 = x1, y1


def main():

    parser = argparse.ArgumentParser(description = 'tapedeck')

    parser.add_argument(
        '-j',
        '--jack',
        dest    = 'use_jack',
        default = False,
        action  = 'store_true',
        help    = """Use the JACK portaudio Host API, if the JACK server isn't
running, you will get undefined behavior""")

    parser.add_argument(
        'input_wave',
        help = "Input wave to process")

    args = parser.parse_args()

    # Low Latency

    if args.use_jack:
        print("Will try to use JACK (known to work on Linux)")
        ns.AudioPlaybackRt.use_jack(True)
        t = LO_T

    # High Latency
    else:
        print("Using High Latency")
        t = HI_T

    signal = ns.AudioStream(args.input_wave).getMono()
    signal.resample2(SR)
    signal.normalize()
    signal *= 0.49

    print("Hello TapeDeck RT!")
    print("Press ESC to quit.")

    q = multiprocessing.Queue(maxsize = 1)
    p = multiprocessing.Process(target=worker, args = (q, signal[0], signal.getDuration(), t))
    p.start()

    # read from queue that the process initialized and is ready.
    good = q.get()

    if not good:
        sys.exit(1)

    # Init only the display, otherwise when using JACK Audio Connection Kit,
    # pygame won't exit cleanly.
    pygame.display.init()

    # Open window
    window = pygame.display.set_mode((WINDOW_SIZE[0], WINDOW_SIZE[1]))
    pygame.display.set_caption("TapeDeck RT! %s" % ns.__package__)

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
                main_loop = False
                break

        elif event.type == pygame.QUIT:
            print("Quitting main loop")
            sys.stdout.flush()
            main_loop = False
            break

    q.put("QUIT")
    p.join()
    pygame.display.quit()
    print("Goodbye!")


if __name__ == "__main__":
    main()
