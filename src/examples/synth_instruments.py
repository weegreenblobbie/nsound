"""synth.py's instruments: their controls, and rendering their notes.

Bebot's sound is made a chunk at a time by synth_engine.  The other
instruments render a whole note at once, kept to be played again.
"""

import collections
import dataclasses
import functools
import typing

import Nsound as ns

SR = 48000.0

# A held note is this long, and repeats from LOOP_START to near its end for as
# long as it is held.  The repeat's two ends fade into each other.
HELD_SECONDS = 2.0
LOOP_START = int(0.4 * SR)
LOOP_END = int(1.6 * SR)
LOOP_FADE = int(0.05 * SR)
# The samples compared to line the repeat's two ends up.
MATCH_SAMPLES = 512

# A struck note is rendered at least this long, a shorter one is cut from it
# and faded out: the instruments can't render very short notes.
SHORTEST_RENDER = 0.15
CUT_FADE = int(0.005 * SR)

# A normalized note's loudest NORMAL_WINDOW has this RMS, unless that would take
# its peak past LOUDEST_PEAK.
NORMAL_RMS = 0.35
NORMAL_WINDOW = int(0.05 * SR)
LOUDEST_PEAK = 2.0

# The root note in Hz at the top of the pad.
PAD_HIGH_FREQUENCY = 650.0

# The pad's middle, where a Hat has its usual tune.
MIDDLE_FREQUENCY = 161.2


@dataclasses.dataclass(frozen=True)
class Parameter:
    """One of an instrument's own controls."""

    label: str
    low: float
    high: float
    default: float
    # For a choice, its names, the values are their indices.
    names: tuple = ()
    # Its values are whole numbers.
    whole: bool = False
    # A choice of two shown as a check box, checked is the second.
    toggle: bool = False


@dataclasses.dataclass(frozen=True)
class Instrument:
    """An instrument to choose."""

    name: str
    # Sounds for as long as the mouse is held, else for the note value.
    held: bool
    parameters: tuple = ()
    # Returns seconds of a note, a Buffer for each channel: (frequency,
    # seconds, values).  None for Bebot.
    render: typing.Optional[typing.Callable] = None
    # The parameter the pad's X axis sets at the start, None for none.
    x_parameter: typing.Optional[int] = None
    # Whether the root moves in whole semitones at the start.
    snap: bool = True
    # The rendered samples are scaled by this, to match the others' volume.
    volume: float = 1.0
    # The shortest struck note it can render, in seconds.
    shortest: float = SHORTEST_RENDER
    # Built from grains as it plays, by synth_engine.
    granular: bool = False
    # Played from a wave table as it goes, by synth_engine.
    oscillator: bool = False
    # Its own envelope's attack, decay, sustain and release, which its last
    # parameter, a check box, puts in place of the Controls'.
    envelope: tuple = ()
    # The root note in Hz at the top of its pad.
    high_frequency: float = PAD_HIGH_FREQUENCY


def render_clarinet(frequency: float, seconds: float, values: tuple) -> tuple:
    """Returns a Clarinet note, values: pressure, filter, reed stiffness."""
    return (ns.Clarinet(SR).play(seconds, frequency, *values)[0],)


def render_flute(frequency: float, seconds: float, values: tuple) -> tuple:
    """Returns a FluteSlide note, values: pressure, breath."""
    return (ns.FluteSlide(SR).play(seconds, frequency, *values)[0],)


def render_organ(frequency: float, seconds: float, values: tuple) -> tuple:
    """Returns an OrganPipe note in stereo, values: pan, 0 left to 1 right."""
    # OrganPipe's pan is the left channel's share.
    note = ns.OrganPipe(SR).play(seconds, frequency, 1.0 - values[0])

    return note[0], note[1]


def render_guitar_bass(
    frequency: float, seconds: float, values: tuple
) -> tuple:
    """Returns a GuitarBass note, it has no values."""
    return (ns.GuitarBass(SR).play(seconds, frequency)[0],)


def render_bass_drum(frequency: float, seconds: float, values: tuple) -> tuple:
    """Returns a DrumBD01 hit, values: velocity."""
    return (ns.DrumBD01(SR).play(seconds, frequency, values[0])[0],)


def render_kick_drum(frequency: float, seconds: float, values: tuple) -> tuple:
    """Returns a DrumKickBass hit falling to frequency, values: high Hz."""
    drum = ns.DrumKickBass(SR, values[0], frequency)

    return (drum.play(seconds, values[0], frequency)[0],)


def render_hat(frequency: float, seconds: float, values: tuple) -> tuple:
    """Returns a Hat hit tuned by frequency, values: 0 closed, 1 open."""
    tune = frequency / MIDDLE_FREQUENCY

    hit = ns.Hat(SR).play(seconds, tune, values[0] < 0.5)[0]

    # Its loudness grows with the tune, the others' peaks are about 1.
    hit.normalize()

    return (hit,)


def render_pluck(frequency: float, seconds: float, values: tuple) -> tuple:
    """Returns a Pluck note, values: the samples its noise is smoothed over."""
    return (ns.Pluck(SR, round(values[0])).generate(seconds, frequency),)


INSTRUMENTS = (
    Instrument(
        "Bebot",
        held=True,
        parameters=(Parameter("Brightness", 0.0, 1.0, 0.5),),
        x_parameter=0,
        snap=False),
    Instrument(
        "Clarinet",
        held=True,
        parameters=(
            Parameter("Pressure", 1.0, 2.2, 1.8),
            Parameter("Filter Hz", 500.0, 2000.0, 1000.0),
            Parameter("Reed stiffness", 0.1, 0.3, 0.2),
        ),
        render=render_clarinet),
    Instrument(
        "Flute",
        held=True,
        parameters=(
            Parameter("Pressure", 0.7, 1.3, 1.0),
            Parameter("Breath", 0.0, 0.2, 0.05),
        ),
        render=render_flute),
    Instrument(
        "Organ Pipe",
        held=True,
        parameters=(Parameter("Pan", 0.0, 1.0, 0.5),),
        render=render_organ,
        volume=0.4),
    Instrument("Guitar Bass", held=False, render=render_guitar_bass),
    Instrument(
        "Pluck",
        held=False,
        parameters=(Parameter("Smoothing", 1.0, 40.0, 10.0),),
        render=render_pluck),
    Instrument(
        "Bass Drum",
        held=False,
        parameters=(Parameter("Velocity", 0.0, 1.0, 1.0),),
        render=render_bass_drum,
        shortest=0.35),
    Instrument(
        "Kick Drum",
        held=False,
        parameters=(Parameter("High Hz", 60.0, 300.0, 120.0),),
        render=render_kick_drum),
    Instrument(
        "Hat",
        held=False,
        parameters=(
            Parameter("Cymbals", 0.0, 1.0, 0.0, names=("Closed", "Open")),),
        render=render_hat),
    Instrument(
        "Granulator",
        held=True,
        parameters=(
            Parameter("Grains/s", 5.0, 300.0, 40.0, whole=True),
            Parameter(
                "Rate follows pitch", 0.0, 1.0, 0.0, names=("Off", "On"),
                toggle=True),
            Parameter("Waves/grain", 1.0, 30.0, 3.0, whole=True),
            Parameter(
                "Shape", 0.0, 3.0, 1.0,
                names=("Gaussian", "Wide", "Decay", "Reverse")),
        ),
        x_parameter=0,
        granular=True,
        high_frequency=4.0 * PAD_HIGH_FREQUENCY),
    # piano.py's sound.
    Instrument(
        "Piano",
        held=True,
        parameters=(
            Parameter(
                "Wave", 0.0, 2.0, 0.0, names=("Sine", "Saw", "Square")),
            Parameter("Harmonics", 1.0, 30.0, 7.0, whole=True),
            Parameter(
                "Piano envelope", 0.0, 1.0, 1.0, names=("Off", "On"),
                toggle=True),
        ),
        oscillator=True,
        envelope=(0.2, 0.4, 0.333, 0.1)),
)

BEBOT = 0


def oscillator(wave: int, harmonics: int) -> ns.Generator:
    """Returns a new generator of the Piano's wave: 0 a sine, 1 a sawtooth or
    2 a square wave, the last two with harmonics.
    """
    original = wave_generator(wave, harmonics)

    # A copy is much quicker than making the wave table again.
    generator = type(original)(original)
    generator.setRealtime(True)

    return generator


@functools.lru_cache(maxsize=64)
def wave_generator(wave: int, harmonics: int) -> ns.Generator:
    """Returns the generator oscillator() copies, don't play it."""
    if wave == 0:
        return ns.Sine(SR)

    if wave == 1:
        return ns.Sawtooth(SR, harmonics)

    return ns.Square(SR, harmonics)

# The Granulator's grain envelopes, in the order of its Shape's names.
GRAIN_SHAPES = (
    ns.Granulator.GAUSSIAN,
    ns.Granulator.GAUSSIAN_50,
    ns.Granulator.DECAY,
    ns.Granulator.REVERSE_DECAY,
)


@functools.lru_cache(maxsize=len(GRAIN_SHAPES))
def granulator(shape: int) -> ns.Granulator:
    """Returns the Granulator for a shape, made once."""
    return ns.Granulator(SR, GRAIN_SHAPES[shape])


@functools.lru_cache(maxsize=1024)
def render_grain(
    frequency: float, waves: int, shape: int, delay: float = 0.0
) -> ns.Buffer:
    """Returns one grain: waves of a sine at frequency in the shape's envelope,
    delay of a sample late, 0 up to 1.

    The Buffer is shared, don't change it.
    """
    if delay:
        grain = render_grain(frequency, waves, shape)
        silence = ns.Buffer.zeros(1)

        # Each sample moves toward the one before it.
        late = ns.Buffer.zeros(1)
        late << grain
        early = ns.Buffer()
        early << grain << silence

        return early * (1.0 - delay) + late * delay

    # Asked for less than a sample, generate() makes just the first grain.
    return granulator(shape).generate(
        0.5 / SR, frequency, waves, grains_per_second=1.0)

MAX_PARAMETERS = 4


def default_values(instrument: int) -> list:
    """Returns the MAX_PARAMETERS values an instrument starts with."""
    values = [p.default for p in INSTRUMENTS[instrument].parameters]

    return values + [0.0] * (MAX_PARAMETERS - len(values))


class NoteKey(typing.NamedTuple):
    """What a rendered note depends on."""

    instrument: int
    note: int           # MIDI note number
    n_samples: int      # a struck note's length, 0 for a held note
    values: tuple       # the instrument's parameters
    normalize: bool = False     # its loudness is evened out


@dataclasses.dataclass
class Note:
    """A rendered note."""

    channels: tuple     # a Buffer, or the left and right Buffers
    # Where a held note repeats from when it reaches the end, else None.
    loop_start: typing.Optional[int] = None


def loop_end(samples: ns.Buffer, frequency: float) -> int:
    """Returns where a repeat of samples from LOOP_START ends smoothly."""
    before_start = samples.subbuffer(LOOP_START - MATCH_SAMPLES, MATCH_SAMPLES)

    def mismatch(end: int) -> float:
        before_end = samples.subbuffer(end - MATCH_SAMPLES, MATCH_SAMPLES)

        return (before_end - before_start).getAbs().getSum()

    # The end, within a period of LOOP_END, where the wave best matches the
    # start's.
    period = int(SR / frequency) + 1

    return min(range(LOOP_END - period, LOOP_END + period), key=mismatch)


def cut_loop(samples: ns.Buffer, end: int) -> ns.Buffer:
    """Returns samples up to end, fading into those leading to LOOP_START."""
    lines = ns.Generator(SR)
    seconds = LOOP_FADE / SR

    fade = (
        samples.subbuffer(end - LOOP_FADE, LOOP_FADE)
            * lines.drawLine(seconds, 1.0, 0.0)
        + samples.subbuffer(LOOP_START - LOOP_FADE, LOOP_FADE)
            * lines.drawLine(seconds, 0.0, 1.0))

    loop = samples.subbuffer(0, end - LOOP_FADE)
    loop << fade

    return loop


def normal_gain(channels: tuple) -> float:
    """Returns the volume that gives a note the usual loudness."""
    loudest = 0.0
    highest = 0.0

    for channel in channels:
        length = channel.getLength()
        highest = max(highest, channel.getAbs().getMax())

        for start in range(0, length, NORMAL_WINDOW):
            window = channel.subbuffer(
                start, min(NORMAL_WINDOW, length - start))
            power = (window * window).getSum() / NORMAL_WINDOW
            loudest = max(loudest, power ** 0.5)

    if loudest == 0.0:
        return 1.0

    return min(NORMAL_RMS / loudest, LOUDEST_PEAK / highest)


def render_note(key: NoteKey) -> Note:
    """Renders the note key describes."""
    instrument = INSTRUMENTS[key.instrument]
    frequency = 440.0 * 2.0 ** ((key.note - 69) / 12.0)
    values = key.values[:len(instrument.parameters)]

    if instrument.held:
        channels = instrument.render(frequency, HELD_SECONDS, values)

        # The channels repeat together.
        mixed = channels[0]

        for channel in channels[1:]:
            mixed = mixed + channel

        end = loop_end(mixed, frequency)

        looped = [cut_loop(c, end) for c in channels]
        volume = instrument.volume

        if key.normalize:
            volume = normal_gain(looped)

        return Note(
            tuple(volume * c for c in looped),
            LOOP_START)

    seconds = max(key.n_samples / SR, instrument.shortest)
    channels = instrument.render(frequency, seconds, values)
    n_samples = min(key.n_samples, channels[0].getLength())

    n_fade = min(CUT_FADE, n_samples)
    n_whole = n_samples - n_fade

    volume = ns.Buffer()

    if n_whole:
        volume << ns.Generator(SR).drawLine(n_whole / SR, 1.0, 1.0)

    volume << ns.Generator(SR).drawLine(n_fade / SR, 1.0, 0.0)
    channels = [c.subbuffer(0, n_samples) * volume for c in channels]
    gain = instrument.volume

    if key.normalize:
        gain = normal_gain(channels)

    return Note(tuple(gain * c for c in channels))


class NoteCache:
    """Keeps the most recently used notes, rendering them when asked."""

    def __init__(self, size: int = 128) -> None:
        """Creates an empty cache of at most size notes."""
        self.size = size
        self.notes = collections.OrderedDict()

    def get(self, key: NoteKey) -> typing.Optional[Note]:
        """Returns the note, or None if it isn't rendered yet."""
        if key not in self.notes:
            self.request(key)

        if key not in self.notes:
            return None

        self.notes.move_to_end(key)

        return self.notes[key]

    def put(self, key: NoteKey, note: Note) -> None:
        """Keeps a rendered note, dropping the least recently used."""
        self.notes[key] = note
        self.notes.move_to_end(key)

        while len(self.notes) > self.size:
            self.notes.popitem(last=False)

    def request(self, key: NoteKey) -> None:
        """Renders a missing note, at once here."""
        self.put(key, render_note(key))
