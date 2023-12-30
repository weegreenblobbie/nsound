//-----------------------------------------------------------------------------
//
//  $Id: bebot.cc 913 2015-08-08 16:41:22Z weegreenblobbie $
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
//~    float64 freq = 110.0;

//~    float64 T = 10.0 / freq;

    FilterLowPassIIR lpf(sr,  6, 80, 0.05);

    Sine gen(sr);
    Triangle gent(sr, 0.02, 0.02);

    GeneratorDecay gend(sr);

    gend.addSync(gen);
//~    gend.addSync(gent);

    AudioStream aout(sr, 1);

    //-------------------------------------------------------------------------
    // Const frequency, n harmonics ramp up

    Buffer freqs = gen.drawLine(2, 49.0, 49.0);
    Buffer Ks    = gen.drawLine(2, 275,  8000.0);
    Buffer alpha = gen.drawLine(2, 6, 36);
    Buffer f_cut = gen.drawLine(2, 1000, 8000);

    aout << gen.silence(0.5);

    Buffer temp = gend.generate2(2, freqs, alpha);
    temp *= gen.generate(2, Ks);

    temp -= temp.getMin();

    temp.normalize();

    temp *= gent.generate(2, freqs);

    aout << lpf.filter(temp, f_cut);

    //-------------------------------------------------------------------------
    // Frequency ramp, n harmonics constant

    aout << gen.silence(0.5);

    freqs = gen.drawLine(2.0, 49, 600.0);
    Ks    = gen.drawLine(2.0, 8000, 8000);

    temp = gend.generate2(2, freqs, 36);
    temp *= gen.generate(2, Ks);

    temp -= temp.getMin();
    temp.normalize();

    temp *= gent.generate(2, freqs);

    aout << lpf.filter(temp, 8000);

    //-------------------------------------------------------------------------
    // Frequency const, n harmonics ramp down

    aout << gen.silence(0.5);

    freqs = gen.drawLine(2.0, 600, 600.0);
    Ks    = gen.drawLine(2.0, 8000, 275);
    alpha = gen.drawLine(2,   36, 6);
    f_cut = gen.drawLine(2,  8000, 1000);

    temp = gend.generate2(2, freqs, alpha);
    temp *= gen.generate(2, Ks);

    temp -= temp.getMin();
    temp.normalize();

    temp *= gent.generate(2, freqs);

    aout << lpf.filter(temp, f_cut);

    //-------------------------------------------------------------------------
    // Frequency ramp down, n harmonics constant

    aout << gen.silence(0.5);

    freqs = gen.drawLine(2.0, 600, 49.0);
    Ks    = gen.drawLine(2.0, 275, 275);
    alpha = gen.drawLine(2,  6, 6);
    f_cut = gen.drawLine(2, 1000, 1000);

    temp = gend.generate2(2, freqs, alpha);
    temp *= gen.generate(2, Ks);

    temp -= temp.getMin();
    temp.normalize();

    temp *= gent.generate(2, freqs);

    aout << lpf.filter(temp, f_cut);

    //-------------------------------------------------------------------------
    // output

    aout << gen.silence(0.5);

    aout *= 0.666;

    aout >> "bebot.wav";

    AudioPlaybackRt pb(sr, 1, 3, 0.05);

    aout >> pb;

    return 0;
}
