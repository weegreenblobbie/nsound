//-----------------------------------------------------------------------------
//
//  $Id: stretcher.cc 913 2015-08-08 16:41:22Z weegreenblobbie $
//
//-----------------------------------------------------------------------------

// Nsound headers
#include <Nsound/NsoundAll.h>

// C++ header
#include <iostream>

using namespace Nsound;

using std::cout;
using std::endl;

//-----------------------------------------------------------------------------
int
main(void)
{
    AudioStream a("Temperature_in.wav");

    // Grab sample rate.
    float64 sr = a.getSampleRate();

    // Create a audio playback object

    AudioPlaybackRt pb(sr, a.getNChannels());

    // Grab the duration in seconds.
    float64 duration = a.getDuration();

    // Create a Gaussian curve for pitch/time shifting.
    Sine sin(sr);

    Buffer bend = sin.drawFatGaussian(duration, 0.15) + 1.0;

    // Create a Stretcher instance
    Stretcher stretch(sr, 0.08, 0.25);

    // Print progress to command line.
    stretch.showProgress(true);

    cout << "Pitch Shifting Up" << endl;

    // Create new output AudioStream, pitch shift input AudioStream.
    AudioStream out(sr, 2);

    out << stretch.pitchShift(a, bend);
    out >> "Temperature_Pitch_Shifted_Up.wav";
    out * 0.666 >> pb;

    cout << "Time Shifting Faster" << endl;

    // Time shift input AudioStream.
    out = AudioStream(sr, 2);

    out << stretch.timeShift(a, 1.0 / bend);
    out >> "Temperature_Time_Shifted_Faster.wav";
    out * 0.666 >> pb;

    bend = 1.0 - 0.25 * sin.drawFatGaussian(duration, 0.15);

    cout << "Pitch Shifting Down" << endl;

    out = AudioStream(sr, 2);
    out << stretch.pitchShift(a, bend);
    out >> "Temperature_Pitch_Shifted_Down.wav";
    out * 0.666 >> pb;

    cout << "Time Shifting Slower" << endl;

    bend = 1.0 + 0.75 * sin.drawFatGaussian(duration, 0.15);

    out = AudioStream(sr,2);
    out << stretch.timeShift(a, bend);
    out >> "Temperature_Time_Shifted_Slower.wav";
    out * 0.666 >> pb;

    return 0;
}

