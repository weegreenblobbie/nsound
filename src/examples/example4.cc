//-----------------------------------------------------------------------------
//
//  $Id: example4.cc 874 2014-09-08 02:21:29Z weegreenblobbie $
//
//-----------------------------------------------------------------------------

// Nsound headers
#include <Nsound/NsoundAll.h>

using namespace Nsound;

#include <iostream>

using std::cerr;
using std::endl;

//-----------------------------------------------------------------------------
int
main(void)
{
    float64 sr = 44100.0;

    Sine sine(sr);

    GuitarBass bass(sr);
    FluteSlide slide(sr);
    Clarinet   clarinet(sr);

    AudioStream out(sr, 1);

    out << bass.play()      << sine.silence(1.0)
        << slide.play()     << sine.silence(1.0)
        << clarinet.play()  << sine.silence(1.0);

    out *= 0.5;

    // Set the default Wavefile sample size and rate.
    Wavefile::setDefaults(sr, 16);

    out >> "example4.wav";

    ReverberationRoom room(sr, 0.9); // 0.9 = room feed back (0.0 to 1.0)

    AudioStream out2 = room.filter(0.5 * (bass.play() << sine.silence(1.5)));

    out2 >> "example4_reverb.wav";

    // Try to play the audio

    try
    {
        AudioPlayback pb(sr, 2, 16);
        out >> pb;
        out2 >> pb;
    }
    catch(Exception e)
    {
        cerr << "Could not play audio: " << e.what() << endl;
    }

    return 0;
}

