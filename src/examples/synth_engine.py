"""synth.py's chord synthesizer: turns the controls into sound.

The window writes the Controls, the sound's process reads them and renders the
sound a chunk at a time with Synth.  Nothing here uses pygame or a sound card.
Bebot's voices are generated here, the other instruments' rendered notes are
played by NotePlayer.
"""

import dataclasses
import math

import Nsound as ns

import synth_chorus
import synth_instruments as instruments

SR = instruments.SR

# The pad's lowest root note in Hz, at its bottom, and in MIDI note numbers.
# The highest, at its top, is the instrument's high_frequency.
LOW_FREQUENCY = 40.0
LOW_NOTE = 69.0 + 12.0 * math.log2(LOW_FREQUENCY / 440.0)

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

# The seconds a voice's volume takes to ramp between silent and full without
# clicking, and the envelope's release at the start.
ATTACK_SECONDS = 0.01
RELEASE_SECONDS = 0.05
# The steps in a semitone of a grain's pitch, and in a sample of its start.
PITCH_STEPS = 16
START_STEPS = 8

# The envelope's shortest ramp, a shorter one would click.
SHORTEST_RAMP = 0.002
# The part of each step a held instrument's arpeggio of one note sounds for, so
# its repeats are heard.
ONE_NOTE_GATE = 0.5

# The longest envelope times.
LONGEST_ATTACK = 2.0
LONGEST_DECAY = 2.0
LONGEST_RELEASE = 3.0

# The filter's cutoff range.  FilterLowPassMoogVcf blows up above it.
LOW_CUTOFF = 100.0
HIGH_CUTOFF = 12000.0
# Its resonance's limit, from 0.9 on it rings by itself.
MAX_RESONANCE = 0.85


@dataclasses.dataclass
class Controls:
    """What the player has set, all numbers so processes can share them."""

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
    instrument: int = 0     # index into instruments.INSTRUMENTS
    param1: float = 0.0     # the instrument's own controls' values, Bebot's
                            # first is its brightness, 0 to 1
    param2: float = 0.0
    param3: float = 0.0
    param4: float = 0.0
    attack: float = 0.0     # the envelope's seconds to rise to full,
    decay: float = 0.0      # its seconds to fall to the sustain level,
    sustain: float = 1.0    # that level, 0 to 1,
    release: float = RELEASE_SECONDS  # and its seconds to fade out
    cutoff: float = HIGH_CUTOFF   # the low pass filter's, in Hz
    resonance: float = 0.0  # and its resonance, 0 to MAX_RESONANCE
    normalize: bool = True  # rendered notes' loudness is evened out
    chorus: bool = False    # the chorus is on, its voices,
    chorus_voices: int = 2
    chorus_detune: float = 12.0   # their detune in cents,
    chorus_rate: float = 0.8      # swings a second,
    chorus_mix: float = 1.0       # volume, 0 to 1,
    chorus_width: float = 0.5     # and spread between the sides, 0 to 1

    def levels(self) -> tuple:
        """Returns the chord notes' volumes, from the root up."""
        return self.level1, self.level2, self.level3, self.level4

    def values(self) -> tuple:
        """Returns the instrument's parameter values."""
        return (
            self.param1, self.param2, self.param3, self.param4)

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


def high_note(instrument: int) -> float:
    """Returns the MIDI note at the top of an instrument's pad."""
    high = instruments.INSTRUMENTS[instrument].high_frequency

    return 69.0 + 12.0 * math.log2(high / 440.0)


def semitones(instrument: int) -> range:
    """Returns the whole semitones on an instrument's pad."""
    return range(math.ceil(LOW_NOTE), math.floor(high_note(instrument)) + 1)


def root_note(
    y: float, snap: bool, instrument: int = instruments.BEBOT
) -> float:
    """Returns the root's MIDI note at height y, 0 to 1, on the pad."""
    note = LOW_NOTE + y * (high_note(instrument) - LOW_NOTE)

    if snap:
        # The nearest semitone inside the range.
        inside = semitones(instrument)

        return float(max(inside[0], min(round(note), inside[-1])))

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


class Envelope:
    """A volume that moves in straight lines through points in time."""

    def __init__(self) -> None:
        """Starts silent."""
        self.level = 0.0
        self.time = 0               # in samples, from the first point
        self.points = [(0, 0.0)]    # each (time, level)

    def trigger(self, attack: float, decay: float, sustain: float) -> None:
        """Rises from the level now to full, then falls to sustain."""
        self._head((attack, 1.0), (decay, sustain))

    def release(self, seconds: float) -> None:
        """Falls from the level now to silence."""
        self._head((seconds, 0.0))

    def finished(self) -> bool:
        """Returns True when it has fallen silent for good."""
        end, level = self.points[-1]

        return level == 0.0 and self.time >= end

    @classmethod
    def hit(cls, controls: Controls, n_samples: float) -> "Envelope":
        """Returns a struck note's envelope: attack, decay to sustain, and the
        release at the end of its n_samples.
        """
        envelope = cls()

        # No attack is the note's own, which a ramp would soften.
        attack = round(controls.attack * SR)
        decay = round(max(controls.decay, SHORTEST_RAMP) * SR)
        envelope.points = [(0, 1.0 if attack == 0 else 0.0)]

        if attack:
            envelope.points.append((attack, 1.0))

        envelope.points.append((attack + decay, controls.sustain))

        end = max(1, int(n_samples))
        release = round(max(controls.release, SHORTEST_RAMP) * SR)
        # A release longer than the note starts when the attack ends.
        knee = max(end - release, min(attack, end - 1))

        level = envelope._level_at(knee)
        envelope.points = [p for p in envelope.points if p[0] < knee]
        envelope.points += [(knee, level), (end, 0.0)]
        envelope.level = envelope.points[0][1]

        return envelope

    def read(self, n_samples: int) -> ns.Buffer:
        """Returns the next n_samples of the volume."""
        line = ns.Buffer()
        lines = ns.Generator(SR)

        while n_samples:
            # Up to the next point, or all the rest after the last.
            later = [time for time, _ in self.points if time > self.time]
            length = n_samples

            if later:
                length = min(n_samples, later[0] - self.time)

            end = self._level_at(self.time + length)
            line << lines.drawLine(
                length / SR, self._level_at(self.time), end)

            self.time += length
            self.level = end
            n_samples -= length

        return line

    def _head(self, *ramps: tuple) -> None:
        """Starts ramps, each (seconds, level), from the level now."""
        self.time = 0
        self.points = [(0, self.level)]

        for seconds, level in ramps:
            length = round(max(seconds, SHORTEST_RAMP) * SR)
            self.points.append((self.points[-1][0] + length, level))

    def _level_at(self, time: float) -> float:
        """Returns the volume at time."""
        for (start, low), (end, high) in zip(self.points, self.points[1:]):
            if time < end:
                return mix(low, high, (time - start) / (end - start))

        return self.points[-1][1]


@dataclasses.dataclass
class Playing:
    """A rendered note being played."""

    note: instruments.Note
    position: float     # the next sample, between two when bent
    gain: float         # where the volume's ramp has reached
    volume: float       # where it heads
    presses: int        # the press it started in
    number: int         # the MIDI note it was rendered at
    # Samples read for each one played, 1 is the rendered pitch, and where
    # it heads over the next read.
    rate: float = 1.0
    end_rate: float = 1.0

    # A struck note's volume over its length.
    envelope: "Envelope" = None

    def bend(self, pitch: float) -> None:
        """Heads for the MIDI pitch, by playing faster or slower."""
        self.end_rate = 2.0 ** ((pitch - self.number) / 12.0)

    def read(self, n_samples: int) -> list:
        """Returns each channel's next n_samples, silent after the end."""
        if (self.rate == self.end_rate == 1.0
                and self.position == int(self.position)):
            out = self._samples(int(self.position), n_samples)
            self._advance(n_samples)

            return out

        # The rate ramps to its end, samples between two are interpolated.
        change = (self.end_rate - self.rate) / n_samples
        start = int(self.position)
        offsets = []
        offset = self.position - start

        for i in range(n_samples):
            offsets.append(offset)
            offset += self.rate + change * i

        spans = [
            span.toList()
            for span in self._samples(start, int(offsets[-1]) + 2)]

        out = [
            ns.Buffer([
                span[int(o)] + (span[int(o) + 1] - span[int(o)]) * (o % 1.0)
                for o in offsets])
            for span in spans]

        self._advance(start + offset - self.position)
        self.rate = self.end_rate

        return out

    def _advance(self, n_samples: float) -> None:
        """Moves the position on, around a repeating note's end."""
        self.position += n_samples
        length = self.note.channels[0].getLength()

        if self.note.loop_start is not None:
            while self.position > length:
                self.position -= length - self.note.loop_start

    def _samples(self, position: int, n_samples: int) -> list:
        """Returns each channel's n_samples from position, as the note plays:
        repeating, or silent after its end.
        """
        channels = self.note.channels
        out = [ns.Buffer() for _ in channels]

        while n_samples:
            if position >= channels[0].getLength():
                if self.note.loop_start is None:
                    for part in out:
                        part << ns.Buffer.zeros(n_samples)

                    break

                position = self.note.loop_start

            length = min(
                n_samples, channels[0].getLength() - position)

            for part, samples in zip(out, channels):
                part << samples.subbuffer(position, length)

            position += length
            n_samples -= length

        return out

    def finished(self) -> bool:
        """Returns True after the last sample of a note that doesn't repeat."""
        return (self.note.loop_start is None
            and self.position >= self.note.channels[0].getLength())


class NotePlayer:
    """Plays rendered notes for the chord or arpeggio the Controls ask for.

    A held instrument's notes sound while the mouse is held.  Another's are
    hit once each note value while held, and ring out when it is released.
    """

    def __init__(self, notes: instruments.NoteCache) -> None:
        """Creates a silent player getting its notes from notes."""
        self.notes = notes
        self.clear()

    def clear(self) -> None:
        """Stops every note."""
        # The note of each of the chord's positions, and those fading out.
        self.playing = [None] * N_VOICES
        self.fading = []
        # What last started each position's note.
        self.triggers = [None] * N_VOICES
        # A struck note waiting to be rendered: (key, volume, pitch).
        self.waiting = [None] * N_VOICES

    def is_silent(self) -> bool:
        """Returns True when no note sounds or is about to."""
        return not (any(self.playing) or self.fading or any(self.waiting))

    def render(
        self, controls: Controls, parts: list, sounding: bool
    ) -> list:
        """Returns the left and right sound of the chunk's parts."""
        spec = instruments.INSTRUMENTS[controls.instrument]
        held = spec.held
        offsets = CHORDS[controls.chord][1]
        keys = self._keys(controls)
        # Each position's MIDI pitch, between semitones when not snapping.
        root = root_note(controls.y, controls.snap, controls.instrument)
        pitches = [root + offset for offset in offsets]

        # Every note of the chord is asked for, ready for the arpeggio.
        if sounding:
            for key in keys:
                self.notes.get(key)

        total = [ns.Buffer(), ns.Buffer()]

        for length, targets, step in parts:
            for i in range(N_VOICES):
                wanted = (sounding and i < len(offsets)
                    and (not controls.arpeggio or step % len(offsets) == i))

                if wanted:
                    self._trigger(
                        i, keys[i], pitches[i], step, targets[i], controls,
                        held)

                self._start_waiting(i, controls)

                note = self.playing[i]

                if note is None:
                    continue

                # A struck note rings on after the release.
                if sounding:
                    note.volume = targets[i]

                # A held note glides with the mouse when not snapping, even
                # while the note nearer its new pitch is being rendered.
                if (held and sounding and i < len(offsets)
                        and not controls.snap):
                    note.bend(pitches[i])


            for side, part in zip(total, self._mix(length)):
                side << part

        return total

    def _keys(self, controls: Controls) -> list:
        """Returns the keys of the chord's notes."""
        spec = instruments.INSTRUMENTS[controls.instrument]
        root = int(root_note(controls.y, True, controls.instrument))

        n_samples = 0

        if not spec.held:
            beats = NOTE_VALUES[controls.note_value][1]
            n_samples = round(beats * 60.0 / controls.tempo * SR)

        return [
            instruments.NoteKey(
                controls.instrument, root + offset, n_samples,
                controls.values(), controls.normalize)
            for offset in CHORDS[controls.chord][1]]

    def _trigger(
        self, i: int, key: instruments.NoteKey, pitch: float, step: int,
        volume: float,
        controls: Controls, held: bool,
    ) -> None:
        """Starts position i's note, if it is a new one."""
        trigger = (key, step, controls.presses)
        last = self.triggers[i]

        if trigger == last:
            return

        # A struck note is hit once a step: a change of pitch or sound is in
        # the next step's hit.
        if not held and last and last[1:] == trigger[1:]:
            return

        if not held:
            self.waiting[i] = (key, volume, pitch)
            self.triggers[i] = trigger
            return

        # Until the new note is rendered the old one goes on.
        note = self.notes.get(key)

        if note is None:
            return

        # A held note that changes pitch goes on from the repeating part, one
        # that starts has its attack.
        old = self.playing[i]
        position = 0

        if old and old.presses == controls.presses and step is None:
            position = note.loop_start

        playing = Playing(
            note, position, 0.0, volume, controls.presses, key.note)

        # It starts at the pitch, without a glide from the rendered one.
        playing.bend(pitch)
        playing.rate = playing.end_rate

        self._replace(i, playing)
        self.triggers[i] = trigger

    def _start_waiting(self, i: int, controls: Controls) -> None:
        """Starts position i's struck note once it is rendered."""
        if self.waiting[i] is None:
            return

        key, volume, pitch = self.waiting[i]
        note = self.notes.get(key)

        if note is None:
            return

        # At full volume from the first sample, to keep the attack.
        playing = Playing(
            note, 0, volume, volume, controls.presses, key.note)

        # A struck note keeps the pitch it was struck at.
        playing.bend(pitch)
        playing.rate = playing.end_rate
        playing.envelope = Envelope.hit(
            controls, note.channels[0].getLength() / playing.rate)

        self._replace(i, playing)
        self.waiting[i] = None

    def _replace(self, i: int, note: Playing) -> None:
        """Plays note at position i, fading out the one there."""
        if self.playing[i]:
            self.playing[i].volume = 0.0
            self.fading.append(self.playing[i])

        self.playing[i] = note

    def _mix(self, length: int) -> list:
        """Returns each side's next length samples, drops ended notes."""
        total = [ns.Buffer.zeros(length), ns.Buffer.zeros(length)]
        lines = ns.Generator(SR)
        seconds = length / SR

        def play(note: Playing) -> bool:
            """Adds the note to total, returns True if it goes on."""
            end = approach(note.gain, note.volume, seconds / ATTACK_SECONDS)
            volumes = lines.drawLine(seconds, note.gain, end)

            if note.envelope:
                volumes *= note.envelope.read(length)

            channels = [c * volumes for c in note.read(length)]

            # A note with one channel sounds the same on both sides.
            for i in range(2):
                total[i] += channels[min(i, len(channels) - 1)]

            note.gain = end

            return not (note.finished() or end == note.volume == 0.0)

        for i, note in enumerate(self.playing):
            if note and not play(note):
                self.playing[i] = None

        self.fading = [note for note in self.fading if play(note)]

        return total


class GrainPlayer:
    """Plays the Granulator: a stream of grains for each of the chord's notes.

    Each grain takes the pitch and settings of the moment it starts, and is
    mixed into its position's sound then, however long it is.
    """

    def __init__(self) -> None:
        """Creates a silent player."""
        self.clear()

    def clear(self) -> None:
        """Stops every grain."""
        # For each of the chord's positions: the sound of its grains, how far
        # that is played, the samples until the next grain starts, the newest
        # grain, its volume and its pitch.
        self.mixed = [ns.Buffer() for _ in range(N_VOICES)]
        self.played = [0] * N_VOICES
        self.waits = [0.0] * N_VOICES
        self.newest = [None] * N_VOICES
        self.gains = [0.0] * N_VOICES
        self.pitches = [0.0] * N_VOICES
        # The volume that keeps lined up grains within full scale.
        self.limit = 1.0

    def is_silent(self) -> bool:
        """Returns True when no grain is left to play."""
        return all(
            mixed.getLength() <= played
            for mixed, played in zip(self.mixed, self.played))

    def render(
        self, controls: Controls, parts: list, sounding: bool
    ) -> ns.Buffer:
        """Returns the sound of the chunk's parts."""
        if sounding:
            root = root_note(
                controls.y, controls.snap, controls.instrument)

            # The pitches are kept for the release.
            for i, offset in enumerate(CHORDS[controls.chord][1]):
                self.pitches[i] = root + offset

        total = ns.Buffer()

        for length, targets, _ in parts:
            part = ns.Buffer.zeros(length)

            for i in range(N_VOICES):
                gain = approach(
                    self.gains[i], targets[i], length / SR / ATTACK_SECONDS)

                if self.gains[i] or gain:
                    sound = self._voice(i, controls, length)
                    sound *= ns.Generator(SR).drawLine(
                        length / SR, self.gains[i], gain)
                    part += sound
                    self.gains[i] = gain
                else:
                    # A silent position starts on a grain when it next sounds.
                    self.mixed[i] = ns.Buffer()
                    self.played[i] = 0
                    self.waits[i] = 0.0

            total << part

        return self._limited(total)

    def _voice(self, i: int, controls: Controls, length: int) -> ns.Buffer:
        """Returns position i's next length samples."""
        rate, follows, waves, shape = controls.values()

        # In steps too fine to hear, so grains are reused.
        pitch = round(self.pitches[i] * PITCH_STEPS) / PITCH_STEPS
        frequency = note_frequency(pitch)

        # Grains faster than their pitch cancel each other out.
        if follows or rate > frequency:
            rate = frequency

        while self.waits[i] < length:
            # The grain starts between samples, to the nearest step, so grains
            # at a high rate are evenly spaced.
            start = round(max(0.0, self.waits[i]) * START_STEPS)
            sample, step = divmod(start, START_STEPS)

            grain = instruments.render_grain(
                frequency, round(waves), round(shape), step / START_STEPS)

            # More grains at once are louder, though not in proportion.
            overlap = rate * grain.getLength() / SR

            self.mixed[i].add(
                grain * (1.0 / max(1.0, overlap) ** 0.5),
                self.played[i] + sample)
            self.newest[i] = grain

            self.waits[i] += SR / rate

        self.waits[i] -= length

        start = self.played[i]
        n_samples = max(0, min(length, self.mixed[i].getLength() - start))
        sound = ns.Buffer()

        if n_samples:
            sound << self.mixed[i].subbuffer(start, n_samples)

        if n_samples < length:
            sound << ns.Buffer.zeros(length - n_samples)

        self.played[i] += length
        self._forget(i)

        return sound

    def _forget(self, i: int) -> None:
        """Drops position i's played samples, now and then: it copies."""
        length = self.mixed[i].getLength()

        if self.played[i] >= length:
            self.mixed[i] = ns.Buffer()
            self.played[i] = 0

        elif self.played[i] >= SR:
            self.mixed[i] = self.mixed[i].subbuffer(self.played[i])
            self.played[i] = 0

    def _limited(self, sound: ns.Buffer) -> ns.Buffer:
        """Returns sound turned down where it would pass full scale: grains
        lined up with each other add up.
        """
        loudest = sound.getAbs().getMax()
        limit = min(1.0, 1.0 / loudest) if loudest else 1.0

        # Down at once, back up over about half a second.
        start = min(limit, self.limit)
        end = min(limit, start + 2.0 * sound.getLength() / SR)
        sound *= ns.Generator(SR).drawLine(
            sound.getLength() / SR, start, end)
        self.limit = end

        return sound


class OscillatorPlayer:
    """Plays the Piano: an oscillator for each of the chord's notes."""

    def __init__(self) -> None:
        """Creates silent oscillators."""
        self.lines = ns.Generator(SR)
        self.clear()

    def clear(self) -> None:
        """Stops every note."""
        # Each of the chord's positions: its generator and the wave and
        # harmonics it was made with, and where its last chunk ended.
        self.generators = [None] * N_VOICES
        self.made = [None] * N_VOICES
        self.frequencies = [0.0] * N_VOICES
        self.gains = [0.0] * N_VOICES

    def is_silent(self) -> bool:
        """Returns True when every note has faded out."""
        return not any(self.gains)

    def render(
        self, controls: Controls, parts: list, sounding: bool
    ) -> ns.Buffer:
        """Returns the sound of the chunk's parts."""
        offsets = CHORDS[controls.chord][1]
        root = root_note(controls.y, controls.snap, controls.instrument)
        wave, harmonics = (round(value) for value in controls.values()[:2])
        n_samples = sum(length for length, _, _ in parts)
        total = ns.Buffer.zeros(n_samples)

        for i in range(N_VOICES):
            heard = any(targets[i] for _, targets, _ in parts)

            if not (self.gains[i] or heard):
                continue

            if self.made[i] != (wave, harmonics):
                self.generators[i] = instruments.oscillator(wave, harmonics)
                self.made[i] = (wave, harmonics)

            # After the release the notes keep their pitch while they fade.
            frequency = self.frequencies[i]

            if sounding and i < len(offsets):
                frequency = note_frequency(root + offsets[i])

            # A silent note starts on its pitch, it doesn't glide to it.
            start = self.frequencies[i] if self.gains[i] else frequency

            sound = self.generators[i].generate(
                n_samples / SR,
                self.lines.drawLine(n_samples / SR, start, frequency))

            volumes = ns.Buffer()
            gain = self.gains[i]

            for length, targets, _ in parts:
                seconds = length / SR
                end = approach(gain, targets[i], seconds / ATTACK_SECONDS)
                volumes << self.lines.drawLine(seconds, gain, end)
                gain = end

            total += sound * volumes
            self.gains[i] = gain
            self.frequencies[i] = frequency

        return total


def envelope_settings(controls: Controls) -> tuple:
    """Returns the held envelope's attack, decay, sustain and release: the
    instrument's own while its check box is on, else the Controls'.
    """
    spec = instruments.INSTRUMENTS[controls.instrument]

    if spec.envelope and controls.values()[len(spec.parameters) - 1]:
        return spec.envelope

    return (
        controls.attack, controls.decay, controls.sustain, controls.release)


def stereo(left: ns.Buffer, right: ns.Buffer) -> ns.AudioStream:
    """Returns the two sides as an AudioStream."""
    sound = ns.AudioStream(SR, 2)
    sound[0] = left
    sound[1] = right

    return sound


class Synth:
    """Plays the chord or arpeggio the Controls ask for.

    Every change is ramped across a chunk or longer, so it doesn't click.
    """

    def __init__(self, notes: instruments.NoteCache = None) -> None:
        """Creates the voices, silent.

        notes renders the other instruments' notes, by default at once.
        """
        self.lines = ns.Generator(SR)
        self.voices = [Voice() for _ in range(N_VOICES)]
        self.player = NotePlayer(notes or instruments.NoteCache())
        self.granulator = GrainPlayer()
        self.oscillators = OscillatorPlayer()
        self.chorus = synth_chorus.Chorus()

        self.low_pass = ns.FilterLowPassIIR(SR, 6, 80, 0.00)
        self.low_pass.setRealtime(True)

        # Where Bebot's last chunk ended, the next one ramps from there.
        self.gains = [0.0] * N_VOICES
        self.frequencies = [0.0] * N_VOICES
        self.brightness = 0.0

        self.presses = 0
        # The note values passed since the press, for the arpeggio and the
        # struck instruments' beat.
        self.arpeggio_position = 0.0

        # The held instruments' envelope, whether the mouse was down for the
        # last chunk, its arpeggio step and the volumes it headed for.
        self.envelope = Envelope()
        self.gate = False
        self.step = None
        self.targets = [0.0] * N_VOICES

        # The low pass filter of each side, and where the last chunk's cutoff
        # ended, None while the filter is off.
        self.filters = None
        self.cutoff = None

    def is_silent(self) -> bool:
        """Returns True when every note has ended."""
        return (not any(self.gains) and self.player.is_silent()
            and self.granulator.is_silent() and self.oscillators.is_silent()
            and self.chorus.is_silent())

    def render(self, controls: Controls, duration: float) -> ns.AudioStream:
        """Returns the next duration seconds of sound, in stereo."""
        pressed = controls.presses != self.presses
        self.presses = controls.presses

        if pressed:
            self.arpeggio_position = 0.0

        # A press released before this chunk still sounds for this chunk.
        sounding = controls.held or pressed

        spec = instruments.INSTRUMENTS[controls.instrument]
        held = spec.held

        if self.gate and not sounding:
            self.envelope.release(envelope_settings(controls)[3])

        self.gate = sounding

        total = self.lines.drawLine(duration, 0.0, 0.0)
        n_samples = total.getLength()

        # A held instrument's notes go on while the envelope releases.
        if held and not sounding and not self.envelope.finished():
            parts = [(n_samples, self.targets, None)]
        else:
            parts = self._parts(controls, sounding, n_samples)

        if sounding:
            self.targets = parts[-1][1]

        # Changing between the kinds of instrument stops the one left.
        if controls.instrument != instruments.BEBOT:
            self.gains = [0.0] * N_VOICES

        if spec.granular or spec.oscillator:
            self.player.clear()

        if not spec.granular:
            self.granulator.clear()

        if not spec.oscillator:
            self.oscillators.clear()

        if controls.instrument == instruments.BEBOT:
            self.player.clear()

            left = self._render_bebot(
                controls, duration, parts, pressed, sounding)
            right = left
        elif spec.granular:
            left = 0.333 * self.granulator.render(controls, parts, sounding)
            right = left * 1.0
        elif spec.oscillator:
            left = 0.333 * self.oscillators.render(controls, parts, sounding)
            right = left * 1.0
        else:
            left, right = self.player.render(controls, parts, sounding)
            left, right = 0.333 * left, 0.333 * right

        # Struck notes have their own envelopes.
        if held:
            volume = self._envelope_line(controls, parts, pressed, sounding)
            left, right = left * volume, right * volume

            if not sounding and self.envelope.finished():
                self.gains = [0.0] * N_VOICES
                self.player.clear()
                self.granulator.clear()
                self.oscillators.clear()

        left, right = self._filter(controls, left, right)

        return stereo(*self.chorus.process(
            left, right, controls.chorus, controls.chorus_voices,
            controls.chorus_detune, controls.chorus_rate,
            controls.chorus_mix, controls.chorus_width))

    def _envelope_line(
        self, controls: Controls, parts: list, pressed: bool, sounding: bool
    ) -> ns.Buffer:
        """Returns the held instruments' volume over the chunk's parts."""
        line = ns.Buffer()

        for i, (length, _, step) in enumerate(parts):
            # The envelope starts at the press and at each arpeggio step.
            if sounding and ((pressed and i == 0) or step != self.step):
                self.envelope.trigger(*envelope_settings(controls)[:3])
                self.step = step

            line << self.envelope.read(length)

        return line

    def _filter(
        self, controls: Controls, left: ns.Buffer, right: ns.Buffer
    ) -> tuple:
        """Returns the two sides through the low pass filter, if it is on."""
        if controls.cutoff >= HIGH_CUTOFF and controls.resonance == 0.0:
            self.filters = None
            return left, right

        if self.filters is None:
            self.filters = [
                ns.FilterLowPassMoogVcf(
                    SR, controls.cutoff, controls.resonance)
                for _ in range(2)]
            self.cutoff = controls.cutoff

        cutoffs = self.lines.drawLine(
            left.getLength() / SR, self.cutoff, controls.cutoff).toList()
        self.cutoff = controls.cutoff

        out = []

        for low_pass, side in zip(self.filters, (left, right)):
            # A louder sound than full scale would blow the filter up.
            side.limit(-1.0, 1.0)

            # Sample by sample, the only way the filter takes a resonance.
            out.append(ns.Buffer([
                low_pass.filter(sample, cutoff, controls.resonance)
                for sample, cutoff in zip(side.toList(), cutoffs)]))

        return tuple(out)

    def _parts(
        self, controls: Controls, sounding: bool, n_samples: int
    ) -> list:
        """Returns the chunk in parts: the whole of it, or one for each step
        of the arpeggio, or of a struck instrument's beat.

        Each is (length in samples, the volume each voice heads for, the
        step's number or None).
        """
        if not sounding:
            return [(n_samples, [0.0] * N_VOICES, None)]

        held = instruments.INSTRUMENTS[controls.instrument].held

        if controls.arpeggio:
            # A chord's notes follow each other without a rest.  A struck
            # note rings as it will, it is hit again at each step.
            gate = 1.0

            if held and len(CHORDS[controls.chord][1]) == 1:
                gate = ONE_NOTE_GATE

            silent = [0.0] * N_VOICES

            return [
                (length,
                    self._arpeggio_targets(controls, step) if on else silent,
                    step)
                for length, step, on in self._steps(controls, n_samples, gate)]

        # A note's share of the volume doesn't depend on the others' levels.
        n_notes = len(CHORDS[controls.chord][1])
        shares = [0.0] * N_VOICES

        for i in range(n_notes):
            shares[i] = controls.levels()[i] / n_notes

        if held:
            return [(n_samples, shares, None)]

        # A struck instrument hits again at each step while held.
        return [
            (length, shares, step)
            for length, step, _ in self._steps(controls, n_samples)]

    def _arpeggio_targets(self, controls: Controls, step: int) -> list:
        """Returns the volumes at the arpeggio's step: one note's."""
        note = step % len(CHORDS[controls.chord][1])
        targets = [0.0] * N_VOICES
        targets[note] = controls.levels()[note]

        return targets

    def _render_bebot(
        self, controls: Controls, duration: float, parts: list,
        pressed: bool, sounding: bool,
    ) -> ns.Buffer:
        """Returns Bebot's next duration seconds of sound."""
        # Only a press starts over: notes at level 0 are silent while held.
        if pressed and not any(self.gains):
            for voice in self.voices:
                voice.reset()

            self.low_pass.reset()
            self.brightness = controls.param1

        offsets = CHORDS[controls.chord][1]
        root = root_note(controls.y, controls.snap, controls.instrument)

        total = self.lines.drawLine(duration, 0.0, 0.0)

        for i, voice in enumerate(self.voices):
            # After the release the notes keep their pitch while they fade.
            frequency = self.frequencies[i]

            if sounding:
                offset = offsets[i] if i < len(offsets) else 0
                frequency = note_frequency(root + offset)

            # A voice that stays silent isn't rendered, which saves its time.
            heard = any(targets[i] for _, targets, _ in parts)

            if not (self.gains[i] or heard):
                self.frequencies[i] = frequency
                continue

            # A silent voice starts on its note, it doesn't glide to it.
            start = self.frequencies[i] if self.gains[i] else frequency

            sound = voice.render(
                duration,
                self.lines.drawLine(duration, start, frequency),
                self._brightness_line(
                    duration, 200.0, 8000.0, controls.param1),
                self._brightness_line(duration, 6.0, 36.0, controls.param1))

            # The volume ramps toward each part's target, from the part's
            # first sample.
            volumes = ns.Buffer()
            gain = self.gains[i]

            for length, targets, _ in parts:
                seconds = length / SR
                end = approach(gain, targets[i], seconds / ATTACK_SECONDS)
                volumes << self.lines.drawLine(seconds, gain, end)
                gain = end

            sound *= volumes
            total += sound

            self.gains[i] = gain
            self.frequencies[i] = frequency

        cutoffs = self._brightness_line(
            duration, 1000.0, 7000.0, controls.param1)
        self.brightness = controls.param1

        return 0.333 * self.low_pass.filter(total, cutoffs)

    def _steps(
        self, controls: Controls, n_samples: int, gate: float = 1.0
    ) -> list:
        """Splits n_samples where a step of the note value starts, and where
        its first gate part ends.  Returns each part's (length, step number,
        whether it is in the gate), and moves the position on.
        """

        beats = NOTE_VALUES[controls.note_value][1]
        steps_per_sample = controls.tempo / 60.0 / beats / SR

        parts = []

        while n_samples:
            # The small numbers keep a rounding error from delaying a step.
            step = int(self.arpeggio_position + 1e-6)
            on = self.arpeggio_position < step + gate - 1e-6
            end = step + (gate if on else 1.0)
            to_end = (end - self.arpeggio_position) / steps_per_sample
            length = min(n_samples, math.ceil(to_end - 1e-3))

            parts.append((length, step, on))

            self.arpeggio_position += length * steps_per_sample
            n_samples -= length

        return parts

    def _brightness_line(
        self, duration: float, low: float, high: float, brightness: float
    ) -> ns.Buffer:
        """Returns a value's ramp from the last chunk's brightness to this."""
        return self.lines.drawLine(
            duration,
            mix(low, high, self.brightness),
            mix(low, high, brightness))
