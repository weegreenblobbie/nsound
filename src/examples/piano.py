"""

$Id: piano.py 852 2014-06-29 22:13:08Z weegreenblobbie $

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

import Queue as queue
import argparse
import multiprocessing
import sys
import time

import pygame
import Nsound as ns

#------------------------------------------------------------------------------
# Globals

N_THREADS = 3

SR = 48000.0
CHANNELS = 1
N_BUFFERS = 10
T = 0.01

# Higher latency on non-Linux platforms
if 'linux' not in sys.platform:
    T = 0.05


N_BUFFERS = 3

# worker state machine
enum = 0

WAITING = enum ; enum += 1
PLAYING = enum ; enum += 1
RELEASING = enum; enum += 1

SINE   = enum; enum += 1
SAW    = enum; enum += 1
SQUARE = enum; enum += 1

# Random seed for chorus
#~rng = ns.RngTausworthe()
#~seed = rng.get()
#~print "seed = ", seed

seed = 1108683026


def worker(q, generator, n_harmonics, chorus):

    #--------------------------------------------------------------------------
    # Initialize

    if generator == SINE:
        gen = ns.Sine(SR)

    elif generator == SAW:
        gen = ns.Sawtooth(SR, n_harmonics)

    elif generator == SQUARE:
        gen = ns.Square(SR, n_harmonics)

    else:
        raise RuntimeError("Unknown generator %d" % generator)

    if chorus > 1:
        # Need to use same random number geneator accross workers.
        rng = gen.getRandomNumberGenerator()
        rng.setSeed(seed)
        gen.setChorus(chorus, 0.01)

    gen.setRealtime(True)

    try:
        playback = ns.AudioPlaybackRt(SR, CHANNELS, N_BUFFERS, T)
        #playback.setBufferUnderrunMode(ns.BUM_NOISE)
    except:
        q.put(False)
        sys.stderr.write("FAILURE: failed to create audio playback object!\n")
        sys.stderr.flush()
        return

    # Sharper envelope
    env = ns.EnvelopeAdsr(SR, 0.2, 0.4, 0.333, 0.1)

    # Tell the main thread we succeeded in initializing our
    # nsound objects.

    q.put(True)

    while q.full():
        pass

    key_on = None
    state = WAITING

    dur = T

    N_SAMPLES = int(dur * SR + 0.5)

    #--------------------------------------------------------------------------
    # Sound processing loop

    process_key = False

    while True:

        try:
            x = q.get(False)
            process_key = True
        except queue.Empty:
            process_key = False

        if process_key:

            if x == "QUIT":
                playback.stop()
                return

            if state == WAITING:
                freq = x
                key_on = True
                state = PLAYING

            elif state == PLAYING:
                key_on = False
                state = RELEASING

        if state == WAITING:
            continue

        for i in xrange(N_SAMPLES):
            sample = env.shape(0.333 * gen.generate(freq), key_on)
            playback.play(sample)

        # Finished? state transition
        if state == RELEASING:
            if env.is_done():
                state = WAITING
                playback.stop()
                gen.reset()
                env.reset()


def main():

    parser = argparse.ArgumentParser(description = 'Simple piano.')

    parser.add_argument(
        '--chorus',
        dest    = 'CHORUS',
        default = 0,
        type    = int,
        help    = 'Add chorus with N voices')

    parser.add_argument(
        '--saw',
        dest    = 'USE_SAW',
        action  = 'store_true',
        default = False,
        help    = 'Generate Sawtooth waves')

    parser.add_argument(
        '--square',
        dest    = 'USE_SQUARE',
        action  = 'store_true',
        default = False,
        help    = 'Generate Square waves')

    parser.add_argument(
        '--n',
        dest    = 'N_HARMONICS',
        type    = int,
        default = None,
        help    = """Specify the number of harmonics for Sawtooth or Square
generators (default is 7)""")

    parser.add_argument(
        '-j',
        '--jack',
        dest    = 'use_jack',
        default = False,
        action  = 'store_true',
        help    = """Use the JACK portaudio Host API, if the JACK server isn't
running, you will get undefined behavior""")

    args = parser.parse_args()

    if args.use_jack:
        print "Will try to use JACK (known to work on Linux)"
        ns.AudioPlaybackRt.use_jack(True)

    generator = SINE
    n_harmonics = 7
    chorus = args.CHORUS

    if args.USE_SAW:
        generator = SAW

    elif args.USE_SQUARE:
        generator = SQUARE

    if args.N_HARMONICS is not None:
        if args.N_HARMONICS <= 0:
            raise ValueError("N harmonics must be >= 1")

        n_harmonics = args.N_HARMONICS

    # Mapy keys to frequencies

    key_to_freq = {
        pygame.K_z: 261.626,
        pygame.K_s: 277.183,
        pygame.K_x: 293.665,
        pygame.K_d: 311.127,
        pygame.K_c: 329.628,
        pygame.K_v: 349.228,
        pygame.K_g: 369.994,
        pygame.K_b: 391.995,
        pygame.K_h: 415.305,
        pygame.K_n: 440.000,
        pygame.K_j: 466.164,
        pygame.K_m: 493.883,
    }

    q_pool = []
    q_idle = set()
    p_pool = []

    for i in xrange(N_THREADS):

        q = multiprocessing.Queue(maxsize = 1)
        p = multiprocessing.Process(
            target = worker,
            args = (q, generator, n_harmonics, chorus) )
        p.start()

        # read from queue that the process initialized and is ready.
        good = q.get()

        if not good:
            sys.exit(1)

        q_idle.add(q)
        q_pool.append(q)
        p_pool.append(p)

    key_to_queue = {}

    # Init only the display, otherwise when using JACK Audio Connection Kit,
    # pygame won't exit cleanly.
    pygame.display.init()

    # Open window
    window = pygame.display.set_mode((400, 100))
    pygame.display.set_caption("Piano! %s" % ns.__package__)

    # Turn off key repeats
    pygame.key.set_repeat()

    print "Hello Piano!"
    print "Press ESC to quit."
    print "Press [z,s,x,d,c,v,g,b,h,n,j,m] to play a note."

    main_loop = True

    while main_loop:

        event = pygame.event.wait()

        if event.type == pygame.KEYDOWN:

            c = event.key

            # Break out of loop
            if c in [pygame.K_ESCAPE]:
                main_loop = False
                break

            if c in key_to_freq:

                if len(q_idle) == 0:
                    print "no worker thread available!"
                    continue

                freq = key_to_freq[c]

                q = q_idle.pop()

                key_to_queue[c] = q

                q.put(freq)

                sys.stdout.write("+")
                sys.stdout.flush()

        elif event.type == pygame.KEYUP:

            c = event.key

            if c in key_to_queue:

                q = key_to_queue[c]

                q.put(None)

                del key_to_queue[c]

                q_idle.add(q)
                sys.stdout.write("-")
                sys.stdout.flush()

        elif event.type == pygame.QUIT:
            main_loop = False
            break

    for q in q_pool:
        q.put("QUIT")

    for p in p_pool:
        p.join()

    pygame.display.quit()
    print ""
    print "Goodbye!"


if __name__ == "__main__":
    main()
