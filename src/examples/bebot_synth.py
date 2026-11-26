"""bebot.py's chord synthesizer: turns the controls into sound.

The window writes the Controls, the sound's process reads them and renders the
sound a chunk at a time with Synth.  Nothing here uses pygame or a sound card.
"""

import dataclasses
import math

import Nsound as ns

SR = 48000.0

# The pad's root note range in Hz, bottom to top.
LOW_FREQUENCY = 40.0
HIGH_FREQUENCY = 650.0

# The same range in MIDI note numbers, and the whole semitones inside it.
LOW_NOTE = 69.0 + 12.0 * math.log2(LOW_FREQUENCY / 440.0)
HIGH_NOTE = 69.0 + 12.0 * math.log2(HIGH_FREQUENCY / 440.0)
SEMITONES = range(math.ceil(LOW_NOTE), math.floor(HIGH_NOTE) + 1)

# The chord qualities: name and the semitones of its notes above the root.
# The first, the default, is the root alone.
CHORDS = (
    ("None", (0,)),
    ("Major", (0, 4, 7)),
    ("Minor", (0, 3, 7)),
    ("Sus2", (0, 2, 7)),
    ("Sus4", (0, 5, 7)),
    ("Dim", (0, 3, 6)),
    ("Aug", (0, 4, 8)),
    ("Maj7", (0, 4, 7, 11)),
    ("Min7", (0, 3, 7, 10)),
)

N_VOICES = max(len(offsets) for _, offsets in CHORDS)

# The arpeggio's tempo range, in beats per minute.  A beat is a quarter note.
LOW_TEMPO = 40.0
HIGH_TEMPO = 240.0

# The arpeggio's note values: name and length in beats.  T is a triplet.
NOTE_VALUES = (
    ("1", 4.0),
    ("1/2", 2.0),
    ("1/4", 1.0),
    ("1/8", 0.5),
    ("1/16", 0.25),
    ("1/8T", 1.0 / 3.0),
    ("1/16T", 1.0 / 6.0),
)

# The seconds a voice's volume takes to ramp from silent to full, and back
# after the mouse is released.
ATTACK_SECONDS = 0.01
RELEASE_SECONDS = 0.05


@dataclasses.dataclass
class Controls:
    """What the player has set, all numbers so processes can share them."""

    x: float = 0.0          # 0 left to 1 right, the brightness
    y: float = 0.0          # 0 bottom to 1 top, the root note
    held: bool = False      # the mouse button is down on the pad
    presses: int = 0        # how many times it went down
    chord: int = 0          # index into CHORDS
    snap: bool = False      # the root moves in whole semitones
    arpeggio: bool = False  # the chord's notes play one at a time
    tempo: float = 120.0    # the arpeggio's beats per minute
    note_value: int = 4     # index into NOTE_VALUES, each step's length
    level1: float = 1.0     # each chord note's volume, 0 to 1, from the root
    level2: float = 1.0
    level3: float = 1.0
    level4: float = 1.0

    def levels(self) -> tuple:
        """Returns the chord notes' volumes, from the root up."""
        return self.level1, self.level2, self.level3, self.level4

    def to_list(self) -> list:
        """Returns the fields as floats, in order."""
        return [float(value) for value in dataclasses.astuple(self)]

    @classmethod
    def from_list(cls, values: list) -> "Controls":
        """Returns the Controls to_list() made values from."""
        return cls(*[
            field.type(value)
            for field, value in zip(dataclasses.fields(cls), values)])


def note_frequency(note: float) -> float:
    """Returns the frequency in Hz of a MIDI note number, A4 = 69 = 440 Hz."""
    return 440.0 * 2.0 ** ((note - 69.0) / 12.0)


def root_note(y: float, snap: bool) -> float:
    """Returns the root's MIDI note at height y, 0 to 1, on the pad."""
    note = LOW_NOTE + y * (HIGH_NOTE - LOW_NOTE)

    if snap:
        # The nearest semitone inside the range.
        return float(max(SEMITONES[0], min(round(note), SEMITONES[-1])))

    return note


def mix(low: float, high: float, amount: float) -> float:
    """Returns the value amount, 0 to 1, of the way from low to high."""
    return low + amount * (high - low)


def approach(gain: float, target: float, max_change: float) -> float:
    """Returns gain moved toward target by at most max_change."""
    # The small number keeps a rounding error from leaving a faded voice just
    # above silent.
    if abs(target - gain) > max_change + 1e-9:
        return gain + math.copysign(max_change, target - gain)

    return target


class Voice:
    """One note of the chord: bebot's pulsed, decaying sine."""

    def __init__(self) -> None:
        """Creates the generators, silent."""
        self.carrier = ns.Sine(SR)
        self.carrier.setRealtime(True)

        self.shape = ns.Triangle(SR, 0.02, 0.02)
        self.shape.setRealtime(True)

        self.pulse = ns.GeneratorDecay(SR)
        self.pulse.setRealtime(True)
        self.pulse.addSlaveSync(self.carrier)

    def reset(self) -> None:
        """Starts the generators over."""
        self.carrier.reset()
        self.pulse.reset()
        self.shape.reset()

    def render(
        self,
        duration: float,
        frequencies: ns.Buffer,
        carrier_frequencies: ns.Buffer,
        decays: ns.Buffer,
    ) -> ns.Buffer:
        """Returns the next duration seconds of the note, within -1 to 1."""
        # The pulse goes first, it tells the carrier when to start over.
        sound = self.pulse.generate2(duration, frequencies, decays)
        sound *= self.carrier.generate(duration, carrier_frequencies)
        sound += 1.0
        sound *= self.shape.generate(duration, frequencies)
        sound /= 2.0

        return sound


class Synth:
    """Renders the chord or arpeggio the Controls ask for.

    Every change is ramped across a chunk or longer, so it doesn't click.
    """

    def __init__(self) -> None:
        """Creates the voices, silent."""
        self.lines = ns.Generator(SR)
        self.voices = [Voice() for _ in range(N_VOICES)]

        self.low_pass = ns.FilterLowPassIIR(SR, 6, 80, 0.00)
        self.low_pass.setRealtime(True)

        # Where the last chunk ended, the next one ramps from there.
        self.gains = [0.0] * N_VOICES
        self.frequencies = [0.0] * N_VOICES
        self.x = 0.0

        self.presses = 0
        # The notes the arpeggio has stepped through since the press.
        self.arpeggio_position = 0.0

    def is_silent(self) -> bool:
        """Returns True when every voice has faded out."""
        return not any(self.gains)

    def render(self, controls: Controls, duration: float) -> ns.Buffer:
        """Returns the next duration seconds of sound."""
        pressed = controls.presses != self.presses
        self.presses = controls.presses

        if pressed:
            self.arpeggio_position = 0.0

        # A press released before this chunk still sounds for this chunk.
        sounding = controls.held or pressed

        # Only a press starts over: notes at level 0 are silent while held.
        if pressed and self.is_silent():
            for voice in self.voices:
                voice.reset()

            self.low_pass.reset()
            self.x = controls.x

        offsets = CHORDS[controls.chord][1]
        root = root_note(controls.y, controls.snap)

        total = self.lines.drawLine(duration, 0.0, 0.0)
        n_samples = total.getLength()

        # The chunk in parts, each a length in samples and the volume each
        # voice heads for: the whole chunk, or one part for each arpeggio step.
        if not sounding:
            parts = [(n_samples, [0.0] * N_VOICES)]
        elif controls.arpeggio:
            parts = self._arpeggio_parts(controls, n_samples)
        else:
            # A note's share of the volume doesn't depend on the others'
            # levels.
            shares = [0.0] * N_VOICES

            for i in range(len(offsets)):
                shares[i] = controls.levels()[i] / len(offsets)

            parts = [(n_samples, shares)]

        ramp_seconds = ATTACK_SECONDS if sounding else RELEASE_SECONDS

        for i, voice in enumerate(self.voices):
            # After the release the notes keep their pitch while they fade.
            frequency = self.frequencies[i]

            if sounding:
                offset = offsets[i] if i < len(offsets) else 0
                frequency = note_frequency(root + offset)

            # A voice that stays silent isn't rendered, which saves its time.
            if not (self.gains[i] or any(targets[i] for _, targets in parts)):
                self.frequencies[i] = frequency
                continue

            # A silent voice starts on its note, it doesn't glide to it.
            start = self.frequencies[i] if self.gains[i] else frequency

            sound = voice.render(
                duration,
                self.lines.drawLine(duration, start, frequency),
                self._brightness_line(duration, 200.0, 8000.0, controls.x),
                self._brightness_line(duration, 6.0, 36.0, controls.x))

            # The volume ramps toward each part's target, from the part's
            # first sample.
            volumes = ns.Buffer()
            gain = self.gains[i]

            for length, targets in parts:
                seconds = length / SR
                end = approach(gain, targets[i], seconds / ramp_seconds)
                volumes << self.lines.drawLine(seconds, gain, end)
                gain = end

            sound *= volumes
            total += sound

            self.gains[i] = gain
            self.frequencies[i] = frequency

        cutoffs = self._brightness_line(duration, 1000.0, 7000.0, controls.x)
        self.x = controls.x

        return 0.333 * self.low_pass.filter(total, cutoffs)

    def _arpeggio_parts(self, controls: Controls, n_samples: int) -> list:
        """Splits n_samples where the arpeggio steps, moving it on."""
        levels = controls.levels()
        n_notes = len(CHORDS[controls.chord][1])

        beats = NOTE_VALUES[controls.note_value][1]
        steps_per_sample = controls.tempo / 60.0 / beats / SR

        parts = []

        while n_samples:
            # The small numbers keep a rounding error from delaying a step.
            step = int(self.arpeggio_position + 1e-6)
            to_next = (step + 1 - self.arpeggio_position) / steps_per_sample
            length = min(n_samples, math.ceil(to_next - 1e-3))

            note = step % n_notes
            targets = [0.0] * N_VOICES
            targets[note] = levels[note]
            parts.append((length, targets))

            self.arpeggio_position += length * steps_per_sample
            n_samples -= length

        return parts

    def _brightness_line(
        self, duration: float, low: float, high: float, x: float
    ) -> ns.Buffer:
        """Returns a parameter's ramp from the last chunk's x to this x."""
        return self.lines.drawLine(
            duration, mix(low, high, self.x), mix(low, high, x))
