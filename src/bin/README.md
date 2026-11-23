# Nsound command line programs

    bazel run //src/bin:<program> -- <arguments>

`bazel run` starts a program in its runfiles folder, not where you typed the
command, so give these programs absolute paths to `.wav` files.

## ns_playback

Lists portaudio's host APIs, or plays a test sound through one with
`AudioPlaybackRt`, to hear which plays cleanly on a machine.

    bazel run //src/bin:ns_playback
    bazel run //src/bin:ns_playback -- "<host API>" [buffer seconds] [sample rate] [channels]

With no arguments it lists the host APIs compiled in, each with its default
output device, sample rate and channels, e.g. on Windows:

    MME
        Speakers (Focusrite USB Audio), 44100 Hz, 2 channels
    Windows WASAPI
        Speakers (Focusrite USB Audio), 48000 Hz, 2 channels
    Windows WDM-KS
        Speakers (wr4800_8219), 48000 Hz, 2 channels

Given a host API, any case, it plays a 4 second sine sweep from 220 to 880 Hz.
A glitch in a pure tone is heard as a click.  Then it prints the player's
settings, the actual latency and the underrun counts.

| Argument | Default |
|---|---|
| buffer seconds | 0.05, raised to the device's lowest latency |
| sample rate | the default output device's |
| channels | 2, or 1 when the device has 1 |

MME and WASAPI convert to the device's format.  WDM-KS doesn't, so it plays
only the sample rates and channels the device does, which the defaults pick.

Built with `--//bazel:portaudio=false`, it only says portaudio is missing.

## ns_readwaveheader

Prints a `.wav` file's header, then `SUCCESS`, or `FAILURE` when it can't be
read.

    bazel run //src/bin:ns_readwaveheader -- /path/to/file.wav

## ns_vocoder

A vocoder: the voice's spectrum shapes the carrier, e.g. a voice and a
synthesizer pad make a talking synthesizer.

    bazel run //src/bin:ns_vocoder -- [options] voice.wav carrier.wav output.wav

| Option | What it sets |
|---|---|
| `-n`, `--bands N` | The number of frequency bands |
| `-w`, `--window S` | The window, in seconds, the bands' envelopes are measured over |
| `-m`, `--fmax F` | The highest frequency in the filter bank |
| `-v`, `--verbose` | Prints progress |
| `-h`, `--help` | Prints the usage |
