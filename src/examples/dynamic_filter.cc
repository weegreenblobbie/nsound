//-----------------------------------------------------------------------------
//
//  $Id: dynamic_filter.cc 913 2015-08-08 16:41:22Z weegreenblobbie $
//
//-----------------------------------------------------------------------------

// Nsound headers
#include <Nsound/NsoundAll.h>

using namespace Nsound;

int
main(void)
{
    // Read in the wavefile.
    AudioStream input("Temperature_in.wav");

    // Grab the duration in seconds.
    float32 duration = input.getDuration();

    // Create a low pass filter with a kernel of 256 terms.
    FilterLowPassFIR lpf(input.getSampleRate(), 256, 100);

    // Create a buffer that will hold cut off frequencies.
    Buffer frequencies;

    // Fill it with two lines
    Sine sin(input.getSampleRate());

    frequencies << sin.drawLine(0.5 * duration, 8000, 50)
                << sin.drawLine(0.5 * duration, 50, 8000);

    // Filter it.
    AudioStream output = lpf.filter(input, frequencies);

    // Write to disk.
    output >> "Temperature_out.wav";

    AudioPlaybackRt pb(input.getSampleRate(), 2);

    output * 0.666 >> pb;

    return 0;
}
