"""Tests bebot.py's chord synthesizer, src/examples/bebot_synth.py."""

import os
import sys
import unittest

# bebot_synth is an example, next to this folder.
sys.path.append(
    os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "examples"))

import bebot_synth as synth

CHUNK = 0.01
NAMES = [name for name, _ in synth.CHORDS]


def peak(sound):
    """Returns the largest magnitude of a Buffer."""
    return sound.getAbs().getMax()


class TestNotes(unittest.TestCase):
    """The pad's heights to notes and frequencies."""

    def test_note_frequency(self):
        """A4 is 440 Hz, an octave doubles it."""
        self.assertAlmostEqual(synth.note_frequency(69), 440.0)
        self.assertAlmostEqual(synth.note_frequency(57), 220.0)
        self.assertAlmostEqual(synth.note_frequency(60), 261.6255653, 6)

    def test_root_note_glides(self):
        """Without snapping the note follows the height, 40 to 650 Hz."""
        for height, frequency in (
                (0.0, 40.0), (0.5, (40.0 * 650.0) ** 0.5), (1.0, 650.0)):
            note = synth.root_note(height, False)
            self.assertAlmostEqual(synth.note_frequency(note), frequency)

    def test_root_note_snaps(self):
        """With snapping the note is the nearest semitone."""
        self.assertEqual(synth.root_note(0.26, True), 40.0)
        self.assertEqual(synth.root_note(0.27, True), 41.0)

    def test_root_note_snaps_inside_the_range(self):
        """At the ends the semitone doesn't leave the range."""
        self.assertEqual(synth.root_note(0.0, True), 28.0)
        self.assertEqual(synth.root_note(1.0, True), 75.0)

    def test_controls_round_trip(self):
        """Controls survive the list the processes share."""
        controls = synth.Controls(
            0.25, 0.75, True, 3, 6, True, False, 90.0, 5, 0.9, 0.6, 0.3, 0.1)

        self.assertEqual(controls.levels(), (0.9, 0.6, 0.3, 0.1))

        copy = synth.Controls.from_list(controls.to_list())

        self.assertEqual(copy, controls)
        self.assertIs(copy.held, True)
        self.assertIs(type(copy.chord), int)


class TestSynth(unittest.TestCase):
    """Rendering chords and arpeggios."""

    def setUp(self):
        """A pressed Major 7 chord."""
        self.synth = synth.Synth()
        self.controls = synth.Controls(
            x=0.3, y=0.5, held=True, presses=1, chord=NAMES.index("Maj7"))

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
        self.controls = synth.Controls(x=0.3, y=0.5, held=True, presses=1)

        sound = self.render(5)

        self.assert_gains([1.0, 0.0, 0.0, 0.0])
        self.assertAlmostEqual(
            self.synth.frequencies[0],
            synth.note_frequency(synth.root_note(0.5, False)))
        self.assertGreater(peak(sound), 0.01)

    def test_chord_voices(self):
        """The chord's notes sound together, sharing the volume."""
        sound = self.render(5)

        self.assertEqual(sound.getLength(), 480)
        self.assert_gains([0.25] * 4)

        root = synth.note_frequency(synth.root_note(0.5, False))

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

        root = synth.note_frequency(synth.root_note(0.75, False))

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

        self.assertAlmostEqual(self.synth.gains[0], 0.8)
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

        self.assertEqual([name for name, _ in synth.NOTE_VALUES], list(steps))

        for i, (name, _) in enumerate(synth.NOTE_VALUES):
            self.synth = synth.Synth()
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


if __name__ == "__main__":
    unittest.main()
