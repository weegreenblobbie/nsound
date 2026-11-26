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
| `bebot` | Two seconds of `bebot.py`'s voice, not interactive: a 49 Hz note brightening as its modulation and low pass cutoff sweep up, played | | `bebot.wav` |
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

`bebot_py` is the Python example with a Bazel target; it runs `bebot.py` with
the Python module Bazel builds, matplotlib and pygame:

    bazel run //src/examples:bebot_py -- --host-api wasapi --buffer 0.005

### bebot.py

A chord synthesizer played with the mouse: hold the left button on the square
pad and move, release to fade out.  Bottom to top raises the chord's root note
from 40 to 650 Hz, left to right brightens the sound.  ESC quits.

The panel beside the pad sets:

| Control | What it does |
|---|---|
| Chord | None (the default, the root alone), Major, Minor, Sus2, Sus4, Dim, Aug, Maj7 or Min7 |
| Snap to Semitones | The root moves in whole semitones, marked by lines on the pad, instead of gliding |
| Arpeggio | The chord's notes play one at a time, from the root at each press |
| Tempo | The arpeggio's tempo, 40 to 240 beats per minute, a beat is a quarter note |
| Note value | Each arpeggio note's length: 1, 1/2, 1/4, 1/8 or 1/16, or the triplets 1/8T and 1/16T.  Notes start on their exact sample |
| Note levels | A volume from 0 to 100% for each of the chord's notes, also used by the arpeggio.  Lowering one leaves the others as loud; a note's level stays with its position when the chord changes |

`bebot_synth.py` makes the sound, tested by `//src/test:py_test_bebot_synth`.
The window and the sound run in two processes; the window shares its controls
with the sound's process.  The options set the latency from mouse to sound:

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
| `bebot-disp.py` | `bebot.py` with an oscilloscope and a spectrogram of the sound drawn in its window (matplotlib) |
| `delayline-rt.py WAVE` | Plays a `.wav` file through a `DelayLine` set with the mouse: left to right, the delay from 0.001 to 1 s; bottom to top, the feedback from 0 to 0.99 |
| `moogvcf-rt.py WAVE` | Plays a `.wav` file through a `FilterLowPassMoogVcf` set with the mouse: the cutoff and the resonance.  `--plot` plots the filter's frequency response |
| `piano.py` | A keyboard piano, the `z`, `s`, `x`, `d`, `c`, `v`... keys play the notes from middle C.  `--saw` or `--square` change the wave, `--n` its harmonics, `--chorus N` adds N voices |
| `ringing.py WAVE` | Ring modulates a `.wav` file with a sine, left to right sets its frequency, 0.1 to 1000 Hz |
| `tapedeck-rt.py WAVE` | Plays a `.wav` file like a tape, left to right sets the speed, -3 to 3 times, backwards on the left |
| `make_oscilloscope_video.py` | Makes an oscilloscope video of a `.wav` file: draws the frames with matplotlib, then joins them with the sound using `ffmpeg`.  `--fps`, `--width`, `--channel`, `--freq` and `--pk-pk` set it up |

The interactive scripts take `--jack` like `bebot.py`, but not its other
options.

## Other files

- `run_from_examples_dir.h`: changes into this folder under `bazel run`.
- `getWavefiles.bash`: fetches the input `.wav` files from Nsound's old
  SourceForge SVN.
