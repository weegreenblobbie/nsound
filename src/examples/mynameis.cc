//-----------------------------------------------------------------------------
//
//  $Id: mynameis.cc 874 2014-09-08 02:21:29Z weegreenblobbie $
//
//-----------------------------------------------------------------------------

// Nsound headers
#include <Nsound/NsoundAll.h>

using namespace Nsound;

int
main(void)
{
    AudioStream as0("mynameis.wav");

    float64 sr = as0.getSampleRate();

    // Get rid of some low frequency noise from the recording.
    FilterHighPassIIR hp(sr, 6, 200.0, 0.01);

    as0 = hp.filter(as0);

    Buffer raw = as0[0];

    raw.normalize();
    raw *= 0.5;

    AudioStream as1(sr, 2);

    as1 << raw;

    float64 raw_duration = as1.getDuration();

    // dynamic pan
    Sine sine(sr);

    AudioStream temp = as1.getPan(sine.generate(1.0,8.0));

    temp >> "mynameis-pan.wav";

    // Create a Stretcher instance
    Stretcher stretch(sr, 0.08, 0.25);

    stretch.showProgress(true);

    // Pitch UP
    temp = stretch.pitchShift(as1, 1.3);

    temp >> "mynameis-high-pitch.wav";

    // Pitch DOWN
    temp = stretch.pitchShift(as1, 0.7);

    temp >> "mynameis-low-pitch.wav";

    // Speed UP
    temp = stretch.timeShift(as1, 0.7);

    temp >> "mynameis-faster.wav";

    // Speed DOWN
    temp = stretch.timeShift(as1, 1.3);

    temp >> "mynameis-slower.wav";

    // wobble
    Buffer wobble = 1.0 + 0.25 * sine.generate(1.0,5.0);

    temp = stretch.pitchShift(as1, wobble);

    temp >> "mynameis-wobble.wav";

    // ramp
    Buffer ramp = sine.drawLine(as1.getDuration(), 0.7, 1.3);

    temp = stretch.pitchShift(as1, ramp);

    temp >> "mynameis-ramp.wav";

    // dynamic filter
    Buffer low_freqs = sine.drawLine(1.0, 1000, 1000)
                    << sine.drawLine(raw_duration - 1.0, 1000, 20);

    Buffer high_freqs = sine.drawLine(1.0, 2000, 2000)
                     << sine.drawLine(raw_duration - 1.0, 2000, 16000);

    FilterBandPassIIR bpf(sr, 4, 1000, 2000, 0.01);

    AudioStream filtered(sr, 1);

    filtered << raw << sine.silence(0.25);

    filtered = bpf.filter(filtered, low_freqs, high_freqs);

    // There is a large click at the end of this result from the IIR high pass
    // portion, IIR filter are not stable at every frequency.

    // Cut off the click.
    filtered = filtered.substream(0.0f, 3.7f);

    filtered.normalize();
    filtered *= 0.5;

    filtered >> "mynameis-filtered.wav";

    return 0;
}
