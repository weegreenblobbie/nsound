"""synth.py's chorus effect: copies of the sound that drift in pitch.

Each voice plays the sound a little late, by a delay that swings slowly.  A
growing delay lowers the voice's pitch and a shrinking one raises it.  The
output is the average of the sound and its voices.
"""

import math

import Nsound as ns

import synth_instruments as instruments

SR = instruments.SR

# A voice's shortest delay, and the most its swing adds either side of the
# middle, in seconds.
BASE_DELAY = 0.005
LONGEST_SWING = 0.03

# The samples of the sound kept for the voices to play.
HISTORY = int((BASE_DELAY + 2.0 * LONGEST_SWING) * SR) + 2

# Samples quieter than this are silence.
SILENCE = 1e-6


def swing_samples(detune: float, rate: float) -> float:
    """Returns the delay's swing in samples that bends the pitch by at most
    detune cents, swinging rate times a second.
    """
    # The pitch changes by the delay's rate of change.
    seconds = (2.0 ** (detune / 1200.0) - 1.0) / (2.0 * math.pi * rate)

    return min(seconds, LONGEST_SWING) * SR


def voice_gains(n_voices: int, width: float) -> list:
    """Returns each voice's (left, right) volume: spread from side to side by
    width, 0 to 1.
    """
    gains = []

    for voice in range(n_voices):
        # From -1, left, to 1, right.
        side = 1.0 if n_voices == 1 else 2.0 * voice / (n_voices - 1) - 1.0

        gains.append(
            (1.0 - max(0.0, side) * width, 1.0 + min(0.0, side) * width))

    return gains


class Chorus:
    """Adds the chorus to a stereo sound, a chunk at a time."""

    def __init__(self) -> None:
        """Starts silent."""
        self.clear()

    def clear(self) -> None:
        """Forgets the sound heard so far."""
        self.history = [[0.0] * HISTORY, [0.0] * HISTORY]
        self.cycles = 0.0       # how far the delays have swung
        self.swing = None       # where the swing's ramp has reached
        self.mix = 0.0          # the voices' volume, likewise
        self.remaining = 0      # samples the voices may still sound for

    def is_silent(self) -> bool:
        """Returns True when the voices have nothing left to play."""
        return self.remaining == 0

    def process(
        self, left: ns.Buffer, right: ns.Buffer, on: bool, n_voices: int,
        detune: float, rate: float, mix: float, width: float,
    ) -> tuple:
        """Returns the two sides with the chorus.

        detune is in cents, rate in swings a second, mix the voices' volume and
        width their spread between the sides, both 0 to 1.
        """
        target = mix if on else 0.0

        if target == 0.0 and self.mix == 0.0:
            self.clear()

            return left, right

        n_samples = left.getLength()
        swing = swing_samples(detune, rate)

        if self.swing is None:
            self.swing = swing

        # The swing and mix ramp across the chunk, so changes don't click.
        swing_step = (swing - self.swing) / n_samples
        mix_step = (target - self.mix) / n_samples
        base = BASE_DELAY * SR

        # Where each voice reads from, in samples into history + chunk.
        positions = []

        for voice in range(n_voices):
            start = self.cycles + voice / n_voices
            positions.append([
                HISTORY + k - base - (self.swing + swing_step * k) * (
                    1.0 + math.sin(2.0 * math.pi * (start + rate * k / SR)))
                for k in range(n_samples)])

        gains = voice_gains(n_voices, width)
        mixes = [self.mix + mix_step * k for k in range(n_samples)]
        out = []
        loud = False

        for side, sound in enumerate((left, right)):
            dry = sound.toList()
            samples = self.history[side] + dry
            wet = [0.0] * n_samples

            for voice, where in enumerate(positions):
                gain = gains[voice][side]

                for k, position in enumerate(where):
                    i = int(position)
                    low = samples[i]
                    wet[k] += gain * (
                        low + (samples[i + 1] - low) * (position - i))

            total = sum(gain[side] for gain in gains)

            out.append(ns.Buffer([
                (d + m * w) / (1.0 + m * total)
                for d, m, w in zip(dry, mixes, wet)]))

            self.history[side] = samples[-HISTORY:]
            loud = loud or any(abs(d) > SILENCE for d in dry)

        self.cycles = (self.cycles + rate * n_samples / SR) % 1.0
        self.swing = swing
        self.mix = target
        self.remaining = (
            HISTORY if loud else max(0, self.remaining - n_samples))

        return tuple(out)
