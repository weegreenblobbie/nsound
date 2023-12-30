//-----------------------------------------------------------------------------
//
//  $Id: example1.cc 874 2014-09-08 02:21:29Z weegreenblobbie $
//
//-----------------------------------------------------------------------------

#include <Nsound/NsoundAll.h>

#include <iostream>

using std::cout;
using std::cerr;
using std::endl;

using namespace Nsound;

int
main(void)
{
    float64 sr = 44100.0;

    // Creating the Pac Man background tone.

    Sine sine(sr);

    float64 time   = 0.40;
    float64 h_time = time / 2.0;

    // The first tone.
    Buffer f1 = sine.drawLine(h_time, 345, 923)
             << sine.drawLine(h_time, 923, 345);

    Buffer pac_man = sine.generate(7*time, f1);

    // Here we create an envelop to smoothly finish the waveform, removing
    // any clicking that might have occured.
    Buffer envelope = sine.drawLine(7*time-0.005, 1.0, 1.0)
                   << sine.drawLine(0.005, 1.0, 0.0);

    pac_man *= envelope;

    pac_man.normalize();
    pac_man *= 0.25;

    pac_man >> "example1.wav";

    // Play to audio device.

    try
    {
        AudioPlayback pb(sr, 1, 16);
        pac_man >> pb;
    }
    catch(Exception & e)
    {
        cerr << "Could not play audio: " << e.what() << endl;
    }

    return 0;
}
