"""Tests synth.py's engine, src/examples/synth_engine.py."""

import math
import os
import sys
import unittest

# synth_engine is an example, next to this folder.
sys.path.append(
    os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "examples"))

import synth_chorus
import synth_engine as engine
import synth_instruments as instruments

CHUNK = 0.01
NAMES = [name for name, _ in engine.CHORDS]
NOTE_INSTRUMENTS = {
    spec.name: i for i, spec in enumerate(instruments.INSTRUMENTS)}


def peak(sound):
    """Returns the largest magnitude of a Buffer."""
    return sound.getAbs().getMax()


class TestNotes(unittest.TestCase):
    """The pad's heights to notes and frequencies."""

    def test_note_frequency(self):
        """A4 is 440 Hz, an octave doubles it."""
        self.assertAlmostEqual(engine.note_frequency(69), 440.0)
        self.assertAlmostEqual(engine.note_frequency(57), 220.0)
        self.assertAlmostEqual(engine.note_frequency(60), 261.6255653, 6)

    def test_root_note_glides(self):
        """Without snapping the note follows the height, 40 to 650 Hz."""
        for height, frequency in (
                (0.0, 40.0), (0.5, (40.0 * 650.0) ** 0.5), (1.0, 650.0)):
            note = engine.root_note(height, False)
            self.assertAlmostEqual(engine.note_frequency(note), frequency)

    def test_root_note_snaps(self):
        """With snapping the note is the nearest semitone."""
        self.assertEqual(engine.root_note(0.26, True), 40.0)
        self.assertEqual(engine.root_note(0.27, True), 41.0)

    def test_root_note_snaps_inside_the_range(self):
        """At the ends the semitone doesn't leave the range."""
        self.assertEqual(engine.root_note(0.0, True), 28.0)
        self.assertEqual(engine.root_note(1.0, True), 75.0)

    def test_granulator_pad_reaches_higher(self):
        """The Granulator's pad goes from 40 to 2600 Hz, others to 650."""
        granulator = [
            spec.name for spec in instruments.INSTRUMENTS].index("Granulator")

        for instrument, top in ((instruments.BEBOT, 650.0),
                (granulator, 2600.0)):
            for height, frequency in ((0.0, 40.0), (1.0, top)):
                note = engine.root_note(height, False, instrument)
                self.assertAlmostEqual(
                    engine.note_frequency(note), frequency)

        # The last semitone inside each range.
        self.assertEqual(engine.root_note(1.0, True, granulator), 99.0)

    def test_controls_round_trip(self):
        """Controls survive the list the processes share."""
        controls = engine.Controls(
            0.75, True, 3, 6, True, False, 90.0, 5, 0.9, 0.6, 0.3, 0.1)

        self.assertEqual(controls.levels(), (0.9, 0.6, 0.3, 0.1))

        copy = engine.Controls.from_list(controls.to_list())

        self.assertEqual(copy, controls)
        self.assertIs(copy.held, True)
        self.assertIs(type(copy.chord), int)


class TestSynth(unittest.TestCase):
    """Rendering chords and arpeggios."""

    def setUp(self):
        """A pressed Major 7 chord."""
        self.synth = engine.Synth()
        self.controls = engine.Controls(
            param1=0.3, y=0.5, held=True, presses=1,
            chord=NAMES.index("Maj7"))

    def render(self, chunks=1):
        """Renders chunks, returns the last one."""
        for _ in range(chunks):
            sound = self.synth.render(self.controls, CHUNK)

        return sound

    def assert_gains(self, expected):
        """Checks each voice's volume."""
        for gain, value in zip(self.synth.gains, expected):
            self.assertAlmostEqual(gain, value)

    def test_default_is_the_root_alone(self):
        """Without choosing a chord, one voice plays at full volume."""
        self.controls = engine.Controls(param1=0.3, y=0.5, held=True, presses=1)

        sound = self.render(5)

        self.assert_gains([1.0, 0.0, 0.0, 0.0])
        self.assertAlmostEqual(
            self.synth.frequencies[0],
            engine.note_frequency(engine.root_note(0.5, False)))
        self.assertGreater(peak(sound), 0.01)

    def test_chord_voices(self):
        """The chord's notes sound together, sharing the volume."""
        sound = self.render(5)

        self.assertEqual(sound.getLength(), 480)
        self.assert_gains([0.25] * 4)

        root = engine.note_frequency(engine.root_note(0.5, False))

        for frequency, offset in zip(self.synth.frequencies, (0, 4, 7, 11)):
            self.assertAlmostEqual(frequency, root * 2.0 ** (offset / 12.0))

        self.assertGreater(peak(sound), 0.01)
        self.assertLessEqual(peak(sound), 1.0)

    def test_note_levels_are_independent(self):
        """Lowering a note's level changes only that note."""
        self.render()
        self.controls.level2 = 0.6
        self.controls.level4 = 0.0
        self.render()

        self.assert_gains([0.25, 0.15, 0.25, 0.0])

    def test_brightness_changes_bebots_sound(self):
        """Bebot's brightness reaches its sound."""
        dull = self.render(5)

        self.synth = engine.Synth()
        self.controls.param1 = 0.9
        bright = self.render(5)

        self.assertGreater(peak(bright - dull), 0.02)

    def test_a_level_changes_the_sound(self):
        """The levels reach the sound."""
        full = peak(self.render(5))

        self.controls.level1 = 0.1
        self.controls.level2 = 0.1
        self.controls.level3 = 0.1
        self.controls.level4 = 0.1

        self.assertLess(peak(self.render(5)), 0.2 * full)

    def test_arpeggio_uses_the_note_levels(self):
        """Each arpeggio step plays at its note's level."""
        self.controls.arpeggio = True
        # 1/16 notes at 150 BPM, a step every 10 chunks.
        self.controls.tempo = 150.0
        self.controls.level1 = 0.8
        self.controls.level2 = 0.4
        self.render()

        self.assert_gains([0.8, 0.0, 0.0, 0.0])

        self.render(10)

        self.assert_gains([0.0, 0.4, 0.0, 0.0])

    def test_voices_follow_the_root(self):
        """Every voice moves with the root."""
        self.render()
        self.controls.y = 0.75
        self.render()

        root = engine.note_frequency(engine.root_note(0.75, False))

        self.assertAlmostEqual(self.synth.frequencies[0], root)
        self.assertAlmostEqual(
            self.synth.frequencies[2], root * 2.0 ** (7 / 12.0))

    def test_fewer_notes_fade_a_voice_out(self):
        """Changing to a three note chord silences the fourth voice."""
        self.render()
        self.controls.chord = NAMES.index("Minor")
        self.render()

        self.assert_gains([1 / 3, 1 / 3, 1 / 3, 0.0])

    def test_release_fades(self):
        """Releasing takes RELEASE_SECONDS to reach silence."""
        self.controls.arpeggio = True
        self.render()
        self.controls.held = False

        sound = self.render()

        self.assertAlmostEqual(self.synth.envelope.level, 0.8)
        self.assertGreater(peak(sound), 0.0)

        self.render(4)

        self.assertTrue(self.synth.is_silent())

        # The low pass filter rings out.
        self.assertLess(peak(self.render(10)), 1e-6)

    def test_release_keeps_the_pitch(self):
        """The mouse's position is ignored while the sound fades."""
        self.render()
        before = list(self.synth.frequencies)

        self.controls.held = False
        self.controls.y = 0.9
        self.render()

        self.assertEqual(self.synth.frequencies, before)

    def test_quick_click_sounds(self):
        """A press released before the sound saw it still plays."""
        self.controls.held = False

        sound = self.render()

        self.assertGreater(peak(sound), 0.0)
        self.assertFalse(self.synth.is_silent())

    def test_arpeggio_steps_and_repeats(self):
        """The notes play one at a time, in order, then again."""
        self.controls.arpeggio = True
        self.controls.chord = NAMES.index("Sus4")
        # 1/16 notes at 150 BPM, a step every 10 chunks.
        self.controls.tempo = 150.0

        playing = []

        for _ in range(40):
            self.render()
            playing.append(self.synth.gains.index(1.0))

        self.assertEqual(
            playing, [0] * 10 + [1] * 10 + [2] * 10 + [0] * 10)

    def test_arpeggio_note_values(self):
        """Half a second at 120 BPM, one beat, takes each value's steps."""
        steps = {
            "1": 0.25, "1/2": 0.5, "1/4": 1.0, "1/8": 2.0, "1/16": 4.0,
            "1/8T": 3.0, "1/16T": 6.0}

        self.assertEqual([name for name, _ in engine.NOTE_VALUES], list(steps))

        for i, (name, _) in enumerate(engine.NOTE_VALUES):
            self.synth = engine.Synth()
            self.controls.arpeggio = True
            self.controls.note_value = i
            self.render(50)

            self.assertAlmostEqual(
                self.synth.arpeggio_position, steps[name], msg=name)

    def test_arpeggio_steps_inside_a_chunk(self):
        """A step starts on its own sample, not at the next chunk."""
        self.controls.arpeggio = True
        # 1/16 notes at 120 BPM are 6000 samples, 12 and a half chunks.
        self.render(12)

        self.assert_gains([1.0, 0.0, 0.0, 0.0])

        # Half way through the chunk, each voice is half way through its ramp.
        self.render()

        self.assert_gains([0.5, 0.5, 0.0, 0.0])

    def test_arpeggio_fits_several_steps_in_a_chunk(self):
        """A long chunk takes every step in it."""
        self.controls.arpeggio = True

        sound = self.synth.render(self.controls, 0.3)

        # 14400 samples: two whole steps and part of the third.
        self.assertEqual(sound.getLength(), 14400)
        self.assert_gains([0.0, 0.0, 1.0, 0.0])
        self.assertAlmostEqual(self.synth.arpeggio_position, 2.4)

    def test_arpeggio_restarts_on_press(self):
        """A new press starts at the root."""
        self.controls.arpeggio = True
        self.controls.tempo = 150.0
        self.render(12)

        self.assert_gains([0.0, 1.0, 0.0, 0.0])

        self.controls.presses = 2
        self.render()

        self.assert_gains([1.0, 0.0, 0.0, 0.0])



CLARINET = NOTE_INSTRUMENTS["Clarinet"]
GUITAR = NOTE_INSTRUMENTS["Guitar Bass"]


def rendered():
    """Returns the (index, instrument) of those that render notes."""
    return [
        (i, spec) for i, spec in enumerate(instruments.INSTRUMENTS)
        if spec.render]


class SlowNotes(instruments.NoteCache):
    """Notes that are rendered only when the test says so."""

    def __init__(self):
        """Creates an empty cache."""
        super().__init__()
        self.asked = []

    def request(self, key):
        """Remembers the note asked for."""
        if key not in self.asked:
            self.asked.append(key)

    def finish(self):
        """Renders every note asked for."""
        for key in self.asked:
            self.put(key, instruments.render_note(key))


def controls_for(instrument, **settings):
    """Returns pressed Controls for an instrument with its default values."""
    values = instruments.default_values(instrument)

    return engine.Controls(
        y=settings.pop("y", 0.5), held=True, presses=1,
        instrument=instrument,
        snap=settings.pop("snap", True),
        param1=values[0], param2=values[1], param3=values[2],
        param4=values[3], **settings)


class TestInstruments(unittest.TestCase):
    """Rendering the instruments' notes."""

    def test_every_note_renders_at_the_parameters_ends(self):
        """No pitch or parameter value fails, blows up or is silent."""
        for i, instrument in rendered():
            lows = [p.low for p in instrument.parameters]
            highs = [p.high for p in instrument.parameters]

            for values in (lows, highs):
                values = tuple(values)

                for note in (28, 52, 75, 86):
                    n_samples = 0 if instrument.held else 2000
                    key = instruments.NoteKey(i, note, n_samples, values)
                    channels = instruments.render_note(key).channels
                    loudest = max(peak(c) for c in channels)
                    message = "%s %d %s" % (instrument.name, note, values)

                    self.assertGreater(loudest, 0.01, message)
                    self.assertLess(loudest, 2.0, message)

    def test_struck_note_is_its_note_value_long(self):
        """A short note is cut to length and faded out."""
        for n_samples in (2000, 12000):
            key = instruments.NoteKey(GUITAR, 40, n_samples, (0.0,) * 3)

            note = instruments.render_note(key)

            self.assertEqual(note.channels[0].getLength(), n_samples)
            self.assertIsNone(note.loop_start)
            self.assertAlmostEqual(note.channels[0][n_samples - 1], 0.0, 2)

    def test_held_note_repeats_smoothly(self):
        """The jump back to the loop's start is like any other sample's."""
        for i, spec in enumerate(instruments.INSTRUMENTS):
            if not (spec.held and spec.render):
                continue

            for number in (30, 52, 70):
                key = instruments.NoteKey(
                    i, number, 0, tuple(instruments.default_values(i)))
                note = instruments.render_note(key)

                self.assertEqual(note.loop_start, instruments.LOOP_START)

                for channel in note.channels:
                    samples = channel.toList()
                    loop = samples[note.loop_start:]

                    usual = max(abs(b - a) for a, b in zip(loop, loop[1:]))
                    jump = abs(samples[note.loop_start] - samples[-1])

                    self.assertLessEqual(
                        jump, usual, "%s %d" % (spec.name, number))

    def test_every_instrument_has_its_default_values_in_range(self):
        """Each parameter starts between its ends."""
        for spec in instruments.INSTRUMENTS:
            for parameter in spec.parameters:
                self.assertTrue(
                    parameter.low <= parameter.default <= parameter.high,
                    "%s %s" % (spec.name, parameter.label))

    def test_organ_pan_moves_the_sound(self):
        """Pan 0 is all left, 1 all right."""
        organ = NOTE_INSTRUMENTS["Organ Pipe"]

        for pan, loud, quiet in ((0.0, 0, 1), (1.0, 1, 0)):
            key = instruments.NoteKey(organ, 52, 0, (pan, 0.0, 0.0))
            channels = instruments.render_note(key).channels

            self.assertGreater(peak(channels[loud]), 0.5)
            self.assertEqual(peak(channels[quiet]), 0.0)

    def test_hat_opens(self):
        """An open hat rings longer than a closed one."""
        hat = NOTE_INSTRUMENTS["Hat"]
        late = []

        for opened in (0.0, 1.0):
            key = instruments.NoteKey(hat, 52, 24000, (opened, 0.0, 0.0))
            samples = instruments.render_note(key).channels[0]
            # From 50 to 100 ms.
            late.append(peak(samples.subbuffer(2400, 2400)))

        self.assertLess(late[0], 0.01)
        self.assertGreater(late[1], 0.05)

    def test_parameters_change_the_note(self):
        """An instrument's values reach its sound."""
        soft = instruments.NoteKey(CLARINET, 52, 0, (1.0, 600.0, 0.2))
        hard = instruments.NoteKey(CLARINET, 52, 0, (2.2, 600.0, 0.2))

        difference = (
            instruments.render_note(soft).channels[0].subbuffer(0, 9000)
            - instruments.render_note(hard).channels[0].subbuffer(0, 9000))

        self.assertGreater(peak(difference), 0.05)

    def test_cache_drops_the_oldest_note(self):
        """The cache holds its size in notes."""
        cache = instruments.NoteCache(size=2)
        keys = [
            instruments.NoteKey(GUITAR, note, 2000, (0.0,) * 3)
            for note in (40, 41, 42)]

        for key in keys:
            cache.get(key)

        self.assertEqual(list(cache.notes), keys[1:])


class TestNotePlayer(unittest.TestCase):
    """Playing the rendered instruments."""

    def render(self, chunks=1):
        """Renders chunks, returns the last one."""
        for _ in range(chunks):
            sound = self.synth.render(self.controls, CHUNK)

        return sound

    def start(self, instrument, notes=None, **settings):
        """Makes a synth with an instrument pressed."""
        self.synth = engine.Synth(notes)
        self.controls = controls_for(instrument, **settings)

    def keys(self):
        """Returns the MIDI note each position last started, or None."""
        return [
            trigger and trigger[0].note
            for trigger in self.synth.player.triggers]

    def test_output_is_stereo(self):
        """A panned note reaches only its side of the output."""
        self.start(NOTE_INSTRUMENTS["Organ Pipe"])
        self.controls.param1 = 1.0

        sound = self.render(5)

        self.assertEqual(sound.getNChannels(), 2)
        self.assertEqual(peak(sound[0]), 0.0)
        self.assertGreater(peak(sound[1]), 0.05)

    def test_every_instrument_plays(self):
        """Each instrument's chord is heard on both sides."""
        for i in range(len(instruments.INSTRUMENTS)):
            self.start(i, chord=NAMES.index("Major"))
            loudest = [0.0, 0.0]

            for _ in range(10):
                sound = self.render()
                loudest = [max(loudest[s], peak(sound[s])) for s in (0, 1)]

            name = instruments.INSTRUMENTS[i].name
            self.assertGreater(min(loudest), 0.01, name)
            self.assertLess(max(loudest), 1.0, name)

    def test_held_note_sounds_past_its_rendered_length(self):
        """A held note repeats for as long as it is held."""
        self.start(CLARINET)

        sound = self.render(350)

        self.assertEqual(sound.getLength(), 480)
        self.assertGreater(peak(sound), 0.05)

    def test_held_note_fades_after_release(self):
        """Releasing a held note silences it."""
        self.start(CLARINET)
        self.render(20)
        self.controls.held = False

        self.assertGreater(peak(self.render()), 0.0)

        self.render(5)

        self.assertTrue(self.synth.is_silent())
        self.assertEqual(peak(self.render()), 0.0)

    def hits(self, chunks):
        """Renders chunks, returns the chunks a new hit started in."""
        started = []
        last = None

        for chunk in range(chunks):
            self.render()
            note = self.synth.player.playing[0]

            if note is not None and note is not last:
                started.append(chunk)
                last = note

        return started

    def test_struck_note_repeats_on_the_beat(self):
        """While held, a hit at each note value."""
        # 1/16 at 150 BPM, 4800 samples, 10 chunks.
        self.start(GUITAR, tempo=150.0)

        self.assertEqual(self.hits(35), [0, 10, 20, 30])

    def test_struck_beat_starts_on_its_sample(self):
        """A hit between chunk boundaries starts there."""
        # 1/16 at 120 BPM, 6000 samples, half way through the 13th chunk.
        self.start(GUITAR)
        self.render(13)

        self.assertEqual(self.synth.player.playing[0].position, 240)

    def test_struck_note_moves_on_the_next_beat(self):
        """Moving doesn't hit between beats, the next hit has the pitch."""
        self.start(GUITAR, tempo=150.0)
        self.render(3)
        first = self.synth.player.playing[0]

        self.controls.y = 0.75
        # The beat's 9th chunk, the old hit goes on.
        self.render(6)

        self.assertIs(self.synth.player.playing[0], first)

        # Into the next beat.
        self.render(2)

        self.assertEqual(
            self.synth.player.playing[0].number,
            int(engine.root_note(0.75, True)))

    def test_struck_chord_hits_together(self):
        """Every note of a chord hits on each beat."""
        self.start(GUITAR, chord=NAMES.index("Major"), tempo=150.0)
        self.render()
        first = list(self.synth.player.playing[:3])
        self.render(10)

        for old, new in zip(first, self.synth.player.playing[:3]):
            self.assertIsNot(old, new)
            self.assertEqual(new.position, 480)

    def test_struck_notes_stop_after_release(self):
        """The last hit rings out and no more follow."""
        self.start(GUITAR, tempo=150.0)
        self.render(3)
        self.controls.held = False

        self.render(7)
        self.assertTrue(self.synth.is_silent())

        self.assertEqual(peak(self.render(20)), 0.0)

    def test_struck_note_rings_after_release(self):
        """Releasing doesn't cut a struck note off."""
        self.start(GUITAR, note_value=2)
        self.render()
        self.controls.held = False

        self.assertGreater(peak(self.render(10)), 0.01)

    def test_chord_notes(self):
        """A chord plays each of its notes."""
        self.start(CLARINET, chord=NAMES.index("Minor"))
        self.render(3)

        root = int(engine.root_note(0.5, True))

        self.assertEqual(self.keys(), [root, root + 3, root + 7, None])

        for playing in self.synth.player.playing[:3]:
            self.assertAlmostEqual(playing.gain, 1 / 3)

    def test_moving_plays_the_new_note(self):
        """A new semitone starts a new note and fades the old one out."""
        self.start(CLARINET)
        self.render(3)
        self.controls.y = 0.75
        self.render()

        self.assertEqual(self.keys()[0], int(engine.root_note(0.75, True)))

        # From the repeating part, there's no new attack.
        self.assertEqual(
            self.synth.player.playing[0].position,
            instruments.LOOP_START + 480)
        self.assertEqual(self.synth.player.fading, [])

    def test_arpeggio_strikes_each_note(self):
        """Each step starts the next note of the chord."""
        self.start(
            GUITAR, chord=NAMES.index("Major"), arpeggio=True, tempo=150.0)
        root = int(engine.root_note(0.5, True))
        started = []

        for chunks in (1, 10, 10):
            self.render(chunks)
            sounding = [
                i for i, playing in enumerate(self.synth.player.playing)
                if playing]
            started.append((sounding, self.keys()[sounding[-1]] - root))

        self.assertEqual(started, [([0], 0), ([1], 4), ([2], 7)])

    def test_waits_for_a_note_without_blocking(self):
        """Until a note is rendered there is silence, then it plays."""
        notes = SlowNotes()
        self.start(GUITAR, notes)

        self.assertEqual(peak(self.render(3)), 0.0)
        self.assertFalse(self.synth.is_silent())

        # Released before the note arrived, it still plays.
        self.controls.held = False
        self.render()
        notes.finish()

        self.assertGreater(peak(self.render()), 0.0)

    def test_struck_note_is_not_restruck_by_a_setting(self):
        """Changing the tempo while held leaves the note alone."""
        self.start(GUITAR, note_value=2)
        self.render(5)
        position = self.synth.player.playing[0].position

        self.controls.tempo = 90.0
        self.render()

        self.assertEqual(
            self.synth.player.playing[0].position, position + 480)

    def test_held_note_takes_a_new_setting(self):
        """Changing a parameter while held changes the note playing."""
        self.start(CLARINET)
        self.render(3)
        self.controls.param1 = 1.2
        self.render()

        self.assertEqual(self.synth.player.triggers[0][0].values[0], 1.2)
        self.assertEqual(
            self.synth.player.playing[0].position,
            instruments.LOOP_START + 480)

    def test_held_note_glides_without_snapping(self):
        """The pitch follows the mouse between semitones."""
        self.start(CLARINET, snap=False)
        self.render(2)
        note = self.synth.player.playing[0]

        exact = engine.root_note(0.5, False)
        self.assertNotAlmostEqual(exact, round(exact), 2)
        self.assertEqual(note.number, round(exact))
        self.assertAlmostEqual(note.rate, 2 ** ((exact - note.number) / 12))

        # A small move bends the same note, over the next chunk.
        self.controls.y = 0.503
        before = note.position
        self.render()

        moved = engine.root_note(0.503, False)
        self.assertIs(self.synth.player.playing[0], note)
        self.assertAlmostEqual(note.rate, 2 ** ((moved - note.number) / 12))
        self.assertNotAlmostEqual(note.position - before, 480.0, 1)

    def test_snapping_plays_the_rendered_pitch(self):
        """With snapping the note plays sample for sample."""
        self.start(CLARINET)
        self.render(3)

        self.assertEqual(self.synth.player.playing[0].rate, 1.0)
        self.assertEqual(self.synth.player.playing[0].position, 3 * 480)

    def test_held_note_glides_while_the_next_is_rendered(self):
        """A far move bends the note playing until the new one arrives."""
        notes = SlowNotes()
        self.start(CLARINET, notes, snap=False)
        self.render()
        notes.finish()
        self.render()
        old = self.synth.player.playing[0]

        self.controls.y = 0.6
        sound = self.render()

        self.assertIs(self.synth.player.playing[0], old)
        self.assertAlmostEqual(
            old.rate,
            2 ** ((engine.root_note(0.6, False) - old.number) / 12))
        self.assertGreater(peak(sound), 0.0)

    def test_struck_note_keeps_its_pitch(self):
        """A struck note is bent when struck, not after."""
        self.start(GUITAR, snap=False, note_value=2)
        self.render()
        note = self.synth.player.playing[0]
        rate = note.rate
        self.assertNotEqual(rate, 1.0)

        self.controls.y = 0.501
        self.render()

        self.assertEqual(note.rate, rate)

    def test_bent_note_is_interpolated(self):
        """Reading at half speed puts samples half way between."""
        samples = instruments.ns.Buffer([0.0, 1.0, 3.0, 7.0, 15.0])
        playing = engine.Playing(
            instruments.Note((samples,)), 1, 1.0, 1.0, 1, 60,
            rate=0.5, end_rate=0.5)

        self.assertEqual(
            playing.read(4)[0].toList(), [1.0, 2.0, 3.0, 5.0])
        self.assertEqual(playing.position, 3.0)

    def test_bent_note_repeats(self):
        """A fast note goes round its loop."""
        samples = instruments.ns.Buffer([0.0, 1.0, 2.0, 3.0, 4.0, 5.0])
        playing = engine.Playing(
            instruments.Note((samples,), loop_start=2), 4, 1.0, 1.0, 1, 60,
            rate=1.5, end_rate=1.5)

        self.assertEqual(
            playing.read(4)[0].toList(), [4.0, 3.5, 3.0, 4.5])
        # 6 samples on from 4 is the end, one loop back.
        self.assertEqual(playing.position, 6.0)

    def test_chord_notes_are_asked_for_ahead(self):
        """The arpeggio's later notes are asked for at the press."""
        notes = SlowNotes()
        self.start(GUITAR, notes, chord=NAMES.index("Maj7"), arpeggio=True)
        self.render()

        self.assertEqual(len(notes.asked), 4)

    def test_changing_instrument_stops_the_other(self):
        """Choosing Bebot drops the notes playing."""
        self.start(CLARINET)
        self.render(3)
        self.controls.instrument = instruments.BEBOT
        self.render()

        self.assertTrue(self.synth.player.is_silent())
        self.assertFalse(self.synth.is_silent())


class TestEnvelope(unittest.TestCase):
    """The attack, decay, sustain and release."""

    def assert_levels(self, line, expected):
        """Checks the volume at samples."""
        for sample, level in expected.items():
            self.assertAlmostEqual(line[sample], level, 6, str(sample))

    def test_attack_decay_sustain_release(self):
        """The volume follows the times, 480 samples each 10 ms."""
        envelope = engine.Envelope()
        envelope.trigger(0.01, 0.02, 0.4)

        self.assert_levels(
            envelope.read(2400),
            {0: 0.0, 240: 0.5, 480: 1.0, 960: 0.7, 1440: 0.4, 2399: 0.4})
        self.assertFalse(envelope.finished())

        envelope.release(0.01)

        self.assert_levels(
            envelope.read(960), {0: 0.4, 240: 0.2, 480: 0.0, 959: 0.0})
        self.assertTrue(envelope.finished())

    def test_retrigger_starts_from_the_level(self):
        """A new attack rises from where the volume is."""
        envelope = engine.Envelope()
        envelope.trigger(0.01, 0.01, 0.5)
        envelope.read(240)
        envelope.trigger(0.01, 0.01, 0.5)

        self.assert_levels(envelope.read(480), {0: 0.5, 240: 0.75})

    def test_reading_in_pieces(self):
        """Chunks make the same volume as one read."""
        whole = engine.Envelope()
        pieces = engine.Envelope()

        for envelope in (whole, pieces):
            envelope.trigger(0.013, 0.007, 0.3)

        line = []

        for length in (100, 7, 600, 293):
            line += pieces.read(length).toList()

        for a, b in zip(line, whole.read(1000).toList()):
            self.assertAlmostEqual(a, b, 9)

    def test_struck_envelope_releases_at_the_end(self):
        """A hit fades out over the release at the end of the note."""
        controls = engine.Controls(decay=0.01, sustain=0.5, release=0.02)
        envelope = engine.Envelope.hit(controls, 2400)

        self.assert_levels(
            envelope.read(2400), {0: 1.0, 480: 0.5, 1440: 0.5, 1920: 0.25})
        self.assertTrue(envelope.finished())

    def test_struck_envelope_with_a_long_release(self):
        """A release longer than the note fades all of it."""
        controls = engine.Controls(release=2.0)
        envelope = engine.Envelope.hit(controls, 1000)

        line = envelope.read(1000)

        self.assertAlmostEqual(line[500], 0.5, 2)
        self.assertTrue(envelope.finished())


class TestEffects(unittest.TestCase):
    """The envelope, filter and normalization on the synth's sound."""

    def start(self, instrument, **settings):
        """Makes a synth with an instrument pressed."""
        self.synth = engine.Synth()
        self.controls = controls_for(instrument, **settings)

    def render(self, chunks=1):
        """Renders chunks, returns the loudest sample of each."""
        peaks = []

        for _ in range(chunks):
            sound = self.synth.render(self.controls, CHUNK)
            peaks.append(max(peak(sound[0]), peak(sound[1])))

        return peaks

    def test_slow_attack(self):
        """A held instrument swells."""
        for instrument in (instruments.BEBOT, CLARINET):
            self.start(instrument, attack=1.0)
            peaks = self.render(120)

            self.assertLess(max(peaks[:10]), 0.2 * max(peaks[100:]))

    def test_zero_sustain_silences_a_held_note(self):
        """After the decay there's nothing, though held."""
        for instrument in (instruments.BEBOT, CLARINET):
            self.start(instrument, decay=0.05, sustain=0.0)
            peaks = self.render(20)

            self.assertGreater(max(peaks[:5]), 0.02)
            self.assertLess(max(peaks[8:]), 1e-6)

    def test_long_release(self):
        """A held instrument fades over the release, then is silent."""
        for instrument in (instruments.BEBOT, CLARINET):
            self.start(instrument, release=0.5)
            self.render(20)
            self.controls.held = False
            peaks = self.render(40)

            self.assertGreater(max(peaks[20:30]), 0.01)
            self.assertFalse(self.synth.is_silent())

            self.render(10)

            self.assertTrue(self.synth.is_silent())

    def test_arpeggio_restarts_the_envelope(self):
        """Each step has its own attack and decay."""
        self.start(
            CLARINET, arpeggio=True, tempo=150.0, decay=0.05, sustain=0.0)
        peaks = self.render(30)

        for step in range(3):
            self.assertGreater(max(peaks[10 * step:10 * step + 3]), 0.05)
            self.assertLess(max(peaks[10 * step + 7:10 * step + 10]), 1e-6)

    def test_struck_decay_shortens_each_hit(self):
        """A short decay to nothing cuts a long tail."""
        self.start(GUITAR, note_value=2, decay=0.05, sustain=0.0)
        peaks = self.render(50)

        self.assertGreater(max(peaks[:5]), 0.02)
        self.assertLess(max(peaks[7:49]), 1e-6)

    def test_low_cutoff_quietens_a_high_note(self):
        """The filter takes away what is above its cutoff."""
        self.start(CLARINET, y=0.9)
        open_peak = max(self.render(20)[10:])

        self.start(CLARINET, y=0.9, cutoff=150.0)
        closed_peak = max(self.render(20)[10:])

        self.assertLess(closed_peak, 0.5 * open_peak)

    def test_open_filter_changes_nothing(self):
        """At the top cutoff without resonance the filter is off."""
        self.start(instruments.BEBOT)
        self.render(5)

        self.assertIsNone(self.synth.filters)

    def test_filter_stays_stable(self):
        """No cutoff or resonance blows the filter up."""
        for cutoff in (engine.LOW_CUTOFF, 3000.0, engine.HIGH_CUTOFF):
            for instrument in (instruments.BEBOT, NOTE_INSTRUMENTS["Flute"]):
                self.start(
                    instrument, cutoff=cutoff, resonance=engine.MAX_RESONANCE,
                    chord=NAMES.index("Maj7"))

                self.assertTrue(all(p < 2.0 for p in self.render(40)), cutoff)

    def test_most_resonance_does_not_ring(self):
        """At the resonance's limit a click dies away."""
        for cutoff in (300.0, 1500.0, 6000.0, engine.HIGH_CUTOFF):
            self.start(
                instruments.BEBOT, cutoff=cutoff,
                resonance=engine.MAX_RESONANCE)

            click = instruments.ns.Buffer([0.3] + [0.0] * 23999)
            left, _ = self.synth._filter(self.controls, click, click * 1.0)

            self.assertLess(peak(left.subbuffer(12000, 12000)), 1e-4, cutoff)

    def test_normalizing_evens_out_the_loudness(self):
        """A quiet note is brought up to the others'."""
        quiet = instruments.NoteKey(CLARINET, 28, 0, (1.8, 1000.0, 0.2))
        loud = instruments.NoteKey(CLARINET, 75, 0, (1.8, 1000.0, 0.2))

        def loudness(key):
            samples = instruments.render_note(key).channels[0]
            part = samples.subbuffer(instruments.LOOP_START, 24000)

            return ((part * part).getSum() / 24000) ** 0.5

        self.assertGreater(loudness(loud) / loudness(quiet), 3.0)

        ratio = (loudness(loud._replace(normalize=True))
            / loudness(quiet._replace(normalize=True)))

        # By its loudest 50 ms, the average differs a little.
        self.assertLess(ratio, 1.5)

    def test_normalized_peaks_are_limited(self):
        """A short hit isn't raised past the loudest peak."""
        for i, spec in rendered():
            key = instruments.NoteKey(
                i, 40, 0 if spec.held else 2000,
                tuple(instruments.default_values(i)), True)

            for channel in instruments.render_note(key).channels:
                self.assertLessEqual(
                    peak(channel), instruments.LOUDEST_PEAK + 1e-9, spec.name)


GRANULATOR = NOTE_INSTRUMENTS["Granulator"]


class TestGranulator(unittest.TestCase):
    """The grains and their scheduling."""

    def start(self, rate=40.0, waves=6.0, follows=False, **settings):
        """Makes a synth with the Granulator pressed, a Gaussian grain."""
        self.synth = engine.Synth()
        self.controls = controls_for(GRANULATOR, **settings)
        self.controls.param1 = rate
        self.controls.param2 = float(follows)
        self.controls.param3 = waves
        self.controls.param4 = 0.0

    def render(self, chunks=1):
        """Renders chunks, returns their left samples."""
        samples = []

        for _ in range(chunks):
            samples += self.synth.render(self.controls, CHUNK)[0].toList()

        return samples

    @staticmethod
    def starts(samples):
        """Returns where sound follows at least 100 silent samples."""
        found = []
        silent = 100

        for i, sample in enumerate(samples):
            if abs(sample) > 1e-9:
                if silent >= 100:
                    found.append(i)

                silent = 0
            else:
                silent += 1

        return found

    @staticmethod
    def level(samples):
        """Returns the samples' RMS."""
        return math.sqrt(sum(s * s for s in samples) / len(samples))

    def test_defaults(self):
        """40 grains a second, not following, of 3 waves, Wide."""
        self.assertEqual(
            instruments.default_values(GRANULATOR), [40.0, 0.0, 3.0, 1.0])

    def test_one_grain(self):
        """A grain is its waves long, at its frequency."""
        grain = instruments.render_grain(1000.0, 6, 0)

        self.assertEqual(grain.getLength(), 288)
        self.assertGreater(peak(grain), 0.5)

    def test_late_grain(self):
        """A grain between samples has samples between its neighbours."""
        grain = instruments.render_grain(1000.0, 6, 0)
        late = instruments.render_grain(1000.0, 6, 0, 0.25)

        self.assertEqual(late.getLength(), grain.getLength() + 1)

        for i in (1, 100, 200):
            self.assertAlmostEqual(
                late[i], 0.75 * grain[i] + 0.25 * grain[i - 1])

    def test_shapes_differ(self):
        """Each shape's envelope makes another grain."""
        grains = [
            instruments.render_grain(1000.0, 6, shape) for shape in range(4)]

        for i, a in enumerate(grains):
            for b in grains[i + 1:]:
                self.assertGreater(peak(a - b), 0.1)

    def test_grains_per_second(self):
        """Grains start at the rate, on their own samples."""
        # 6 waves at 1300 Hz are short, with silence between them.
        for rate in (25.0, 64.0):
            self.start(rate=rate, y=0.8)
            starts = self.starts(self.render(50))
            gaps = [b - a for a, b in zip(starts, starts[1:])]

            self.assertEqual(len(starts), math.ceil(0.5 * rate), rate)

            for gap in gaps:
                self.assertAlmostEqual(gap, 48000 / rate, delta=2)

    def test_rate_follows_the_pitch(self):
        """Following, a grain starts each wave, whatever the rate's slider."""
        self.start(rate=5.0, waves=1.0, follows=True, y=0.5)
        frequency = engine.note_frequency(
            engine.root_note(0.5, True, GRANULATOR))

        samples = self.render(20)[4800:]

        # The sound repeats at the pitch, to the nearest sample's fraction.
        period = 48000 / frequency
        n_periods = 20
        shift = round(n_periods * period)

        self.assertAlmostEqual(shift, n_periods * period, delta=0.1 * period)

        difference = [a - b for a, b in zip(samples, samples[shift:])]

        self.assertLess(self.level(difference), 0.1 * self.level(samples))

    def test_following_is_as_loud_at_every_pitch(self):
        """Low notes are no quieter than high ones."""
        levels = []

        for y in (0.0, 0.2, 0.5, 1.0):
            self.start(waves=3.0, follows=True, y=y)
            levels.append(self.level(self.render(60)[14400:]))

        self.assertGreater(min(levels), 0.7 * max(levels))

    def test_rate_is_held_to_the_pitch(self):
        """A rate above the pitch plays as the pitch, it doesn't cancel."""
        self.start(rate=300.0, y=0.0)
        fast = self.render(50)

        self.start(follows=True, y=0.0)

        self.assertEqual(fast, self.render(50))
        self.assertGreater(self.level(fast[4800:]), 0.05)

    def test_next_grain_has_the_new_pitch(self):
        """Moving changes the grains that start afterwards."""
        self.start(rate=25.0, y=1.0)
        self.render()
        first = self.synth.granulator.newest[0]

        self.controls.y = 0.8
        # Into the second grain, 1920 samples after the first.
        self.render(4)
        newest = self.synth.granulator.newest[0]

        # A lower pitch's grain is longer.
        self.assertGreater(newest.getLength(), first.getLength())

    def test_grain_mixes_across_chunks(self):
        """A long grain plays on chunk after chunk, sample for sample."""
        # One grain, 30 waves at 650 Hz are over 2000 samples.
        self.start(rate=5.0, waves=30.0, y=0.8)

        played = self.render(6)

        grain = self.synth.granulator.newest[0].toList()
        self.assertGreater(len(grain), 960)

        # After the attack's ramps, the sound is the grain's: grains that
        # don't overlap play at their own volume.
        for sample in range(960, min(len(grain), 2880), 97):
            self.assertAlmostEqual(played[sample], 0.333 * grain[sample], 6)

    def test_long_grains_are_forgotten(self):
        """Played grains don't pile up however long the note is held."""
        self.start(rate=40.0, waves=30.0, y=0.0)
        self.render(400)

        self.assertLess(self.synth.granulator.mixed[0].getLength(), 100000)

    def test_loudness_stays_within_full_scale(self):
        """Hundreds of overlapping grains don't clip."""
        self.start(
            waves=30.0, follows=True, y=1.0, chord=NAMES.index("Maj7"))

        loudest = max(abs(sample) for sample in self.render(100))

        self.assertGreater(loudest, 0.02)
        self.assertLessEqual(loudest, 0.34)

    def test_release_ends_the_grains(self):
        """After the release there is silence."""
        self.start(rate=100.0)
        self.render(10)
        self.controls.held = False

        self.assertGreater(max(map(abs, self.render())), 0.0)

        self.render(5)

        self.assertTrue(self.synth.is_silent())
        self.assertEqual(max(map(abs, self.render())), 0.0)

    def test_arpeggio_plays_one_note_at_a_time(self):
        """Only the step's position sounds, after the last one fades."""
        self.start(
            rate=100.0, arpeggio=True, tempo=150.0, chord=NAMES.index("Major"))
        self.render(15)

        sounding = [
            mixed.getLength() > 0 for mixed in self.synth.granulator.mixed]

        self.assertEqual(sounding, [False, True, False, False])


class TestPiano(unittest.TestCase):
    """piano.py's oscillators and envelope."""

    def start(self, wave=0, harmonics=7, own=True, **settings):
        """Makes a synth with the Piano pressed."""
        self.synth = engine.Synth()
        self.controls = controls_for(NOTE_INSTRUMENTS["Piano"], **settings)
        self.controls.param1 = float(wave)
        self.controls.param2 = float(harmonics)
        self.controls.param3 = float(own)

    def render(self, chunks=1):
        """Renders chunks, returns their left samples."""
        samples = []

        for _ in range(chunks):
            samples += self.synth.render(self.controls, CHUNK)[0].toList()

        return samples

    def test_sine_at_the_pads_pitch(self):
        """The wave crosses zero twice a period."""
        self.start(own=False)
        samples = self.render(100)[4800:]
        frequency = engine.note_frequency(
            engine.root_note(0.5, True, NOTE_INSTRUMENTS["Piano"]))

        crossings = sum(a < 0 <= b for a, b in zip(samples, samples[1:]))

        self.assertAlmostEqual(
            crossings, frequency * len(samples) / 48000.0, delta=1.5)
        self.assertAlmostEqual(max(samples), 0.333, 2)

    def test_waves_and_harmonics_differ(self):
        """Each wave, and its harmonics, sound their own way."""
        sounds = []

        for wave, harmonics in ((0, 7), (1, 7), (2, 7), (1, 2)):
            self.start(wave=wave, harmonics=harmonics)
            sounds.append(self.render(30))

        for i, a in enumerate(sounds):
            for b in sounds[i + 1:]:
                self.assertGreater(
                    max(abs(x - y) for x, y in zip(a, b)), 0.01)

    def test_piano_envelope(self):
        """Its attack, decay to a third and release, whatever the sliders."""
        self.start(attack=0.0, sustain=1.0, release=2.0)
        levels = []

        for chunk in range(80):
            self.render()
            levels.append(self.synth.envelope.level)

        # 10 ms chunks: half way up, full, then sustaining.
        self.assertAlmostEqual(levels[9], 0.5)
        self.assertAlmostEqual(levels[19], 1.0)
        self.assertAlmostEqual(levels[70], 0.333)

        self.controls.held = False
        self.render(10)

        self.assertTrue(self.synth.is_silent())

    def test_sliders_envelope_without_its_own(self):
        """With the box off the panel's envelope is used."""
        self.start(own=False, attack=1.0)
        self.render(10)

        self.assertAlmostEqual(self.synth.envelope.level, 0.1)

    def test_chord_glides(self):
        """Without snapping every note follows the mouse."""
        self.start(snap=False, chord=NAMES.index("Major"))
        self.render(2)
        self.controls.y = 0.7
        self.render()

        root = engine.note_frequency(engine.root_note(
            0.7, False, NOTE_INSTRUMENTS["Piano"]))

        for frequency, offset in zip(
                self.synth.oscillators.frequencies, (0, 4, 7)):
            self.assertAlmostEqual(frequency, root * 2 ** (offset / 12.0))


class TestOneNoteArpeggio(unittest.TestCase):
    """The arpeggio without a chord."""

    def levels(self, instrument, chunks=30, **settings):
        """Returns the loudest sample of each chunk of a held arpeggio."""
        self.synth = engine.Synth()
        controls = controls_for(
            instrument, arpeggio=True, tempo=150.0, **settings)

        if instrument == NOTE_INSTRUMENTS["Piano"]:
            controls.param3 = 0.0

        return [
            peak(self.synth.render(controls, CHUNK)[0])
            for _ in range(chunks)]

    def test_held_instruments_repeat_the_note(self):
        """Each step sounds for its first half and rests for the second."""
        # 1/16 at 150 BPM, 10 chunks a step.
        for name in ("Bebot", "Clarinet", "Granulator", "Piano"):
            levels = self.levels(
                instruments.INSTRUMENTS.index(next(
                    spec for spec in instruments.INSTRUMENTS
                    if spec.name == name)))

            for step in range(3):
                sounding = levels[10 * step + 1:10 * step + 5]
                # The grains and the low pass filter die away at first.
                resting = levels[10 * step + 8:10 * step + 10]

                self.assertGreater(max(sounding), 0.02, name)
                self.assertLess(max(resting), 0.1 * max(sounding), name)

    def test_a_chord_has_no_rests(self):
        """A chord's notes follow each other."""
        levels = self.levels(CLARINET, chord=NAMES.index("Major"))

        self.assertGreater(min(levels[5:]), 0.02)

    def test_struck_note_is_hit_each_step(self):
        """A struck note rings through the step as without the arpeggio."""
        arpeggio = self.levels(GUITAR)

        self.synth = engine.Synth()
        controls = controls_for(GUITAR, tempo=150.0)
        plain = [
            peak(self.synth.render(controls, CHUNK)[0]) for _ in range(30)]

        for a, b in zip(arpeggio, plain):
            self.assertAlmostEqual(a, b, 6)

    def test_release_during_a_rest_stays_silent(self):
        """Letting go between notes adds no sound."""
        self.synth = engine.Synth()
        controls = controls_for(CLARINET, arpeggio=True, tempo=150.0)

        for _ in range(8):
            self.synth.render(controls, CHUNK)

        controls.held = False
        levels = [
            peak(self.synth.render(controls, CHUNK)[0]) for _ in range(10)]

        self.assertLess(max(levels), 1e-6)
        self.assertTrue(self.synth.is_silent())


class TestChorus(unittest.TestCase):
    """The chorus effect."""

    def setUp(self):
        """A silent chorus and a 1 kHz tone on each side."""
        self.chorus = synth_chorus.Chorus()
        sine = instruments.ns.Sine(48000.0)
        self.tone = 0.5 * sine.generate(1.0, 1000.0)

    def process(self, left, right=None, on=True, voices=1, detune=20.0,
            rate=1.0, mix=1.0, width=0.0):
        """Returns the chorus's two sides as lists."""
        out = self.chorus.process(
            left, right or left * 1.0, on, voices, detune, rate, mix, width)

        return [side.toList() for side in out]

    def test_off_changes_nothing(self):
        """Off, the sound passes as it is."""
        left, _ = self.process(self.tone, on=False)

        self.assertEqual(left, self.tone.toList())
        self.assertTrue(self.chorus.is_silent())

    def test_output_averages_the_sound_and_voices(self):
        """Without detune a voice is the sound 5 ms late."""
        # The mix ramps up over the first call.
        self.process(self.tone.subbuffer(0, 4800), detune=0.0)
        left, _ = self.process(
            self.tone.subbuffer(4800, 4800), detune=0.0)

        for i in range(0, 4800, 97):
            expected = (self.tone[4800 + i] + self.tone[4560 + i]) / 2.0
            self.assertAlmostEqual(left[i], expected, 6)

    def test_mix_sets_the_voices_volume(self):
        """At half mix the voice is half as loud as the sound."""
        self.process(self.tone.subbuffer(0, 4800), detune=0.0, mix=0.5)
        left, _ = self.process(
            self.tone.subbuffer(4800, 4800), detune=0.0, mix=0.5)

        expected = (self.tone[4900] + 0.5 * self.tone[4660]) / 1.5
        self.assertAlmostEqual(left[100], expected, 6)

    def test_swing_bends_the_pitch_by_the_detune(self):
        """The delay changes fastest by the detune's ratio."""
        swing = synth_chorus.swing_samples(20.0, 1.0)

        per_second = swing * 2.0 * math.pi * 1.0 / 48000.0

        self.assertAlmostEqual(per_second, 2.0 ** (20.0 / 1200.0) - 1.0)

    def test_slow_swing_is_limited(self):
        """A slow rate can't delay without end."""
        self.assertEqual(
            synth_chorus.swing_samples(50.0, 0.1), 0.03 * 48000.0)

    def test_width_spreads_the_voices(self):
        """Wide, two voices are each on one side."""
        self.assertEqual(
            synth_chorus.voice_gains(2, 1.0), [(1.0, 0.0), (0.0, 1.0)])
        self.assertEqual(
            synth_chorus.voice_gains(3, 0.5),
            [(1.0, 0.5), (1.0, 1.0), (0.5, 1.0)])

        left, right = self.process(self.tone, voices=2, width=1.0)

        self.assertGreater(
            max(abs(a - b) for a, b in zip(left, right)), 0.1)

    def test_chunks_match_one_call(self):
        """The delays carry on from chunk to chunk."""
        # The ramp to the full mix happens in the first 480 samples.
        whole = synth_chorus.Chorus()
        settings = (True, 3, 20.0, 2.0, 1.0, 0.5)
        first = self.tone.subbuffer(0, 480)
        rest = self.tone.subbuffer(480, 4800)

        for chorus in (whole, self.chorus):
            chorus.process(first, first * 1.0, *settings)

        expected = whole.process(rest, rest * 1.0, *settings)[0].toList()
        out = []

        for k in range(10):
            part = rest.subbuffer(480 * k, 480)
            out += self.chorus.process(
                part, part * 1.0, *settings)[0].toList()

        for a, b in zip(out, expected):
            self.assertAlmostEqual(a, b, 9)

    def test_voices_ring_out(self):
        """After the sound stops the voices play what is left."""
        self.process(self.tone.subbuffer(0, 4800))
        silence = instruments.ns.Buffer.zeros(480)

        left, _ = self.process(silence)

        self.assertGreater(max(map(abs, left)), 0.05)
        self.assertFalse(self.chorus.is_silent())

        for _ in range(10):
            left, _ = self.process(silence)

        self.assertTrue(self.chorus.is_silent())
        self.assertEqual(max(map(abs, left)), 0.0)

    def test_every_instrument_takes_the_chorus(self):
        """The synth's output changes, then ends after the release."""
        for i in range(len(instruments.INSTRUMENTS)):
            sounds = []

            for on in (False, True):
                synth = engine.Synth()
                controls = controls_for(i, chorus=on)
                sound = []

                for _ in range(10):
                    sound += synth.render(controls, CHUNK)[0].toList()

                sounds.append(sound)

            name = instruments.INSTRUMENTS[i].name
            self.assertGreater(
                max(abs(a - b) for a, b in zip(*sounds)), 0.005, name)
            controls.held = False

            for _ in range(100):
                synth.render(controls, CHUNK)

            self.assertTrue(synth.is_silent(), name)


if __name__ == "__main__":
    unittest.main()
