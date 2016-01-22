//-----------------------------------------------------------------------------
//
//  $Id: example3.cc 874 2014-09-08 02:21:29Z weegreenblobbie $
//
//  Simulates a drum.  Based on the Csound drum by Hans Mikelson.
//
//  source: http://www.csounds.com/ezine/winter2001/synthesis/
//
//-----------------------------------------------------------------------------

// Nsound headers
#include <Nsound/NsoundAll.h>

// C++ header
#include <iostream>

using namespace Nsound;

using std::cerr;
using std::endl;
using std::flush;

// Eck, globals
float64 sr = 44100.0;
uint64 SAMPLE_RATE = static_cast<uint64>(sr);
uint64 BITS_PER_SAMPLE = 16;

//-----------------------------------------------------------------------------
Buffer
drum(
    float32 duration,
    float32 attack_time,
    float32 high_frequency,
    float32 low_frequency,
    float32 tension,
    float32 resident_frequency)
{
    Sine sin(sr);

    Buffer frequency_sweep;
    frequency_sweep
        << sin.drawLine(attack_time, high_frequency, low_frequency)
        << sin.drawLine((duration - attack_time), low_frequency, low_frequency);

    Buffer hz_20 = sin.generate(duration, resident_frequency);

    Buffer rezzy = hz_20 * frequency_sweep;

    Buffer parabola = sin.drawParabola(duration, 1.0, duration / 2, 0.25, 0.0);

    rezzy *= parabola;

    Buffer temp1 = rezzy * tension;

    frequency_sweep -= temp1;

    Buffer audio = sin.generate(duration, frequency_sweep);

    audio *= sin.drawParabola(duration, 1.0, 0.5 * duration, 0.3, 0.0);

    return audio;
}

int
my_main(void)
{
    Sine sine(sr);

    DrumBD01     db01(sr);
    DrumKickBass dkb(sr, 266, 0.0);

    AudioStream out(sr, 1);

    out << db01.play()
        << sine.silence(0.25)
        << dkb.play()
        << sine.silence(0.25)

        // duration,  attack, high f, low f, tension, ressonance
        << drum(0.5f, 0.012f,    160,    51,    0.9f,         54)
        << drum(0.5f, 0.012f,    160,    51,    0.9f,         54)
        << drum(0.5f, 0.012f,    160,    51,    0.9f,         54)
        << drum(0.5f, 0.012f,    160,    51,    0.9f,         54)
        << sine.silence(0.25);

    out *= 0.5;

    Hat hat(sr);

    out << 0.666 * hat.play() << sine.silence(0.25);

    out >> "example3.wav";

    // ReverberationRoom(sample_rate, room_feedback, wet_percent, dry_percent, low_pass_freq)
    ReverberationRoom room(sr, 0.60, 0.5, 1.0, 100.0);

    AudioStream out2 = 0.5 * room.filter(out);

    out2 >> "example3_reverb.wav";

    // Try to play the audio

    try
    {
        AudioPlayback pb(sr, 2, 16);
        out2 >> pb;
    }
    catch(std::exception & e)
    {
        cerr << "Warning: Could not play audio: " << e.what() << endl;
    }

    return 0;
}

int
main(void)
{
    try
    {
        my_main();
    }
    catch(std::exception & e)
    {
        cerr << "Exception: " << e.what() << endl;
        return 1;
    }

    return 0;
}
