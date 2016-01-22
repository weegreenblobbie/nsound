//-----------------------------------------------------------------------------
//
//  $Id: example2.cc 874 2014-09-08 02:21:29Z weegreenblobbie $
//
//-----------------------------------------------------------------------------

// Include the Nsound headers
#include <Nsound/NsoundAll.h>

using namespace Nsound;

using std::cerr;
using std::endl;

int
main(void)
{
    float64 sr = 44100.0;

    // Create a new instance of the Sine Generator
    Sine sine(sr);

    Buffer pan;

    pan << sine.generate(1.0, 3.0);

    // Create a stereo AudioStream.
    AudioStream as(sr, 2);

    // Fill it with a 220 Hz sine wave.
    as << 0.5 * sine.generate(4.9, 220);

    // Execute the pan method.
    as.pan(pan);

    // Write the AudioStream to a wave file
    as >> "example2.wav";

    // Play it through the sound card

    try
    {
        AudioPlayback pb(sr, 2, 16);
        as >> pb;
    }
    catch(Exception e)
    {
        cerr << "Could not play audio: " << e.what() << endl;
    }

    return 0;
}
