# Nsound examples

Example programs showing Nsound's synthesis, filters and playback.  The C++
examples are Bazel targets:

    bazel run //src/examples:example1

Under `bazel run` they change into this folder (see `run_from_examples_dir.h`),
where they read their input `.wav` files and write their output.  The inputs
kept in git are `mynameis.wav`, `california.wav`, `walle.wav` and
`Temperature_in.wav`.

The examples that play their result use `AudioPlayback` or `AudioPlaybackRt`,
which need Nsound built with portaudio (the default).  Without portaudio they
still write their `.wav` files.

## C++ examples

| Target | What it does | Reads | Writes |
|---|---|---|---|
| `bebot` | Two seconds of `synth.py`'s Bebot voice, not interactive: a 49 Hz note brightening as its modulation and low pass cutoff sweep up, played | | `bebot.wav` |
| `cepstral_pitch_estimate` | Estimates the pitch of speech by cepstral analysis of its spectrogram, and plots it (needs matplotlib, see `Plotter`) | `walle.wav` | |
| `dynamic_filter` | A low pass FIR filter whose cutoff sweeps from 8000 Hz down to 50 Hz and back, and plays the result | `Temperature_in.wav` | `Temperature_out.wav` |
| `example1` | A sine whose frequency sweeps up and down, seven times | | `example1.wav` |
| `example2` | A 220 Hz tone panned left and right by a 3 Hz sine | | `example2.wav` |
| `example3` | Drums: `DrumBD01`, `DrumKickBass` and `Hat`, dry and in a reverberant room; based on the Csound drum by Hans Mikelson | | `example3.wav`, `example3_reverb.wav` |
| `example4` | Instruments: `GuitarBass`, `FluteSlide` and `Clarinet`, then the bass in a reverberant room | | `example4.wav`, `example4_reverb.wav` |
| `example5` | A `Vocoder`, the voice shaping a `Granulator` carrier | `california.wav` | `example5.wav` |
| `example6` | Pairs of soft tones, one in each channel, dry and in a reverberant room | | `example6.wav`, `example6_reverb.wav` |
| `interstellar` | An `OrganPipe` melody through a low pass filter and a reverberant room, 48000 Hz stereo | | `interstellar-theme.wav` |
| `mynameis` | Variations on a voice: panned, pitched up and down, faster and slower with `Stretcher`, and more | `mynameis.wav` | `mynameis-*.wav` |
| `stretcher` | `Stretcher` pitch and time shifts along a Gaussian curve, each played | `Temperature_in.wav` | `Temperature_*_Shifted_*.wav` |

## Python examples

`synth` is the Python example with a Bazel target; it runs `synth.py` with
the Python module Bazel builds, matplotlib and pygame:

    bazel run //src/examples:synth -- --host-api wasapi --buffer 0.005

### synth.py

A chord synthesizer played with the mouse: hold the left button on the square
pad and move, release to stop.  Bottom to top raises the chord's root note
from 40 to 650 Hz, for the Granulator to 2600 Hz.  ESC quits.

Left to right sets the slider whose `x` box is checked, one at most for each
instrument: any of its own controls, the tempo, a note level or an envelope or
filter setting.  At the start only Bebot's brightness and the Granulator's
grains per second are.  "Reset to defaults" puts every setting of every
instrument back, and keeps the instrument chosen.

The panel's left column chooses the instrument and sets its own controls:

| Instrument | Plays | Its controls |
|---|---|---|
| Bebot | While held, gliding with the mouse | Brightness |
| Clarinet | While held | Pressure, filter, reed stiffness |
| Flute | While held | Pressure, breath |
| Organ Pipe | While held, in stereo | Pan, left to right |
| Guitar Bass | Struck, see below | |
| Pluck | Struck | Smoothing of the plucked noise, higher is mellower |
| Bass Drum | Struck | Velocity |
| Kick Drum | Struck, the pad sets the low frequency it falls to | High frequency it starts from |
| Hat | Struck, the pad tunes it, as usual at the middle | Closed or open cymbals |
| Granulator | While held, a stream of grains: short bursts of a sine at the pad's pitch.  Left to right sets the grains per second | Grains per second, up to 300; a box to have the rate follow the pitch, a grain each wave; waves per grain; the grain's envelope shape |
| Piano | While held, `piano.py`'s oscillator | Wave: sine, sawtooth or square; the harmonics of the last two; a box for its own envelope (attack 0.2 s, decay 0.4 s, sustain 33%, release 0.1 s) in place of the panel's |

A struck instrument hits once each note value at the tempo while held, at the
pitch under the mouse when it hits, and the last hit rings out after the
release.  Set from left to right, the controls of the instruments that render
whole notes take 20 values.

Bebot, the Granulator and the Piano make their sound as they go, so they follow
the mouse at once.  Each grain takes the pitch and settings of the moment it
starts.
Grains faster than their pitch would cancel each other, so each note's rate is
held to its frequency in Hz.

The others render a whole note at a time, which is kept to be played again, so
a note is late the first time it plays.
Without snapping, they glide by playing the nearest semitone's note a little
faster or slower; a struck note keeps the pitch it was struck at.

The right column shapes the sound of every instrument:

| Control | What it does |
|---|---|
| Attack | The seconds the volume takes to rise to full, 0 to 2 |
| Decay | The seconds it then takes to fall to the sustain level, 0 to 2 |
| Sustain | That level, 0 to 100% |
| Release | The seconds it takes to fade out, 0.002 to 3 |
| Cutoff | A Moog style low pass filter's cutoff, 100 to 12000 Hz.  At the top with no resonance the filter is off |
| Resonance | The filter's emphasis at the cutoff, 0 to 85%, which also quietens the sound below it |
| Normalize volume | Evens out the rendered notes' loudness across pitches and settings, on at the start.  Bebot's is even already |

The last column adds a chorus to every instrument, after the filter:

| Control | What it does |
|---|---|
| On | Turns the chorus on, off at the start |
| Voices | 1 to 4 copies of the sound |
| Detune | The most each copy's pitch drifts up or down, 0 to 50 cents.  At slow rates the delays reach their limit and it drifts less |
| Rate | How often the pitch drifts up and down, 0.1 to 5 times a second |
| Mix | The copies' volume, 0 to 100%.  The output is the average of the sound and its copies |
| Stereo width | Spreads the copies from side to side, 0 to 100% |

`synth_chorus.py` delays each copy by 5 ms or more, swinging the delay to bend
its pitch.

A held instrument's envelope starts at the press and at each arpeggio step, and
releases when the mouse does.  Each struck note has its own: it releases at the
end of the note value, and an attack or decay can only soften the instrument's
own sound.

The middle column sets the notes all instruments share:

| Control | What it does |
|---|---|
| Chord | None (the default, the root alone), Major, Minor, Sus2, Sus4, Dim, Aug, Maj7 or Min7 |
| Snap to Semitones | The root moves in whole semitones, marked by lines on the pad, instead of gliding.  Each instrument remembers its own, at the start on for all but Bebot |
| Arpeggio | The chord's notes play one at a time, from the root at each press.  Without a chord a held instrument's note repeats, sounding for half of each step |
| Tempo | The arpeggio's tempo, 40 to 240 beats per minute, a beat is a quarter note |
| Note value | Each arpeggio note's length, and the time between a struck instrument's hits: 1, 1/2, 1/4, 1/8 or 1/16, or the triplets 1/8T and 1/16T.  Arpeggio notes start on their exact sample |
| Note levels | A volume from 0 to 100% for each of the chord's notes, also used by the arpeggio.  Lowering one leaves the others as loud; a note's level stays with its position when the chord changes |

`synth_engine.py` and `synth_instruments.py` make the sound, tested by
`//src/test:py_test_synth`.
The window, the sound and the note rendering run in three processes; the window
shares its controls with the sound's process.  The options set the latency from
mouse to sound:

| Option | Default | What it does |
|---|---|---|
| `--host-api NAME` | portaudio's default | The portaudio host API to play through, with 0.01 s buffers.  `bazel run //src/bin:ns_playback` lists them; "windows " can be left off and case doesn't matter, e.g. `wasapi`, `wdm-ks` or `mme` |
| `--buffer SECONDS` | 0.1, or 0.01 with `--host-api` or `--jack` | The seconds of audio in each buffer |
| `--buffers N` | 3 | How many buffers the sound can be ahead of what is heard |
| `--chunk SECONDS` | 0.01 | The seconds of sound made per read of the controls.  Smaller follows the mouse sooner, too small crackles |
| `-j`, `--jack` | | Plays through JACK, Linux only |
| `--debug` | | Prints the chunks' render times and the playback's underrun and overrun counters every 5 s of sound and at each release |

When its sound starts it prints the host API and device, the format, these
settings and the latency they add up to, at most:

    chunk + buffers × buffer + the device's output latency

### The other Python examples

These have no Bazel target; they run with a Python that can import Nsound,
pygame for the interactive ones and matplotlib for the plotting ones.

| Script | What it does |
|---|---|
| `example1.py` to `example6.py`, `dynamic_filter.py`, `stretcher.py` | The same as their C++ examples above |
| `bebot-disp.py` | An earlier version of the synth with only Bebot, with an oscilloscope and a spectrogram of the sound drawn in its window (matplotlib) |
| `delayline-rt.py WAVE` | Plays a `.wav` file through a `DelayLine` set with the mouse: left to right, the delay from 0.001 to 1 s; bottom to top, the feedback from 0 to 0.99 |
| `moogvcf-rt.py WAVE` | Plays a `.wav` file through a `FilterLowPassMoogVcf` set with the mouse: the cutoff and the resonance.  `--plot` plots the filter's frequency response |
| `piano.py` | A keyboard piano, the `z`, `s`, `x`, `d`, `c`, `v`... keys play the notes from middle C.  `--saw` or `--square` change the wave, `--n` its harmonics, `--chorus N` adds N voices |
| `ringing.py WAVE` | Ring modulates a `.wav` file with a sine, left to right sets its frequency, 0.1 to 1000 Hz |
| `tapedeck-rt.py WAVE` | Plays a `.wav` file like a tape, left to right sets the speed, -3 to 3 times, backwards on the left |
| `make_oscilloscope_video.py` | Makes an oscilloscope video of a `.wav` file: draws the frames with matplotlib, then joins them with the sound using `ffmpeg`.  `--fps`, `--width`, `--channel`, `--freq` and `--pk-pk` set it up |

The interactive scripts take `--jack` like `synth.py`, but not its other
options.

## Other files

- `run_from_examples_dir.h`: changes into this folder under `bazel run`.
- `getWavefiles.bash`: fetches the input `.wav` files from Nsound's old
  SourceForge SVN.
