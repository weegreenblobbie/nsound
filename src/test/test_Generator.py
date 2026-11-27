"""Tests the Generator's Python binding."""

import unittest

import Nsound as ns


class TestChorus(unittest.TestCase):
    """Generator.setChorus()."""

    def setUp(self):
        """A Generator at 8 Hz playing a ramp, 0 to 7 / 8."""
        self.gen = ns.Generator(8, ns.Buffer([i / 8 for i in range(8)]))

    def test_chosen_voices_from_lists(self):
        """The output is the average of the voices asked for."""
        self.gen.setChorus([0.0, 1.0], [0.0, 0.5])

        self.assertEqual(
            self.gen.generate(1.0, 1.0).toList(),
            [0.4375, 0.125, 0.3125, 0.5, 0.6875, 0.375, 0.5625, 0.25])

    def test_chosen_voices_from_buffers(self):
        """Buffers work as the lists do."""
        self.gen.setChorus(
            detunes=ns.Buffer([0.0]), phases=ns.Buffer([0.25]))

        self.assertEqual(
            self.gen.generate(1.0, 1.0).toList(),
            [0.375, 0.5, 0.625, 0.75, 0.875, 0.0, 0.125, 0.25])

    def test_random_voices(self):
        """The number of voices and a spread still work."""
        self.gen.setChorus(1, 0.0)

        self.assertEqual(self.gen.generate(1.0, 1.0)[0], 0.125)

        self.gen.setChorus(0)

        self.assertEqual(self.gen.generate(1.0, 1.0)[1], 0.125)

    def test_mismatched_lengths(self):
        """Each voice needs both values."""
        with self.assertRaises(Exception):
            self.gen.setChorus([0.0, 0.1], [0.0])


if __name__ == "__main__":
    unittest.main()
