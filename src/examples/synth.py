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
import math
import sys
import time
import multiprocessing
import queue

import pygame
import Nsound as ns

import synth_engine as engine
import synth_instruments as instruments

#------------------------------------------------------------------------------
# Globals

SR = engine.SR
CHANNELS = 2
N_BUFFERS = 3
CHUNK = 0.01 # The seconds of sound made per read of the controls

LO_T = 0.01
HI_T = 0.10 # Higher latency for non-JACK


N_THREADS = 1

REPORT_SECONDS = 5.0 # The seconds of sound between --debug reports

# The notes the renderer is asked for at a time, the rest are asked for again
# if still wanted, so a moving pad doesn't queue up old ones.
MAX_ASKED = engine.N_VOICES

# The values a rendered instrument's control takes from the X axis: each is a
# note to render.
X_STEPS = 20

# The window: the square X-Y pad on the left, then the panel's four columns,
# the instrument's own controls, the notes all instruments share, their
# envelope and filter, and the chorus.
PAD_SIZE = 400
PANEL_WIDTH = 220
GLOBAL_LEFT = PAD_SIZE + PANEL_WIDTH
EFFECT_LEFT = GLOBAL_LEFT + PANEL_WIDTH
CHORUS_LEFT = EFFECT_LEFT + PANEL_WIDTH
WINDOW_WIDTH = CHORUS_LEFT + PANEL_WIDTH

ROW_HEIGHT = 28
INSTRUMENT_BOX = pygame.Rect(PAD_SIZE + 20, 30, PANEL_WIDTH - 40, ROW_HEIGHT)
# A slider for each of the instrument's own controls.
PARAMETER_BARS = tuple(
    pygame.Rect(PAD_SIZE + 20, 92 + 52 * i, PANEL_WIDTH - 40, 18)
    for i in range(instruments.MAX_PARAMETERS))
CHORD_BOX = pygame.Rect(GLOBAL_LEFT + 20, 30, PANEL_WIDTH - 40, ROW_HEIGHT)
SNAP_BOX = pygame.Rect(GLOBAL_LEFT + 20, 68, 18, 18)
ARPEGGIO_BOX = pygame.Rect(GLOBAL_LEFT + 20, 94, 18, 18)
TEMPO_BAR = pygame.Rect(GLOBAL_LEFT + 20, 140, PANEL_WIDTH - 40, 18)
# A button for each note value: five in a row, then the triplets below the
# notes they divide, 1/8 and 1/16.
NOTE_BOXES = tuple(
    pygame.Rect(GLOBAL_LEFT + 10 + 40 * column, 186 + 26 * row, 38, 22)
    for row, column in [(0, i) for i in range(5)] + [(1, 3), (1, 4)])
# A slider for each chord note's level, the root's on top.
LEVEL_BARS = tuple(
    pygame.Rect(GLOBAL_LEFT + 82, 268 + 32 * i, 68, 18)
    for i in range(engine.N_VOICES))

# The envelope's, filter's and chorus's sliders: Controls field, label, ends, and
# how the values spread along the slider: evenly, in whole numbers ("whole"),
# finer at the low end ("square"), or each octave as wide ("octaves").
EFFECTS = (
    ("attack", "Attack", 0.0, engine.LONGEST_ATTACK, "square"),
    ("decay", "Decay", 0.0, engine.LONGEST_DECAY, "square"),
    ("sustain", "Sustain", 0.0, 1.0, "even"),
    ("release", "Release", engine.SHORTEST_RAMP, engine.LONGEST_RELEASE,
        "square"),
    ("cutoff", "Cutoff", engine.LOW_CUTOFF, engine.HIGH_CUTOFF, "octaves"),
    ("resonance", "Resonance", 0.0, engine.MAX_RESONANCE, "even"),
    ("chorus_voices", "Voices", 1, 4, "whole"),
    ("chorus_detune", "Detune", 0.0, 50.0, "even"),
    ("chorus_rate", "Rate", 0.1, 5.0, "square"),
    ("chorus_mix", "Mix", 0.0, 1.0, "even"),
    ("chorus_width", "Stereo width", 0.0, 1.0, "even"),
)
EFFECT_BARS = tuple(
    pygame.Rect(EFFECT_LEFT + 20, top, PANEL_WIDTH - 40, 18)
    for top in (52, 96, 140, 184, 250, 294)) + tuple(
    pygame.Rect(CHORUS_LEFT + 20, 92 + 52 * i, PANEL_WIDTH - 40, 18)
    for i in range(5))
NORMALIZE_BOX = pygame.Rect(EFFECT_LEFT + 20, 338, 18, 18)
CHORUS_BOX = pygame.Rect(CHORUS_LEFT + 20, 38, 18, 18)

SLIDERS = (TEMPO_BAR,) + LEVEL_BARS + PARAMETER_BARS + EFFECT_BARS


def x_box(bar):
    """Returns the box that has the pad's X axis set a slider."""
    if bar in LEVEL_BARS:
        return pygame.Rect(GLOBAL_LEFT + 196, bar.top + 2, 14, 14)

    # At the end of the label's line.
    return pygame.Rect(bar.right - 14, bar.top - 19, 14, 14)


RESET_BOX = pygame.Rect(PAD_SIZE + 20, 354, PANEL_WIDTH - 40, ROW_HEIGHT)

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


def render_notes(requests, results):
    """The note renderer's process: renders each note asked for until None."""
    # Quitting doesn't wait for notes nobody will read.
    results.cancel_join_thread()

    for key in iter(requests.get, None):
        note = instruments.render_note(key)
        channels = [samples.toList() for samples in note.channels]
        results.put((key, channels, note.loop_start))


class BackgroundNotes(instruments.NoteCache):
    """Notes rendered by render_notes(), so the sound doesn't wait for them."""

    def __init__(self, requests, results):
        """Asks for notes on requests, gets them from results."""
        super().__init__()
        self.requests = requests
        self.results = results
        self.asked = set()

    def get(self, key):
        """Returns the note, or None while it is being rendered."""
        while True:
            try:
                done, channels, loop_start = self.results.get_nowait()
            except queue.Empty:
                break

            self.asked.discard(done)
            channels = tuple(ns.Buffer(samples) for samples in channels)
            self.put(done, instruments.Note(channels, loop_start))

        return super().get(key)

    def request(self, key):
        """Asks the renderer for a note, once, if it isn't too busy."""
        if key not in self.asked and len(self.asked) < MAX_ASKED:
            self.asked.add(key)
            self.requests.put(key)


def worker(
        ready, t, host_api, shared, quitting, n_buffers, chunk, debug,
        requests,
        results):
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

    voices = engine.Synth(BackgroundNotes(requests, results))

    # Tell the main thread we succeeded in initializing our
    # nsound objects.

    ready.put(True)

    if DEBUG_RECORD:
        aout = ns.AudioStream(SR, CHANNELS)
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
            controls = engine.Controls.from_list(shared[:])

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
                fn = "synth-rt-%02d.wav" % aid
                aout >> fn

                print("Wrote %s" % fn)

                aout = ns.AudioStream(SR, CHANNELS)
                aid += 1

    playback.stop()


def effect_value(effect, amount):
    """Returns an EFFECTS value at amount, 0 to 1, along its slider."""
    _, _, low, high, spread = effect

    if spread == "octaves":
        return low * (high / low) ** amount

    if spread == "square":
        amount *= amount

    if spread == "whole":
        return round(engine.mix(low, high, amount))

    return engine.mix(low, high, amount)


def effect_amount(effect, value):
    """Returns where along its slider an EFFECTS value is, 0 to 1."""
    _, _, low, high, spread = effect

    if spread == "octaves":
        return math.log(value / low) / math.log(high / low)

    amount = (value - low) / (high - low)

    if spread == "square":
        return amount ** 0.5

    return amount


def limit(x, y):
    """Returns the position moved onto the pad."""
    last = PAD_SIZE - 1

    return max(0, min(x, last)), max(0, min(y, last))


class Window:
    """Turns mouse events into Controls and draws them."""

    def __init__(self):
        """Starts with nothing pressed, every setting at its default."""
        self.controls = engine.Controls()
        self.cursor = None          # The mouse's position on the pad
        self.menu_box = None        # The box whose list is showing
        self.sliding = None         # The slider's bar being dragged
        self.reset()

    def reset(self):
        """Puts every setting back to its default, keeps the instrument."""
        self.controls = engine.Controls(
            instrument=self.controls.instrument,
            presses=self.controls.presses)

        every = range(len(instruments.INSTRUMENTS))

        # Whether each instrument snaps to semitones.
        self.snaps = [spec.snap for spec in instruments.INSTRUMENTS]

        # Each instrument's own controls, kept while another is chosen.
        self.values = [instruments.default_values(i) for i in every]

        # The bar of the slider each instrument's X axis sets, or None.
        self.x_bars = [
            None if spec.x_parameter is None
            else PARAMETER_BARS[spec.x_parameter]
            for spec in instruments.INSTRUMENTS]

        self._choose(INSTRUMENT_BOX, self.controls.instrument)

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
        # An open list covers what is under it, a click closes it.
        if self.menu_box:
            for i in range(len(self._menu_names(self.menu_box))):
                if self._menu_item(self.menu_box, i).collidepoint(pos):
                    self._choose(self.menu_box, i)

            self.menu_box = None

        elif pos[0] < PAD_SIZE:
            self.controls.held = True
            self.controls.presses += 1
            self._move(pos)

        elif INSTRUMENT_BOX.collidepoint(pos):
            self.menu_box = INSTRUMENT_BOX

        elif CHORD_BOX.collidepoint(pos):
            self.menu_box = CHORD_BOX

        # The check boxes' labels toggle them too.
        elif self._row(SNAP_BOX).collidepoint(pos):
            self.controls.snap = not self.controls.snap
            self.snaps[self.controls.instrument] = self.controls.snap

        elif self._row(ARPEGGIO_BOX).collidepoint(pos):
            self.controls.arpeggio = not self.controls.arpeggio

        elif RESET_BOX.collidepoint(pos):
            self.reset()

        elif self._row(NORMALIZE_BOX).collidepoint(pos):
            self.controls.normalize = not self.controls.normalize

        elif self._row(CHORUS_BOX).collidepoint(pos):
            self.controls.chorus = not self.controls.chorus

        else:
            for i, box in enumerate(NOTE_BOXES):
                if box.collidepoint(pos):
                    self.controls.note_value = i

            for i, box in self._toggles():
                if self._row(box).collidepoint(pos):
                    checked = self.controls.values()[i]
                    self._set(PARAMETER_BARS[i], 0.0 if checked else 1.0)

            for bar in self._sliders():
                if x_box(bar).collidepoint(pos):
                    self._toggle_x(bar)

                elif bar.collidepoint(pos):
                    self.sliding = bar
                    self._slide(pos[0])

    def _sliders(self):
        """Returns the bars of the sliders showing."""
        # Only the chord's notes and the instrument's controls have them.
        n_notes = len(engine.CHORDS[self.controls.chord][1])
        parameters = tuple(
            bar for bar, parameter in zip(
                PARAMETER_BARS, self._instrument().parameters)
            if not parameter.toggle)

        return ((TEMPO_BAR,) + LEVEL_BARS[:n_notes] + parameters
            + EFFECT_BARS)

    def _toggles(self):
        """Returns (index, box) of the instrument's own check boxes."""
        return [
            (i, pygame.Rect(
                PARAMETER_BARS[i].left, PARAMETER_BARS[i].top - 14, 18, 18))
            for i, parameter in enumerate(self._instrument().parameters)
            if parameter.toggle]

    def _toggle_x(self, bar):
        """Has the X axis set bar's slider, or none if it already does."""
        instrument = self.controls.instrument

        if self.x_bars[instrument] == bar:
            self.x_bars[instrument] = None
        else:
            self.x_bars[instrument] = bar

    def _move(self, pos):
        """Moves the note to pos, clamped to the pad."""
        x, y = limit(*pos)
        self.cursor = x, y
        self.controls.y = 1.0 - y / (PAD_SIZE - 1)

        bar = self.x_bars[self.controls.instrument]

        if bar in self._sliders():
            steps = 100

            if self._instrument().render and bar in PARAMETER_BARS:
                steps = X_STEPS

            self._set(bar, x / (PAD_SIZE - 1), steps)

    def _slide(self, x):
        """Sets the dragged slider's value from the position x."""
        amount = (x - self.sliding.left) / (self.sliding.width - 1)
        self._set(self.sliding, max(0.0, min(amount, 1.0)))

    def _set(self, bar, amount, steps=100):
        """Sets bar's slider to amount, 0 to 1, of its range."""
        if bar == TEMPO_BAR:
            # Whole beats per minute.
            self.controls.tempo = float(round(engine.mix(
                engine.LOW_TEMPO, engine.HIGH_TEMPO, amount)))
        elif bar in LEVEL_BARS:
            note = LEVEL_BARS.index(bar) + 1
            setattr(self.controls, "level%d" % note, amount)
        elif bar in EFFECT_BARS:
            effect = EFFECTS[EFFECT_BARS.index(bar)]
            setattr(self.controls, effect[0], effect_value(effect, amount))
        else:
            i = PARAMETER_BARS.index(bar)
            parameter = self._instrument().parameters[i]

            # In steps of the range, so going back finds the same rendered
            # notes.
            value = engine.mix(
                parameter.low, parameter.high,
                round(amount * steps) / steps)

            if parameter.names or parameter.whole:
                value = float(round(value))

            self.values[self.controls.instrument][i] = value
            setattr(self.controls, "param%d" % (i + 1), value)

    def _instrument(self):
        """Returns the chosen instrument."""
        return instruments.INSTRUMENTS[self.controls.instrument]

    def _choose(self, box, i):
        """Chooses item i of the list under box."""
        if box == CHORD_BOX:
            self.controls.chord = i
            return

        self.controls.instrument = i
        self.controls.snap = self.snaps[i]
        (self.controls.param1,
         self.controls.param2,
         self.controls.param3,
         self.controls.param4) = self.values[i]

    @staticmethod
    def _menu_names(box):
        """Returns the names in the list under box."""
        if box == CHORD_BOX:
            return [name for name, _ in engine.CHORDS]

        return [instrument.name for instrument in instruments.INSTRUMENTS]

    @staticmethod
    def _row(box):
        """Returns the area of a check box and its label."""
        return pygame.Rect(box.left, box.top, PANEL_WIDTH - 40, box.height)

    @staticmethod
    def _menu_item(box, i):
        """Returns the area of item i of the list under box."""
        return box.move(0, (i + 1) * box.height)

    def draw(self, surface, font):
        """Draws the pad and the panel."""
        surface.fill(BACKGROUND, (0, 0, PAD_SIZE, PAD_SIZE))
        surface.fill(PANEL_COLOR, (PAD_SIZE, 0, 4 * PANEL_WIDTH, PAD_SIZE))

        if self.controls.snap:
            self._draw_guides(surface)

        if self.cursor:
            self._draw_cursor(surface, font)

        self._draw_panel(surface, font)
        self._draw_instrument(surface, font)
        self._draw_effects(surface, font)
        self._draw_x_boxes(surface, font)

        if self.menu_box:
            self._draw_menu(surface, font)

    def _draw_guides(self, surface):
        """Draws a line at each semitone, brighter at each C."""
        instrument = self.controls.instrument
        span = engine.high_note(instrument) - engine.LOW_NOTE

        for note in engine.semitones(instrument):
            height = (note - engine.LOW_NOTE) / span
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
            note = engine.root_note(
                self.controls.y, self.controls.snap, self.controls.instrument)
            nearest = round(note)
            text = "%s%d  %.1f Hz" % (
                NOTE_NAMES[nearest % 12],
                nearest // 12 - 1,
                engine.note_frequency(note))
            surface.blit(font.render(text, True, TEXT_COLOR), (8, 8))

    def _draw_panel(self, surface, font):
        """Draws the chord selector, the check boxes and the sliders."""

        def write(text, pos):
            surface.blit(font.render(text, True, TEXT_COLOR), pos)

        write("Chord", (CHORD_BOX.left, 10))
        self._draw_selector(
            surface, font, CHORD_BOX, engine.CHORDS[self.controls.chord][0])

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
            (self.controls.tempo - engine.LOW_TEMPO)
                / (engine.HIGH_TEMPO - engine.LOW_TEMPO))

        write("Note value", (TEMPO_BAR.left, NOTE_BOXES[0].top - 20))

        for i, (box, (name, _)) in enumerate(
                zip(NOTE_BOXES, engine.NOTE_VALUES)):
            chosen = i == self.controls.note_value
            pygame.draw.rect(
                surface, ACCENT_COLOR if chosen else BOX_COLOR, box)
            text = font.render(
                name, True, BACKGROUND if chosen else TEXT_COLOR)
            surface.blit(text, text.get_rect(center=box.center))

        write("Note levels", (TEMPO_BAR.left, LEVEL_BARS[0].top - 22))

        offsets = engine.CHORDS[self.controls.chord][1]

        for bar, offset, level in zip(
                LEVEL_BARS, offsets, self.controls.levels()):
            write(INTERVAL_NAMES[offset], (TEMPO_BAR.left, bar.top + 2))
            self._draw_bar(surface, bar, level)
            write("%.0f%%" % (100.0 * level), (bar.right + 6, bar.top + 2))

    def _draw_instrument(self, surface, font):
        """Draws the instrument selector and the instrument's controls."""

        def write(text, pos):
            surface.blit(font.render(text, True, TEXT_COLOR), pos)

        instrument = self._instrument()

        write("Instrument", (INSTRUMENT_BOX.left, 10))
        self._draw_selector(surface, font, INSTRUMENT_BOX, instrument.name)

        # The Granulator's rate slider is unused while its rate follows the
        # pitch.
        follows = instrument.granular and self.controls.param2

        for i, (bar, parameter, value) in enumerate(zip(
                PARAMETER_BARS, instrument.parameters,
                self.controls.values())):
            if parameter.toggle:
                continue

            shown = "%.4g" % value
            color = ACCENT_COLOR

            if parameter.names:
                shown = parameter.names[int(value)]

            if follows and i == 0:
                shown = "follows pitch"
                color = IDLE_COLOR

            write(
                "%s: %s" % (parameter.label, shown),
                (bar.left, bar.top - 20))
            self._draw_bar(
                surface,
                bar,
                (value - parameter.low) / (parameter.high - parameter.low),
                color)

        for i, box in self._toggles():
            pygame.draw.rect(surface, BOX_COLOR, box)

            if self.controls.values()[i]:
                pygame.draw.rect(surface, ACCENT_COLOR, box.inflate(-8, -8))

            write(instrument.parameters[i].label, (box.right + 8, box.top + 2))

    def _draw_effects(self, surface, font):
        """Draws the envelope's, filter's and chorus's controls and the
        normalize box.
        """

        def write(text, pos):
            surface.blit(font.render(text, True, TEXT_COLOR), pos)

        write("Envelope", (EFFECT_LEFT + 20, 10))
        write("Filter", (EFFECT_LEFT + 20, 210))
        write("Chorus", (CHORUS_LEFT + 20, 10))

        # An instrument's own envelope leaves the envelope's sliders unused.
        envelope = ("attack", "decay", "sustain", "release")
        own = engine.envelope_settings(self.controls) == (
            self._instrument().envelope)

        for bar, effect in zip(EFFECT_BARS, EFFECTS):
            field, label = effect[:2]
            value = getattr(self.controls, field)
            color = ACCENT_COLOR

            # It shows the instrument's value, and keeps its own for later.
            if own and field in envelope:
                value = self._instrument().envelope[envelope.index(field)]
                color = IDLE_COLOR

            if field == "cutoff":
                shown = "%.0f Hz" % value
            elif field in (
                    "sustain", "resonance", "chorus_mix", "chorus_width"):
                shown = "%.0f%%" % (100.0 * value)
            elif field == "chorus_voices":
                shown = "%d" % value
            elif field == "chorus_detune":
                shown = "%.0f cents" % value
            elif field == "chorus_rate":
                shown = "%.2f Hz" % value
            else:
                shown = "%.3f s" % value

            write("%s: %s" % (label, shown), (bar.left, bar.top - 20))
            self._draw_bar(
                surface, bar, effect_amount(effect, value), color)

        pygame.draw.rect(surface, BOX_COLOR, NORMALIZE_BOX)

        if self.controls.normalize:
            pygame.draw.rect(
                surface, ACCENT_COLOR, NORMALIZE_BOX.inflate(-8, -8))

        write(
            "Normalize volume",
            (NORMALIZE_BOX.right + 8, NORMALIZE_BOX.top + 2))

        pygame.draw.rect(surface, BOX_COLOR, CHORUS_BOX)

        if self.controls.chorus:
            pygame.draw.rect(
                surface, ACCENT_COLOR, CHORUS_BOX.inflate(-8, -8))

        write("On", (CHORUS_BOX.right + 8, CHORUS_BOX.top + 2))

    def _draw_x_boxes(self, surface, font):
        """Draws the sliders' X axis boxes and the reset button."""

        def centered(text, color, box):
            image = font.render(text, True, color)
            surface.blit(image, image.get_rect(center=box.center))

        for bar in self._sliders():
            box = x_box(bar)
            chosen = self.x_bars[self.controls.instrument] == bar
            pygame.draw.rect(
                surface, ACCENT_COLOR if chosen else BOX_COLOR, box)
            centered("x", BACKGROUND if chosen else IDLE_COLOR, box)

        surface.blit(
            font.render("x: set left to right", True, IDLE_COLOR),
            (RESET_BOX.left, RESET_BOX.top - 22))
        pygame.draw.rect(surface, BOX_COLOR, RESET_BOX)
        centered("Reset to defaults", TEXT_COLOR, RESET_BOX)

    def _draw_menu(self, surface, font):
        """Draws the open list."""
        box = self.menu_box
        chosen = (self.controls.chord if box == CHORD_BOX
            else self.controls.instrument)

        for i, name in enumerate(self._menu_names(box)):
            item = self._menu_item(box, i)
            color = OCTAVE_COLOR if i == chosen else BOX_COLOR
            pygame.draw.rect(surface, color, item)
            surface.blit(
                font.render(name, True, TEXT_COLOR),
                (item.left + 8, item.top + 7))

    @staticmethod
    def _draw_selector(surface, font, box, name):
        """Draws a closed list showing name."""
        pygame.draw.rect(surface, BOX_COLOR, box)
        surface.blit(
            font.render(name, True, TEXT_COLOR), (box.left + 8, box.top + 7))
        pygame.draw.polygon(surface, TEXT_COLOR, [
            (box.right - 18, box.top + 11),
            (box.right - 8, box.top + 11),
            (box.right - 13, box.top + 17)])

    @staticmethod
    def _draw_bar(surface, bar, amount, color=ACCENT_COLOR):
        """Draws a slider filled to amount, 0 to 1."""
        pygame.draw.rect(surface, BOX_COLOR, bar)

        filled = bar.copy()
        filled.width = round(amount * bar.width)
        pygame.draw.rect(surface, color, filled)


def main():

    parser = argparse.ArgumentParser(description = 'synth')

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

    print("Hello Synth!")
    print("Press ESC to quit.")

    window = Window()

    # The controls are shared, written and read all together under the lock:
    # the worker reads the newest when it needs them, it doesn't fall behind
    # a queue of old ones.
    shared = multiprocessing.Array("d", window.controls.to_list())
    quitting = multiprocessing.Event()
    ready = multiprocessing.Queue()

    # A third process renders the instruments' notes.  Daemon, so it doesn't
    # outlive a failed start.
    requests = multiprocessing.Queue()
    results = multiprocessing.Queue()
    renderer = multiprocessing.Process(
        target=render_notes, args=(requests, results), daemon=True)
    renderer.start()

    p = multiprocessing.Process(
        target=worker,
        args = (
            ready, t, args.host_api, shared, quitting, args.n_buffers,
            args.chunk, args.debug, requests, results))
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
    screen = pygame.display.set_mode((WINDOW_WIDTH, PAD_SIZE))
    pygame.display.set_caption("Nsound Synth")
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
    requests.put(None)
    renderer.join()
    pygame.quit()
    print("Goodbye!")


if __name__ == "__main__":
    main()
