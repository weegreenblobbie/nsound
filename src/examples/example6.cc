//-----------------------------------------------------------------------------
//
//  $Id: example6.cc 874 2014-09-08 02:21:29Z weegreenblobbie $
//
//-----------------------------------------------------------------------------

// Nsound headers
#include <Nsound/NsoundAll.h>

using namespace Nsound;

using std::cerr;
using std::endl;

//-----------------------------------------------------------------------------
AudioStream
softTones(
    const float64 sr,
    const float64 duration,
    const float64 f1,
    const float64 f2,
    const float64 gaussian_width)
{
    Sine sin(sr);

    AudioStream as(sr, 2);

    Buffer env = sin.drawFatGaussian(duration, gaussian_width);

    as[0] = sin.generate(duration, f1);
    as[1] = sin.generate(duration, f2);

    return as * env;
}

//-----------------------------------------------------------------------------
int
main(void)
{
    float64 sr = 44100;

    Sine sine(sr);

    AudioStream out(sr, 2);

    out << softTones(sr, 0.25, 261.63, 523.25, 0.90)
        << softTones(sr, 0.25, 493.87, 293.66, 0.90)
        << softTones(sr, 0.25, 329.61, 439.96, 0.90)
        << softTones(sr, 0.25, 391.97, 349.22, 0.90)
        << softTones(sr, 0.25, 349.22, 391.97, 0.90)
        << softTones(sr, 0.25, 439.96, 329.61, 0.90)
        << softTones(sr, 0.25, 293.66, 493.87, 0.90)
        << softTones(sr, 0.25, 523.25, 261.63, 0.90)
        << softTones(sr, 0.25, 261.63, 523.25, 0.90)
        << softTones(sr, 0.25, 493.87, 293.66, 0.90)
        << softTones(sr, 0.25, 329.61, 439.96, 0.90)
        << softTones(sr, 0.25, 391.97, 349.22, 0.90)
        << softTones(sr, 0.25, 349.22, 391.97, 0.90)
        << softTones(sr, 0.25, 439.96, 329.61, 0.90)
        << softTones(sr, 0.25, 293.66, 493.87, 0.90)
        << softTones(sr, 0.25, 523.25, 261.63, 0.90)
        << sine.silence(0.25);

    out << softTones(sr, 0.25, 261.63, 523.25, 0.30)
        << softTones(sr, 0.25, 493.87, 293.66, 0.30)
        << softTones(sr, 0.25, 329.61, 439.96, 0.30)
        << softTones(sr, 0.25, 391.97, 349.22, 0.30)
        << softTones(sr, 0.25, 349.22, 391.97, 0.30)
        << softTones(sr, 0.25, 439.96, 329.61, 0.30)
        << softTones(sr, 0.25, 293.66, 493.87, 0.30)
        << softTones(sr, 0.25, 523.25, 261.63, 0.30)
        << softTones(sr, 0.25, 261.63, 523.25, 0.30)
        << softTones(sr, 0.25, 493.87, 293.66, 0.30)
        << softTones(sr, 0.25, 329.61, 439.96, 0.30)
        << softTones(sr, 0.25, 391.97, 349.22, 0.30)
        << softTones(sr, 0.25, 349.22, 391.97, 0.30)
        << softTones(sr, 0.25, 439.96, 329.61, 0.30)
        << softTones(sr, 0.25, 293.66, 493.87, 0.30)
        << softTones(sr, 0.25, 523.25, 261.63, 0.30)
        << sine.silence(0.25);

    out *= 0.5;

    out >> "example6.wav";

    // ReverberationRoom(sample_rate, room_feedback, wet_percent, dry_percent, low_pass_freq)
    ReverberationRoom room(sr, 0.50, 1.0, 1.0, 100.0);

    AudioStream out2 = 0.5 * room.filter(out);

    out2 >> "example6_reverb.wav";

    // Try to play the audio

    try
    {
        AudioPlayback pb(sr, 2, 16);
        out2 >> pb;
    }
    catch(Exception e)
    {
        cerr << "Could not play audio: " << e.what() << endl;
    }

    return 0;
}

