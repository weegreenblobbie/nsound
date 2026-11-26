"""

$Id: bebot.py 925 2015-08-23 02:41:47Z weegreenblobbie $

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
import time
import multiprocessing

import pygame
import Nsound as ns

import bebot_synth as synth

#------------------------------------------------------------------------------
# Globals

SR = synth.SR
CHANNELS = 1
N_BUFFERS = 3
CHUNK = 0.01 # The seconds of sound made per read of the controls

LO_T = 0.01
HI_T = 0.10 # Higher latency for non-JACK


N_THREADS = 1

REPORT_SECONDS = 5.0 # The seconds of sound between --debug reports

# The window: the square X-Y pad on the left, the panel on the right.
PAD_SIZE = 400
PANEL_WIDTH = 220

ROW_HEIGHT = 28
CHORD_BOX = pygame.Rect(PAD_SIZE + 20, 30, PANEL_WIDTH - 40, ROW_HEIGHT)
SNAP_BOX = pygame.Rect(PAD_SIZE + 20, 68, 18, 18)
ARPEGGIO_BOX = pygame.Rect(PAD_SIZE + 20, 94, 18, 18)
TEMPO_BAR = pygame.Rect(PAD_SIZE + 20, 140, PANEL_WIDTH - 40, 18)
# A button for each note value: five in a row, then the triplets below the
# notes they divide, 1/8 and 1/16.
NOTE_BOXES = tuple(
    pygame.Rect(PAD_SIZE + 10 + 40 * column, 186 + 26 * row, 38, 22)
    for row, column in [(0, i) for i in range(5)] + [(1, 3), (1, 4)])
# A slider for each chord note's level, the root's on top.
LEVEL_BARS = tuple(
    pygame.Rect(PAD_SIZE + 88, 268 + 32 * i, 84, 18)
    for i in range(synth.N_VOICES))

# The semitones above the root, named.
INTERVAL_NAMES = {
    0: "Root", 2: "2nd", 3: "Min 3rd", 4: "Maj 3rd", 5: "4th", 6: "Dim 5th",
    7: "5th", 8: "Aug 5th", 10: "Min 7th", 11: "Maj 7th"}

NOTE_NAMES = (
    "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B")

BACKGROUND = (18, 20, 28)
PANEL_COLOR = (32, 35, 46)
BOX_COLOR = (58, 63, 82)
GUIDE_COLOR = (44, 48, 64)
OCTAVE_COLOR = (80, 86, 112)
TEXT_COLOR = (220, 224, 236)
ACCENT_COLOR = (90, 200, 250)
IDLE_COLOR = (120, 126, 150)

DEBUG_RECORD = False


def print_settings(playback, n_buffers, t, chunk):
    """Prints the playback settings and the latency they add up to."""

    # getInfo() names the device, then the host API.
    names = [
        line.split(":", 1)[1].strip()
        for line in playback.getInfo().splitlines()
        if line.strip().startswith("name:")]

    device, api = (names + ["?", "?"])[:2]

    latency = playback.getActualLatencySeconds()

    print("Playing through %s, %s" % (api, device))
    print("    %g Hz, %d channel(s)" % (SR, CHANNELS))
    print("    %d buffers of %g s, chunks of %g s" % (n_buffers, t, chunk))
    print("    output latency: %.1f ms" % (1000.0 * latency))
    print("    mouse to sound, at most: chunk + buffers + output latency = %.1f ms" % (
        1000.0 * (chunk + n_buffers * t + latency)))
    sys.stdout.flush()


def print_counters(playback, seconds, render_times, chunk):
    """Prints how long chunks took to render and the playback's counters."""
    info = playback.get_debug_info()

    print(
        "%6.1f s: render ms per %g ms chunk: mean %.3f, max %.3f, late %d/%d"
        " | underrun %d, overrun %d, pa_underrun %d, pa_overrun %d,"
        " unknown %d" % (
            seconds,
            1000.0 * chunk,
            1000.0 * sum(render_times) / len(render_times),
            1000.0 * max(render_times),
            sum(used > chunk for used in render_times),
            len(render_times),
            info.underrun_count,
            info.overrun_count,
            info.pa_underrun_count,
            info.pa_overrun_count,
            info.unknown_error_count))
    sys.stdout.flush()


def short_name(host_api):
    """Returns a host API's name as typed: lower case, without "windows "."""
    return host_api.lower().removeprefix("windows ")


def full_host_api_name(name, host_apis):
    """Returns the host API in host_apis that name is short for, else name."""
    for host_api in host_apis:
        if short_name(host_api) == short_name(name):
            return host_api

    return name


def worker(ready, t, host_api, shared, quitting, n_buffers, chunk, debug):
    """The sound's process: renders and plays what the shared controls ask."""

    #--------------------------------------------------------------------------
    # Initialize

    try:
        # Here, not in main(): on Windows this process starts a new Python.
        if host_api:
            ns.AudioPlaybackRt.use_host_api(full_host_api_name(
                host_api, ns.AudioPlaybackRt.getHostApis()))

        playback = ns.AudioPlaybackRt(SR, CHANNELS, n_buffers, t)
    except:
        ready.put(False)
        sys.stderr.write("FAILURE: failed to create audio playback object!\n")
        sys.stderr.flush()
        return

    print_settings(playback, n_buffers, t, chunk)

    voices = synth.Synth()

    # Tell the main thread we succeeded in initializing our
    # nsound objects.

    ready.put(True)

    if DEBUG_RECORD:
        aout = ns.AudioStream(SR, 1)
        aid = 0

    #--------------------------------------------------------------------------
    # Sound processing loop

    # Stopping drops what is still in the buffers, so after the sound fades
    # out, this much silence pushes the end of the fade to the device.
    tail_seconds = (n_buffers + 1) * t
    silence_left = 0.0
    playing = False

    # For --debug: the seconds played since the sound started, and the time
    # each chunk took to render since the last report.
    seconds = 0.0
    render_times = []

    while not quitting.is_set():

        # All the controls at once, as the window last wrote them.
        with shared.get_lock():
            controls = synth.Controls.from_list(shared[:])

        pressed = controls.presses != voices.presses

        if not (playing or controls.held or pressed):
            time.sleep(0.002)
            continue

        # Blocks while the buffers are full.
        start = time.perf_counter()
        temp = voices.render(controls, chunk)
        used = time.perf_counter() - start
        temp >> playback
        playing = True

        if debug:
            seconds += chunk
            render_times.append(used)

            if len(render_times) * chunk >= REPORT_SECONDS:
                print_counters(playback, seconds, render_times, chunk)
                render_times = []

        if DEBUG_RECORD:
            aout << temp

        if controls.held or pressed or not voices.is_silent():
            silence_left = tail_seconds
            continue

        silence_left -= chunk

        if silence_left <= 0.0:
            if render_times:
                print_counters(playback, seconds, render_times, chunk)

            seconds = 0.0
            render_times = []

            playback.stop()
            playing = False

            if DEBUG_RECORD:
                fn = "bebot-rt-%02d.wav" % aid
                aout >> fn

                print("Wrote %s" % fn)

                aout = ns.AudioStream(SR, 1)
                aid += 1

    playback.stop()


def limit(x, y):
    """Returns the position moved onto the pad."""
    last = PAD_SIZE - 1

    return max(0, min(x, last)), max(0, min(y, last))


class Window:
    """Turns mouse events into Controls and draws them."""

    def __init__(self):
        """Starts with nothing pressed."""
        self.controls = synth.Controls()
        self.cursor = None          # The mouse's position on the pad
        self.menu_open = False      # The chord list is showing
        self.sliding = None         # The slider's bar being dragged

    def handle(self, event):
        """Updates the controls for a mouse event."""
        if event.type == pygame.MOUSEBUTTONDOWN and event.button == 1:
            self._press(event.pos)

        elif event.type == pygame.MOUSEBUTTONUP and event.button == 1:
            self.controls.held = False
            self.sliding = None

        elif event.type == pygame.MOUSEMOTION:
            if self.controls.held:
                self._move(event.pos)
            elif event.pos[0] < PAD_SIZE:
                self.cursor = limit(*event.pos)

            if self.sliding:
                self._slide(event.pos[0])

    def _press(self, pos):
        """Handles the left button going down at pos."""
        # The open chord list covers what is under it, a click closes it.
        if self.menu_open:
            for i in range(len(synth.CHORDS)):
                if self._menu_item(i).collidepoint(pos):
                    self.controls.chord = i

            self.menu_open = False

        elif pos[0] < PAD_SIZE:
            self.controls.held = True
            self.controls.presses += 1
            self._move(pos)

        elif CHORD_BOX.collidepoint(pos):
            self.menu_open = True

        # The check boxes' labels toggle them too.
        elif self._row(SNAP_BOX).collidepoint(pos):
            self.controls.snap = not self.controls.snap

        elif self._row(ARPEGGIO_BOX).collidepoint(pos):
            self.controls.arpeggio = not self.controls.arpeggio

        else:
            for i, box in enumerate(NOTE_BOXES):
                if box.collidepoint(pos):
                    self.controls.note_value = i

            # Only the chord's notes have sliders.
            n_notes = len(synth.CHORDS[self.controls.chord][1])

            for bar in (TEMPO_BAR,) + LEVEL_BARS[:n_notes]:
                if bar.collidepoint(pos):
                    self.sliding = bar
                    self._slide(pos[0])

    def _move(self, pos):
        """Moves the note to pos, clamped to the pad."""
        x, y = limit(*pos)
        self.cursor = x, y
        self.controls.x = x / (PAD_SIZE - 1)
        self.controls.y = 1.0 - y / (PAD_SIZE - 1)

    def _slide(self, x):
        """Sets the dragged slider's value from the position x."""
        amount = (x - self.sliding.left) / (self.sliding.width - 1)
        amount = max(0.0, min(amount, 1.0))

        if self.sliding == TEMPO_BAR:
            # Whole beats per minute.
            self.controls.tempo = float(round(synth.mix(
                synth.LOW_TEMPO, synth.HIGH_TEMPO, amount)))
        else:
            note = LEVEL_BARS.index(self.sliding) + 1
            setattr(self.controls, "level%d" % note, amount)

    @staticmethod
    def _row(box):
        """Returns the area of a check box and its label."""
        return pygame.Rect(box.left, box.top, PANEL_WIDTH - 40, box.height)

    @staticmethod
    def _menu_item(i):
        """Returns the area of the chord list's item i."""
        return CHORD_BOX.move(0, (i + 1) * CHORD_BOX.height)

    def draw(self, surface, font):
        """Draws the pad and the panel."""
        surface.fill(BACKGROUND, (0, 0, PAD_SIZE, PAD_SIZE))
        surface.fill(PANEL_COLOR, (PAD_SIZE, 0, PANEL_WIDTH, PAD_SIZE))

        if self.controls.snap:
            self._draw_guides(surface)

        if self.cursor:
            self._draw_cursor(surface, font)

        self._draw_panel(surface, font)

    def _draw_guides(self, surface):
        """Draws a line at each semitone, brighter at each C."""
        span = synth.HIGH_NOTE - synth.LOW_NOTE

        for note in synth.SEMITONES:
            height = (note - synth.LOW_NOTE) / span
            y = round((1.0 - height) * (PAD_SIZE - 1))
            color = OCTAVE_COLOR if note % 12 == 0 else GUIDE_COLOR
            pygame.draw.line(surface, color, (0, y), (PAD_SIZE - 1, y))

    def _draw_cursor(self, surface, font):
        """Draws the crosshair, and while held the root note."""
        x, y = self.cursor
        color = ACCENT_COLOR if self.controls.held else IDLE_COLOR

        pygame.draw.line(surface, color, (x, 0), (x, PAD_SIZE - 1))
        pygame.draw.line(surface, color, (0, y), (PAD_SIZE - 1, y))
        pygame.draw.circle(surface, color, (x, y), 8, 2)

        if self.controls.held:
            note = synth.root_note(self.controls.y, self.controls.snap)
            nearest = round(note)
            text = "%s%d  %.1f Hz" % (
                NOTE_NAMES[nearest % 12],
                nearest // 12 - 1,
                synth.note_frequency(note))
            surface.blit(font.render(text, True, TEXT_COLOR), (8, 8))

    def _draw_panel(self, surface, font):
        """Draws the chord selector, the check boxes and the sliders."""

        def write(text, pos):
            surface.blit(font.render(text, True, TEXT_COLOR), pos)

        write("Chord", (CHORD_BOX.left, 10))
        pygame.draw.rect(surface, BOX_COLOR, CHORD_BOX)
        write(
            synth.CHORDS[self.controls.chord][0],
            (CHORD_BOX.left + 8, CHORD_BOX.top + 7))
        pygame.draw.polygon(surface, TEXT_COLOR, [
            (CHORD_BOX.right - 18, CHORD_BOX.top + 11),
            (CHORD_BOX.right - 8, CHORD_BOX.top + 11),
            (CHORD_BOX.right - 13, CHORD_BOX.top + 17)])

        for box, checked, label in (
                (SNAP_BOX, self.controls.snap, "Snap to Semitones"),
                (ARPEGGIO_BOX, self.controls.arpeggio, "Arpeggio")):
            pygame.draw.rect(surface, BOX_COLOR, box)

            if checked:
                pygame.draw.rect(surface, ACCENT_COLOR, box.inflate(-8, -8))

            write(label, (box.right + 8, box.top + 2))

        write(
            "Tempo: %.0f BPM" % self.controls.tempo,
            (TEMPO_BAR.left, TEMPO_BAR.top - 20))
        self._draw_bar(
            surface,
            TEMPO_BAR,
            (self.controls.tempo - synth.LOW_TEMPO)
                / (synth.HIGH_TEMPO - synth.LOW_TEMPO))

        write("Note value", (TEMPO_BAR.left, NOTE_BOXES[0].top - 20))

        for i, (box, (name, _)) in enumerate(
                zip(NOTE_BOXES, synth.NOTE_VALUES)):
            chosen = i == self.controls.note_value
            pygame.draw.rect(
                surface, ACCENT_COLOR if chosen else BOX_COLOR, box)
            text = font.render(
                name, True, BACKGROUND if chosen else TEXT_COLOR)
            surface.blit(text, text.get_rect(center=box.center))

        write("Note levels", (TEMPO_BAR.left, LEVEL_BARS[0].top - 22))

        offsets = synth.CHORDS[self.controls.chord][1]

        for bar, offset, level in zip(
                LEVEL_BARS, offsets, self.controls.levels()):
            write(INTERVAL_NAMES[offset], (TEMPO_BAR.left, bar.top + 2))
            self._draw_bar(surface, bar, level)
            write("%.0f%%" % (100.0 * level), (bar.right + 6, bar.top + 2))

        if self.menu_open:
            for i, (name, _) in enumerate(synth.CHORDS):
                item = self._menu_item(i)
                chosen = i == self.controls.chord
                pygame.draw.rect(
                    surface, OCTAVE_COLOR if chosen else BOX_COLOR, item)
                write(name, (item.left + 8, item.top + 7))

    @staticmethod
    def _draw_bar(surface, bar, amount):
        """Draws a slider filled to amount, 0 to 1."""
        pygame.draw.rect(surface, BOX_COLOR, bar)

        filled = bar.copy()
        filled.width = round(amount * bar.width)
        pygame.draw.rect(surface, ACCENT_COLOR, filled)


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

    parser.add_argument(
        '--host-api',
        dest    = 'host_api',
        default = None,
        help    = """The portaudio host API to play through, with low latency,
e.g. wasapi, wdm-ks or mme.  ns_playback lists them, "windows " can be left
off and case doesn't matter""")

    parser.add_argument(
        '--buffer',
        dest    = 'buffer_seconds',
        default = None,
        type    = float,
        help    = """The seconds of audio in each buffer""")

    parser.add_argument(
        '--buffers',
        dest    = 'n_buffers',
        default = N_BUFFERS,
        type    = int,
        help    = """How many buffers the sound can be ahead of what is heard,
%d by default""" % N_BUFFERS)

    parser.add_argument(
        '--chunk',
        dest    = 'chunk',
        default = CHUNK,
        type    = float,
        help    = """The seconds of sound made per read of the controls, %g by
default: smaller follows the mouse sooner, too small crackles""" % CHUNK)

    parser.add_argument(
        '--debug',
        dest    = 'debug',
        default = False,
        action  = 'store_true',
        help    = """Print the render times and playback counters every %g s of
sound and at each release""" % REPORT_SECONDS)

    args = parser.parse_args()

    # Low latency

    if args.use_jack:
        print("Will try to use JACK (known to work on Linux)")
        ns.AudioPlaybackRt.use_jack(True)
        t = LO_T

    elif args.host_api:
        print("Using %s with low latency" % args.host_api)
        t = LO_T

    # High latency
    else:
        print("Using Higher Latency")
        t = HI_T

    if args.buffer_seconds:
        t = args.buffer_seconds

    print("Hello Bebot!")
    print("Press ESC to quit.")

    window = Window()

    # The controls are shared, written and read all together under the lock:
    # the worker reads the newest when it needs them, it doesn't fall behind
    # a queue of old ones.
    shared = multiprocessing.Array("d", window.controls.to_list())
    quitting = multiprocessing.Event()
    ready = multiprocessing.Queue()
    p = multiprocessing.Process(
        target=worker,
        args = (
            ready, t, args.host_api, shared, quitting, args.n_buffers,
            args.chunk, args.debug))
    p.start()

    # read from queue that the process initialized and is ready.
    good = ready.get()

    if not good:
        sys.exit(1)

    # Init only the display and fonts, otherwise when using JACK Audio
    # Connection Kit, pygame won't exit cleanly.
    pygame.display.init()
    pygame.font.init()

    # Open window
    screen = pygame.display.set_mode((PAD_SIZE + PANEL_WIDTH, PAD_SIZE))
    pygame.display.set_caption("Bebot! %s" % ns.__package__)
    font = pygame.font.Font(None, 22)

    # Turn off key repeats
    pygame.key.set_repeat()

    main_loop = True

    while main_loop:

        window.draw(screen, font)
        pygame.display.flip()

        # Handle every waiting event before drawing again.
        for event in [pygame.event.wait()] + pygame.event.get():

            if event.type == pygame.KEYDOWN:

                # Break out of loop
                if event.key == pygame.K_ESCAPE:
                    main_loop = False

            elif event.type == pygame.QUIT:
                print("Quitting main loop")
                sys.stdout.flush()
                main_loop = False

            else:
                window.handle(event)

                # Each event, so a quick press and release is seen.
                with shared.get_lock():
                    shared[:] = window.controls.to_list()

    quitting.set()
    p.join()
    pygame.quit()
    print("Goodbye!")


if __name__ == "__main__":
    main()
