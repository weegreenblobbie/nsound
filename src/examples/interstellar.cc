#include <iostream>
#include <string>
#include <map>
#include <Nsound/NsoundAll.h>


using namespace Nsound;
using std::cout;

// The 88 white keys on a piano:
//     https://en.wikipedia.org/wiki/Piano_key_frequencies

const auto tune = 1.00;

const auto A0 =  27.50000 * tune;
const auto B0 =  30.86771 * tune;
const auto C1 =  32.70320 * tune;
const auto D1 =  36.70810 * tune;
const auto E1 =  41.20344 * tune;
const auto F1 =  43.65353 * tune;
const auto G1 =  48.99943 * tune;
const auto A1 =  55.00000 * tune;
const auto B1 =  61.73541 * tune;
const auto C2 =  65.40639 * tune;
const auto D2 =  73.41619 * tune;
const auto E2 =  82.40689 * tune;
const auto F2 =  87.30706 * tune;
const auto G2 =  97.99886 * tune;
const auto A2 = 110.0000  * tune;
const auto B2 = 123.4708  * tune;
const auto C3 = 130.8128  * tune;
const auto D3 = 146.8324  * tune;
const auto E3 = 164.8138  * tune;
const auto F3 = 174.6141  * tune;
const auto G3 = 195.9977  * tune;
const auto A3 = 220.0000  * tune;
const auto B3 = 246.9417  * tune;
const auto C4 = 261.6256  * tune;
const auto D4 = 293.6648  * tune;
const auto E4 = 329.6276  * tune;
const auto F4 = 349.2282  * tune;
const auto G4 = 391.9954  * tune;
const auto A4 = 440.0000  * tune;
const auto B4 = 493.8833  * tune;
const auto C5 = 523.2511  * tune;
const auto D5 = 587.3295  * tune;
const auto E5 = 659.2551  * tune;
const auto F5 = 698.4565  * tune;
const auto G5 = 783.9909  * tune;
const auto A5 = 880.0000  * tune;
const auto B5 = 987.7666  * tune;
const auto C6 = 1046.502  * tune;
const auto D6 = 1174.659  * tune;
const auto E6 = 1318.510  * tune;
const auto F6 = 1396.913  * tune;
const auto G6 = 1567.982  * tune;
const auto A6 = 1760.000  * tune;
const auto B6 = 1975.533  * tune;
const auto C7 = 2093.005  * tune;
const auto D7 = 2349.318  * tune;
const auto E7 = 2637.020  * tune;
const auto F7 = 2793.826  * tune;
const auto G7 = 3135.963  * tune;
const auto A7 = 3520.000  * tune;
const auto B7 = 3951.066  * tune;
const auto C8 = 4186.009  * tune;


struct Asdr
{
    float64 _attack;
    float64 _delay;
    float64 _sustain_amp;
    float64 _release;
};

AudioStream operator*(const AudioStream & lhs, const Asdr & rhs)
{
    auto gen = Generator(lhs.getSampleRate());
    auto duration = lhs.getDuration();
    auto att_dur = duration * rhs._attack;
    auto del_dur = duration * rhs._delay;
    auto rel_dur = duration * rhs._release;
    auto sus_dur = std::max(duration - att_dur - del_dur - rel_dur, 0.001);

    auto env = gen.drawLine(att_dur, 0.0, 1.0)
            << gen.drawLine(del_dur, 1.0, rhs._sustain_amp)
            << gen.drawLine(sus_dur, rhs._sustain_amp, rhs._sustain_amp)
            << gen.drawLine(rel_dur, rhs._sustain_amp, 0.0);

    while(lhs.getLength() > env.getLength())
    {
        env << 0.0;
    }

    return lhs * env;
}
AudioStream operator*(const Asdr & lhs, const AudioStream & rhs){return rhs * lhs;}


void play_n(AudioStream & out, float64 time, const AudioStream & in1)
{
    out.add(in1, time);
}

void play_n(AudioStream & out, float64 time, const AudioStream & in1, const AudioStream & in2)
{
    out.add(in1, time);
    out.add(in2, time);
}

void play_n(AudioStream & out, float64 time, const AudioStream & in1, const AudioStream & in2, const AudioStream & in3)
{
    out.add(in1, time);
    out.add(in2, time);
    out.add(in3, time);
}

void play_n(AudioStream & out, float64 time, const AudioStream & in1, const AudioStream & in2, const AudioStream & in3, const AudioStream & in4)
{
    out.add(in1, time);
    out.add(in2, time);
    out.add(in3, time);
    out.add(in4, time);
}


int main(void)
{
    cout << "Constructing pipe organ and filters ..."; fflush(stdout);
    const auto sr = 22050.0;

    auto out = AudioStream(sr, 2);
    auto gen = Generator(sr);
    auto po = OrganPipe(sr);
    auto filter = FilterLowPassIIR(sr, 5, 5000.0, 0.05);
//~    auto reverb = ReverberationRoom(sr, 0.750, 1.00, 0.50, 2500.0, 0.1000);
//~    auto reverb = ReverberationRoom(sr, 0.750, 0.50, 0.50, 1500.0, 0.1000);
    auto reverb = ReverberationRoom(sr, 0.10, 0.5, 0.50, 5000.0, 0.10);

    Tic();

    cout << "\nRendering notes ..."; fflush(stdout);

    // Interstellar Main Theme Arranged by Patrik Pietschmann.
    //
    // Quarter notes are 90 BPM.
    //
    const auto _quarter = 60.0 / 100.0;
    const auto _eighth = _quarter / 2.0;
    const auto _sixth  = _eighth / 2.0;

    // Actual note durations.
    const auto sixth = _sixth ; //- 0.005;
    const auto eighth = _eighth - 0.05;
    const auto quarter = _quarter - 0.1;
    const auto half = 2 * _quarter - 0.1;
    const auto half_dot = 3 * _quarter - 0.1;
    const auto half_dot_2 = 6 * _quarter - 0.1;

    auto env_s   = Asdr{0.10, 0.25, 0.80, 0.25};
    auto env_e   = Asdr{0.10, 0.25, 0.80, 0.25};
    auto env     = Asdr{0.10, 0.25, 0.80, 0.25};
    auto env_h   = Asdr{0.08, 0.15, 0.80, 0.15};
    auto env_hd  = Asdr{0.04, 0.10, 0.80, 0.10};
    auto env_2hd = Asdr{0.02, 0.05, 0.80, 0.05};

    const auto a3_s = po.play(sixth, A3) * env_s;
    const auto a4_s = po.play(sixth, A4) * env_s;
    const auto a5_s = po.play(sixth, A5) * env_s;
    const auto a6_s = po.play(sixth, A6) * env_s;
    const auto b4_s = po.play(sixth, B4) * env_s;
    const auto b5_s = po.play(sixth, B5) * env_s;
    const auto b6_s = po.play(sixth, B6) * env_s;
    const auto c4_s = po.play(sixth, C4) * env_s;
    const auto c5_s = po.play(sixth, C5) * env_s;
    const auto c6_s = po.play(sixth, C6) * env_s;
    const auto d4_s = po.play(sixth, D4) * env_s;
    const auto d5_s = po.play(sixth, D5) * env_s;
    const auto d6_s = po.play(sixth, D6) * env_s;
    const auto e4_s = po.play(sixth, E4) * env_s;
    const auto e5_s = po.play(sixth, E5) * env_s;
    const auto e6_s = po.play(sixth, E6) * env_s;
    const auto f3_s = po.play(sixth, F3) * env_s;
    const auto f4_s = po.play(sixth, F4) * env_s;
    const auto f5_s = po.play(sixth, F5) * env_s;
    const auto f6_s = po.play(sixth, F6) * env_s;
    const auto g3_s = po.play(sixth, G3) * env_s;
    const auto g4_s = po.play(sixth, G4) * env_s;
    const auto g5_s = po.play(sixth, G5) * env_s;
    const auto g6_s = po.play(sixth, G6) * env_s;

    const auto a4_e = po.play(eighth, A4) * env_e;
    const auto a5_e = po.play(eighth, A5) * env_e;
    const auto b4_e = po.play(eighth, B4) * env_e;
    const auto b5_e = po.play(eighth, B5) * env_e;
    const auto c4_e = po.play(eighth, C4) * env_e;
    const auto c5_e = po.play(eighth, C5) * env_e;
    const auto c6_e = po.play(eighth, C6) * env_e;
    const auto d4_e = po.play(eighth, D4) * env_e;
    const auto d5_e = po.play(eighth, D5) * env_e;
    const auto d6_e = po.play(eighth, D6) * env_e;
    const auto e4_e = po.play(eighth, E4) * env_e;
    const auto e5_e = po.play(eighth, E5) * env_e;
    const auto e6_e = po.play(eighth, E6) * env_e;
    const auto f4_e = po.play(eighth, F4) * env_e;
    const auto f5_e = po.play(eighth, F5) * env_e;
    const auto f6_e = po.play(eighth, F6) * env_e;
    const auto g4_e = po.play(eighth, G4) * env_e;
    const auto g5_e = po.play(eighth, G5) * env_e;
    const auto g6_e = po.play(eighth, G6) * env_e;

    const auto a3 = po.play(quarter, A3) * env;
    const auto a4 = po.play(quarter, A4) * env;
    const auto a5 = po.play(quarter, A5) * env;
    const auto b3 = po.play(quarter, B3) * env;
    const auto b4 = po.play(quarter, B4) * env;
    const auto b5 = po.play(quarter, B5) * env;
    const auto c3 = po.play(quarter, C3) * env;
    const auto c4 = po.play(quarter, C4) * env;
    const auto c5 = po.play(quarter, C5) * env;
    const auto c6 = po.play(quarter, C6) * env;
    const auto d4 = po.play(quarter, D4) * env;
    const auto d5 = po.play(quarter, D5) * env;
    const auto d6 = po.play(quarter, D6) * env;
    const auto e4 = po.play(quarter, E4) * env;
    const auto e5 = po.play(quarter, E5) * env;
    const auto e6 = po.play(quarter, E6) * env;
    const auto f4 = po.play(quarter, F4) * env;
    const auto f5 = po.play(quarter, F5) * env;
    const auto g4 = po.play(quarter, G4) * env;
    const auto g5 = po.play(quarter, G5) * env;

    const auto a3_h = po.play(half, A3) * env_h;
    const auto a4_h = po.play(half, A4) * env_h;

    const auto a3_hd = po.play(half_dot, A3) * env_hd;
    const auto a4_hd = po.play(half_dot, A4) * env_hd;
    const auto c3_hd = po.play(half_dot, C3) * env_hd;
    const auto c4_hd = po.play(half_dot, C4) * env_hd;
    const auto c5_hd = po.play(half_dot, C5) * env_hd;
    const auto d3_hd = po.play(half_dot, D3) * env_hd;
    const auto d4_hd = po.play(half_dot, D4) * env_hd;
    const auto d5_hd = po.play(half_dot, D5) * env_hd;
    const auto e5_hd = po.play(half_dot, E5) * env_hd;

    const auto a2_2hd = po.play(half_dot_2, A2) * env_2hd;
    const auto a3_2hd = po.play(half_dot_2, A3) * env_2hd;
    const auto b3_2hd = po.play(half_dot_2, B3) * env_2hd;
    const auto b4_2hd = po.play(half_dot_2, B4) * env_2hd;
    const auto f2_2hd = po.play(half_dot_2, F2) * env_2hd;
    const auto f3_2hd = po.play(half_dot_2, F3) * env_2hd;
    const auto g2_2hd = po.play(half_dot_2, G2) * env_2hd;
    const auto g3_2hd = po.play(half_dot_2, G3) * env_2hd;

    // Utility to compute the time at the measure and quarter note given.
    auto move_to = [&](float64 measure, float64 qnote = 1.0, float64 enote = 1.0, float64 snote = 1.0)
    {
        static auto m = -1.0;
        if (std::abs(measure - m) > 0.5)
        {
            m = measure;
            cout << measure << " ";
            fflush(stdout);
        }
        return 3.0 * _quarter * (measure - 1.0) + _quarter * (qnote - 1.0) + _eighth * (enote - 1.0) + _sixth * (snote - 1.0);
    };

    auto play = [&](float64 time, const auto&... args)
    {
        play_n(out, time, args...);
    };

    cout << "\nRendering measures ..."; fflush(stdout);

//~#define ORGAN_TEST
#ifdef ORGAN_TEST
    //-------------------------------------------------------------------------
    // Testing C scales on the organ.
    {
//~        {
//~            auto e4 = po.play(quarter, E4) * env;
//~            auto e4_s = po.play(sixth, E4) * env_s;
//~        }


//~        play(move_to(1, 1), c5, c4);
//~        play(move_to(1, 2), d5, d4);
//~        play(move_to(1, 3), e5, e4);
//~        play(move_to(2, 1), f5, f4);
//~        play(move_to(2, 2), g5, g4);
//~        play(move_to(2, 3), a5, a4);
//~        play(move_to(3, 1), b5, b4);
//~        play(move_to(3, 2), c6, c5);
//~        play(move_to(3, 3), d6, d5);


//~        play(move_to(4, 1, 1), c5_e, c4_e);
//~        play(move_to(4, 1, 2), d5_e, d4_e);
//~        play(move_to(4, 1, 3), e5_e, e4_e);
//~        play(move_to(4, 1, 4), f5_e, f4_e);
//~        play(move_to(4, 1, 5), g5_e, g4_e);
//~        play(move_to(4, 1, 6), a5_e, a4_e);
//~        play(move_to(5, 1, 1), b5_e, b4_e);
//~        play(move_to(5, 1, 2), c6_e, c5_e);
//~        play(move_to(5, 1, 3), d6_e, d5_e);
//~        play(move_to(5, 1, 4), e6_e, e5_e);
//~        play(move_to(5, 1, 5), f6_e, f5_e);
//~        play(move_to(5, 1, 6), g6_e, g5_e);



//~        play(move_to(6, 1, 1,  1), c5_s, c4_s);
//~        play(move_to(6, 1, 1,  2), d5_s, d4_s);
//~        play(move_to(6, 1, 1,  3), e5_s, e4_s);
//~        play(move_to(6, 1, 1,  4), f5_s, f4_s);
//~        play(move_to(6, 1, 1,  5), g5_s, g4_s);
//~        play(move_to(6, 1, 1,  6), a5_s, a4_s);
//~        play(move_to(6, 1, 1,  7), b5_s, b4_s);
//~        play(move_to(6, 1, 1,  8), c6_s, c5_s);
//~        play(move_to(6, 1, 1,  9), d6_s, d5_s);
//~        play(move_to(6, 1, 1, 10), e6_s, e5_s);
//~        play(move_to(6, 1, 1, 11), f6_s, f5_s);
//~        play(move_to(6, 1, 1, 12), g6_s, g5_s);

//~        play(move_to(1, 1, 1,  1), b5_s * 0.5, a4_s);
//~        play(move_to(1, 1, 1,  2), c6_s * 0.5, c5_s * 0.5);
//~        play(move_to(1, 1, 1,  3), b5_s * 0.5, e5_s * 0.5);
//~        play(move_to(1, 1, 1,  4), c6_s * 0.5);
//~        play(move_to(1, 1, 1,  5), b5_s * 0.5, a4_s);
//~        play(move_to(1, 1, 1,  6), c6_s * 0.5, c5_s * 0.5);
//~        play(move_to(1, 1, 1,  7), b5_s * 0.5, e5_s * 0.5);
//~        play(move_to(1, 1, 1,  8), c6_s * 0.5);
//~        play(move_to(1, 1, 1,  9), b5_s * 0.5, a4_s);
//~        play(move_to(1, 1, 1, 10), c6_s * 0.5, c5_s * 0.5);
//~        play(move_to(1, 1, 1, 11), b5_s * 0.5, e5_s * 0.5);
//~        play(move_to(1, 1, 1, 12), c6_s * 0.5);

//~        play(move_to(2, 1, 1,  1), b5_s * 0.5, b4_s);
//~        play(move_to(2, 1, 1,  2), c6_s * 0.5, d5_s * 0.5);
//~        play(move_to(2, 1, 1,  3), b5_s * 0.5, e5_s * 0.5);
//~        play(move_to(2, 1, 1,  4), c6_s * 0.5);
//~        play(move_to(2, 1, 1,  5), b5_s * 0.5, b4_s);
//~        play(move_to(2, 1, 1,  6), c6_s * 0.5, d5_s * 0.5);
//~        play(move_to(2, 1, 1,  7), b5_s * 0.5, e5_s * 0.5);
//~        play(move_to(2, 1, 1,  8), c6_s * 0.5);
//~        play(move_to(2, 1, 1,  9), b5_s * 0.5, b4_s);
//~        play(move_to(2, 1, 1, 10), c6_s * 0.5, d5_s * 0.5);
//~        play(move_to(2, 1, 1, 11), b5_s * 0.5, e5_s * 0.5);
//~        play(move_to(2, 1, 1, 12), c6_s * 0.5);

//~        play(move_to(3, 1, 1,  1), b5_s * 0.5, b4_s);
//~        play(move_to(3, 1, 1,  2), c6_s * 0.5, d5_s * 0.5);
//~        play(move_to(3, 1, 1,  3), b5_s * 0.5, e5_s * 0.5);
//~        play(move_to(3, 1, 1,  4), c6_s * 0.5);
//~        play(move_to(3, 1, 1,  5), b5_s * 0.5, b4_s);
//~        play(move_to(3, 1, 1,  6), c6_s * 0.5, d5_s * 0.5);
//~        play(move_to(3, 1, 1,  7), b5_s * 0.5, e5_s * 0.5);
//~        play(move_to(3, 1, 1,  8), c6_s * 0.5);
//~        play(move_to(3, 1, 1,  9), b5_s * 0.5, b4_s);
//~        play(move_to(3, 1, 1, 10), c6_s * 0.5, d5_s * 0.5);
//~        play(move_to(3, 1, 1, 11), b5_s * 0.5, e5_s * 0.5);
//~        play(move_to(3, 1, 1, 12), c6_s * 0.5);

//~        play(move_to(4, 1, 1,  1), b5_s * 0.5, a4_s);
//~        play(move_to(4, 1, 1,  2), c6_s * 0.5, c5_s * 0.5);
//~        play(move_to(4, 1, 1,  3), b5_s * 0.5, e5_s * 0.5);
//~        play(move_to(4, 1, 1,  4), c6_s * 0.5);
//~        play(move_to(4, 1, 1,  5), b5_s * 0.5, a4_s);
//~        play(move_to(4, 1, 1,  6), c6_s * 0.5, c5_s * 0.5);
//~        play(move_to(4, 1, 1,  7), b5_s * 0.5, e5_s * 0.5);
//~        play(move_to(4, 1, 1,  8), c6_s * 0.5);
//~        play(move_to(4, 1, 1,  9), b5_s * 0.5, a4_s);
//~        play(move_to(4, 1, 1, 10), c6_s * 0.5, c5_s * 0.5);
//~        play(move_to(4, 1, 1, 11), b5_s * 0.5, e5_s * 0.5);
//~        play(move_to(4, 1, 1, 12), c6_s * 0.5);

        // 80
        play(move_to(1, 1, 1,  1), b5_s, b4_s);
        play(move_to(1, 1, 1,  2), c6_s, d5_s);
        play(move_to(1, 1, 1,  3), b5_s, e5_s);
        play(move_to(1, 1, 1,  4), c6_s);
        play(move_to(1, 1, 1,  5), b5_s, b4_s);
        play(move_to(1, 1, 1,  6), c6_s, d5_s);
        play(move_to(1, 1, 1,  7), b5_s, e5_s);
        play(move_to(1, 1, 1,  8), c6_s);
        play(move_to(1, 1, 1,  9), b5_s, b4_s);
        play(move_to(1, 1, 1, 10), c6_s, d5_s);
        play(move_to(1, 1, 1, 11), b5_s, e5_s);
        play(move_to(1, 1, 1, 12), c6_s);

        // 81
        play(move_to(2, 1, 1,  1), f3_s, a5, a4);
        play(move_to(2, 1, 1,  2), c4_s);
        play(move_to(2, 1, 1,  3), e4_s);
        play(move_to(2, 1, 1,  4), a4_s);
        play(move_to(2, 1, 1,  5), e4_s, e6, e5);
        play(move_to(2, 1, 1,  6), c4_s);
        play(move_to(2, 1, 1,  7), f3_s);
        play(move_to(2, 1, 1,  8), c4_s);
        play(move_to(2, 1, 1,  9), e4_s);
        play(move_to(2, 1, 1, 10), a4_s);
        play(move_to(2, 1, 1, 11), e4_s);
        play(move_to(2, 1, 1, 12), c4_s);

        // 82
        play(move_to(3, 1, 1,  1), f3_s, a5, a4);
        play(move_to(3, 1, 1,  2), c4_s);
        play(move_to(3, 1, 1,  3), e4_s);
        play(move_to(3, 1, 1,  4), a4_s);
        play(move_to(3, 1, 1,  5), e4_s, e6, e5);
        play(move_to(3, 1, 1,  6), c4_s);
        play(move_to(3, 1, 1,  7), f3_s);
        play(move_to(3, 1, 1,  8), c4_s);
        play(move_to(3, 1, 1,  9), e4_s);
        play(move_to(3, 1, 1, 10), a4_s);
        play(move_to(3, 1, 1, 11), e4_s);
        play(move_to(3, 1, 1, 12), c4_s);

        // 83
        play(move_to(4, 1, 1,  1), g3_s, b5, b4);
        play(move_to(4, 1, 1,  2), d4_s);
        play(move_to(4, 1, 1,  3), e4_s);
        play(move_to(4, 1, 1,  4), b4_s);
        play(move_to(4, 1, 1,  5), e4_s, e6, e5);
        play(move_to(4, 1, 1,  6), d4_s);
        play(move_to(4, 1, 1,  7), f3_s);
        play(move_to(4, 1, 1,  8), d4_s);
        play(move_to(4, 1, 1,  9), e4_s);
        play(move_to(4, 1, 1, 10), b4_s);
        play(move_to(4, 1, 1, 11), e4_s);
        play(move_to(4, 1, 1, 12), d4_s);

        // 84
        play(move_to(5, 1, 1,  1), g3_s, b5, b4);
        play(move_to(5, 1, 1,  2), d4_s);
        play(move_to(5, 1, 1,  3), e4_s);
        play(move_to(5, 1, 1,  4), b4_s);
        play(move_to(5, 1, 1,  5), e4_s, e6, e5);
        play(move_to(5, 1, 1,  6), d4_s);
        play(move_to(5, 1, 1,  7), f3_s);
        play(move_to(5, 1, 1,  8), d4_s);
        play(move_to(5, 1, 1,  9), e4_s);
        play(move_to(5, 1, 1, 10), b4_s);
        play(move_to(5, 1, 1, 11), e4_s);
        play(move_to(5, 1, 1, 12), d4_s);

        // 85
        play(move_to(6, 1, 1,  1), a3_s, c6, c5);
        play(move_to(6, 1, 1,  2), c4_s);
        play(move_to(6, 1, 1,  3), e4_s);
        play(move_to(6, 1, 1,  4), a4_s);
        play(move_to(6, 1, 1,  5), e4_s, e6, e5);
        play(move_to(6, 1, 1,  6), c4_s);
        play(move_to(6, 1, 1,  7), f3_s);
        play(move_to(6, 1, 1,  8), c4_s);
        play(move_to(6, 1, 1,  9), e4_s);
        play(move_to(6, 1, 1, 10), a4_s);
        play(move_to(6, 1, 1, 11), e4_s);
        play(move_to(6, 1, 1, 12), c4_s);

        // 86
        play(move_to(7, 1, 1,  1), a3_s, c6, c5);
        play(move_to(7, 1, 1,  2), c4_s);
        play(move_to(7, 1, 1,  3), e4_s);
        play(move_to(7, 1, 1,  4), a4_s);
        play(move_to(7, 1, 1,  5), e4_s, e6, e5);
        play(move_to(7, 1, 1,  6), c4_s);
        play(move_to(7, 1, 1,  7), f3_s);
        play(move_to(7, 1, 1,  8), c4_s);
        play(move_to(7, 1, 1,  9), e4_s);
        play(move_to(7, 1, 1, 10), a4_s);
        play(move_to(7, 1, 1, 11), e4_s);
        play(move_to(7, 1, 1, 12), c4_s);

        // 87
        play(move_to(8, 1, 1,  1), g3_s, d6, d5);
        play(move_to(8, 1, 1,  2), d4_s);
        play(move_to(8, 1, 1,  3), e4_s);
        play(move_to(8, 1, 1,  4), b4_s);
        play(move_to(8, 1, 1,  5), e4_s, e6, e5);
        play(move_to(8, 1, 1,  6), d4_s);
        play(move_to(8, 1, 1,  7), g3_s);
        play(move_to(8, 1, 1,  8), d4_s);
        play(move_to(8, 1, 1,  9), e4_s);
        play(move_to(8, 1, 1, 10), b4_s);
        play(move_to(8, 1, 1, 11), e4_s);
        play(move_to(8, 1, 1, 12), d4_s);



    }
#endif
#ifndef ORGAN_TEST


    // The first 12 E4's are marked with <, start soft and increase volume.
    auto v = linspace(0.1, 1.0, 12);
    for (std::size_t i = 0; i < 12; ++i) play(move_to(1, i + 1.0), e4 * v[i]);

    play(move_to(5, 1), a4_hd, a3_hd, e4);
    play(move_to(5, 2), e4);
    play(move_to(5, 3), e4);

    play(move_to(6, 1), b4_2hd, b3_2hd, e4);
    play(move_to(6, 2), e4);
    play(move_to(6, 3), e4);

    play(move_to(7, 1), e4);
    play(move_to(7, 2), e4);
    play(move_to(7, 3), e4);

    play(move_to(8, 1), a4,e4, a3);
    play(move_to(8, 2), b4,e4, b3);
    play(move_to(8, 3), c5,e4, c4);

    play(move_to(9, 1), b4, e4, b3);
    play(move_to(9, 2), a4, e4, a3);
    play(move_to(9, 3), b4, e4, b3);

    play(move_to(10, 1), c5_hd, e4, c4_hd);
    play(move_to(10, 2), e4);
    play(move_to(10, 3), e4);

    play(move_to(11, 1), b4_2hd, e4, b3_2hd);
    play(move_to(11, 2), e4);
    play(move_to(11, 3), e4);

    play(move_to(12, 1), e4);
    play(move_to(12, 2), e4);
    play(move_to(12, 3), e4);

    play(move_to(13, 1), a4_h, e4, a3_h);
    play(move_to(13, 2), e4);
    play(move_to(13, 3), e5, e4);

    play(move_to(14, 1), c5_hd, e4, c4_hd);
    play(move_to(14, 2), e4);
    play(move_to(14, 3), e4);

    play(move_to(15, 1), b4_2hd, e4, b3_2hd);
    play(move_to(15, 2), e4);
    play(move_to(15, 3), e4);

    play(move_to(16, 1), e4);
    play(move_to(16, 2), e4);
    play(move_to(16, 3), e4);

    play(move_to(17, 1), a4, e4, a3);
    play(move_to(17, 2), e5, e4);
    play(move_to(17, 3), c5, e4, c4);

    play(move_to(18, 1), b4, e4, b3);
    play(move_to(18, 2), a4, e4, a3);
    play(move_to(18, 3), b4, e4, b3);

    play(move_to(19, 1), c5_hd, e4, c4_hd);
    play(move_to(19, 2), e4);
    play(move_to(19, 3), e4);

    play(move_to(20, 1), b4_2hd, e4, b3_2hd);
    play(move_to(20, 2), e4);
    play(move_to(20, 3), e4);

    play(move_to(21, 1), e4);
    play(move_to(21, 2), e4);
    play(move_to(21, 3), e4);

    play(move_to(22, 1), a4, e4, a3);
    play(move_to(22, 2), e5, e4);
    play(move_to(22, 3), c5, e4, c4);

    play(move_to(23, 1), b4, e4, b3);
    play(move_to(23, 2), a4, e4, a3);
    play(move_to(23, 3), b4, e4, b3);

    play(move_to(24, 1), c5_hd, e4, c4_hd);
    play(move_to(24, 2), e4);
    play(move_to(24, 3), e4);

    play(move_to(25, 1), d5_hd, e4, d4_hd);
    play(move_to(25, 2), e4);
    play(move_to(25, 3), e4);

    play(move_to(26, 1), e5_hd, e4);
    play(move_to(26, 2), e4);
    play(move_to(26, 3), e4);

    play(move_to(27, 1, 1), a4_e, e4, a3_hd);
    play(move_to(27, 1, 2), b4_e);
    play(move_to(27, 1, 3), c5_e, e4);
    play(move_to(27, 1, 4), b4_e);
    play(move_to(27, 1, 5), a4_e, e4);
    play(move_to(27, 1, 6), c5_e);

    play(move_to(28, 1, 1), b4_e, e4, b3_2hd);
    play(move_to(28, 1, 2), a4_e);
    play(move_to(28, 1, 3), g4_e, e4);
    play(move_to(28, 1, 4), a4_e);
    play(move_to(28, 1, 5), b4_e, e4);
    play(move_to(28, 1, 6), g4_e);

    play(move_to(29, 1, 1), b4_e, e4);
    play(move_to(29, 1, 2), a4_e);
    play(move_to(29, 1, 3), g4_e, e4);
    play(move_to(29, 1, 4), a4_e);
    play(move_to(29, 1, 5), b4_e, e4);
    play(move_to(29, 1, 6), g4_e);

    play(move_to(30, 1, 1), a4_e, e4, a3);
    play(move_to(30, 1, 2), c5_e);
    play(move_to(30, 1, 3), b4_e, e4, b3);
    play(move_to(30, 1, 4), d5_e);
    play(move_to(30, 1, 5), c5_e, e4, c4);
    play(move_to(30, 1, 6), a4_e);

    play(move_to(31, 1, 1), b4_e, e4, b3);
    play(move_to(31, 1, 2), g4_e);
    play(move_to(31, 1, 3), a4_e, e4, a3);
    play(move_to(31, 1, 4), c5_e);
    play(move_to(31, 1, 5), b4_e, e4, b3);
    play(move_to(31, 1, 6), a4_e);

    play(move_to(32, 1, 1), c5_e, e4, c4_hd);
    play(move_to(32, 1, 2), d5_e);
    play(move_to(32, 1, 3), c5_e, e4);
    play(move_to(32, 1, 4), b4_e);
    play(move_to(32, 1, 5), a4_e, e4);
    play(move_to(32, 1, 6), c5_e);

    play(move_to(33, 1, 1), b4_e, e4, b3_2hd);
    play(move_to(33, 1, 2), a4_e);
    play(move_to(33, 1, 3), g4_e, e4);
    play(move_to(33, 1, 4), a4_e);
    play(move_to(33, 1, 5), b4_e, e4);
    play(move_to(33, 1, 6), g4_e);

    play(move_to(34, 1, 1), b4_e, e4);
    play(move_to(34, 1, 2), a4_e);
    play(move_to(34, 1, 3), g4_e, e4);
    play(move_to(34, 1, 4), e5_e);
    play(move_to(34, 1, 5), b4_e, e4);
    play(move_to(34, 1, 6), g4_e);

    play(move_to(35, 1, 1,  1), a4_s, a3_hd);
    play(move_to(35, 1, 1,  2), e4_s);
    play(move_to(35, 1, 1,  3), b4_s);
    play(move_to(35, 1, 1,  4), e4_s);
    play(move_to(35, 1, 1,  5), c5_s);
    play(move_to(35, 1, 1,  6), e4_s);
    play(move_to(35, 1, 1,  7), b4_s);
    play(move_to(35, 1, 1,  8), e4_s);
    play(move_to(35, 1, 1,  9), a4_s);
    play(move_to(35, 1, 1, 10), e4_s);
    play(move_to(35, 1, 1, 11), c5_s);
    play(move_to(35, 1, 1, 12), e4_s);

    play(move_to(36, 1, 1,  1), b4_s, b3_2hd);
    play(move_to(36, 1, 1,  2), e4_s);
    play(move_to(36, 1, 1,  3), a4_s);
    play(move_to(36, 1, 1,  4), e4_s);
    play(move_to(36, 1, 1,  5), g4_s);
    play(move_to(36, 1, 1,  6), e4_s);
    play(move_to(36, 1, 1,  7), a4_s);
    play(move_to(36, 1, 1,  8), e4_s);
    play(move_to(36, 1, 1,  9), b4_s);
    play(move_to(36, 1, 1, 10), e4_s);
    play(move_to(36, 1, 1, 11), g4_s);
    play(move_to(36, 1, 1, 12), e4_s);

    play(move_to(37, 1, 1,  1), b4_s);
    play(move_to(37, 1, 1,  2), e4_s);
    play(move_to(37, 1, 1,  3), a4_s);
    play(move_to(37, 1, 1,  4), e4_s);
    play(move_to(37, 1, 1,  5), g4_s);
    play(move_to(37, 1, 1,  6), e4_s);
    play(move_to(37, 1, 1,  7), e5_s);
    play(move_to(37, 1, 1,  8), e4_s);
    play(move_to(37, 1, 1,  9), b4_s);
    play(move_to(37, 1, 1, 10), e4_s);
    play(move_to(37, 1, 1, 11), g4_s);
    play(move_to(37, 1, 1, 12), e4_s);

    play(move_to(38, 1, 1,  1), a4_s, a3);
    play(move_to(38, 1, 1,  2), e4_s);
    play(move_to(38, 1, 1,  3), c5_s);
    play(move_to(38, 1, 1,  4), e4_s);
    play(move_to(38, 1, 1,  5), b4_s, b3);
    play(move_to(38, 1, 1,  6), e4_s);
    play(move_to(38, 1, 1,  7), d5_s);
    play(move_to(38, 1, 1,  8), e4_s);
    play(move_to(38, 1, 1,  9), c5_s, c4);
    play(move_to(38, 1, 1, 10), e4_s);
    play(move_to(38, 1, 1, 11), a4_s);
    play(move_to(38, 1, 1, 12), e4_s);

    play(move_to(39, 1, 1,  1), b4_s, b3);
    play(move_to(39, 1, 1,  2), e4_s);
    play(move_to(39, 1, 1,  3), g4_s);
    play(move_to(39, 1, 1,  4), e4_s);
    play(move_to(39, 1, 1,  5), a4_s, a3);
    play(move_to(39, 1, 1,  6), e4_s);
    play(move_to(39, 1, 1,  7), c5_s);
    play(move_to(39, 1, 1,  8), e4_s);
    play(move_to(39, 1, 1,  9), b4_s, b3);
    play(move_to(39, 1, 1, 10), e4_s);
    play(move_to(39, 1, 1, 11), d5_s);
    play(move_to(39, 1, 1, 12), e4_s);

    play(move_to(40, 1, 1,  1), c5_s, c4_hd);
    play(move_to(40, 1, 1,  2), e4_s);
    play(move_to(40, 1, 1,  3), d5_s);
    play(move_to(40, 1, 1,  4), e4_s);
    play(move_to(40, 1, 1,  5), c5_s);
    play(move_to(40, 1, 1,  6), e4_s);
    play(move_to(40, 1, 1,  7), b4_s);
    play(move_to(40, 1, 1,  8), e4_s);
    play(move_to(40, 1, 1,  9), a4_s);
    play(move_to(40, 1, 1, 10), e4_s);
    play(move_to(40, 1, 1, 11), c5_s);
    play(move_to(40, 1, 1, 12), e4_s);

    play(move_to(41, 1, 1,  1), b4_s, b3_2hd);
    play(move_to(41, 1, 1,  2), e4_s);
    play(move_to(41, 1, 1,  3), a4_s);
    play(move_to(41, 1, 1,  4), e4_s);
    play(move_to(41, 1, 1,  5), g4_s);
    play(move_to(41, 1, 1,  6), e4_s);
    play(move_to(41, 1, 1,  7), a4_s);
    play(move_to(41, 1, 1,  8), e4_s);
    play(move_to(41, 1, 1,  9), b4_s);
    play(move_to(41, 1, 1, 10), e4_s);
    play(move_to(41, 1, 1, 11), g4_s);
    play(move_to(41, 1, 1, 12), e4_s);

    play(move_to(42, 1, 1,  1), b4_s);
    play(move_to(42, 1, 1,  2), e4_s);
    play(move_to(42, 1, 1,  3), a4_s);
    play(move_to(42, 1, 1,  4), e4_s);
    play(move_to(42, 1, 1,  5), g4_s);
    play(move_to(42, 1, 1,  6), e4_s);
    play(move_to(42, 1, 1,  7), e5_s);
    play(move_to(42, 1, 1,  8), e4_s);
    play(move_to(42, 1, 1,  9), b4_s);
    play(move_to(42, 1, 1, 10), e4_s);
    play(move_to(42, 1, 1, 11), g4_s);
    play(move_to(42, 1, 1, 12), e4_s);

    play(move_to(43, 1, 1,  1), a4_s, a3_h);
    play(move_to(43, 1, 1,  2), e4_s);
    play(move_to(43, 1, 1,  3), b4_s);
    play(move_to(43, 1, 1,  4), e4_s);
    play(move_to(43, 1, 1,  5), c5_s);
    play(move_to(43, 1, 1,  6), e4_s);
    play(move_to(43, 1, 1,  7), b4_s);
    play(move_to(43, 1, 1,  8), e4_s);
    play(move_to(43, 1, 1,  9), e5_s, e4_s);
    play(move_to(43, 1, 1, 10), e4_s);
    play(move_to(43, 1, 1, 11), b4_s);
    play(move_to(43, 1, 1, 12), e4_s);

    play(move_to(44, 1, 1,  1), c5_s, c4_hd);
    play(move_to(44, 1, 1,  2), e4_s);
    play(move_to(44, 1, 1,  3), d5_s);
    play(move_to(44, 1, 1,  4), e4_s);
    play(move_to(44, 1, 1,  5), c5_s);
    play(move_to(44, 1, 1,  6), e4_s);
    play(move_to(44, 1, 1,  7), b4_s);
    play(move_to(44, 1, 1,  8), e4_s);
    play(move_to(44, 1, 1,  9), a4_s);
    play(move_to(44, 1, 1, 10), e4_s);
    play(move_to(44, 1, 1, 11), c5_s);
    play(move_to(44, 1, 1, 12), e4_s);

    play(move_to(45, 1, 1,  1), b4_s, b3_2hd);
    play(move_to(45, 1, 1,  2), e4_s);
    play(move_to(45, 1, 1,  3), a4_s);
    play(move_to(45, 1, 1,  4), e4_s);
    play(move_to(45, 1, 1,  5), g4_s);
    play(move_to(45, 1, 1,  6), e4_s);
    play(move_to(45, 1, 1,  7), a4_s);
    play(move_to(45, 1, 1,  8), e4_s);
    play(move_to(45, 1, 1,  9), b4_s);
    play(move_to(45, 1, 1, 10), e4_s);
    play(move_to(45, 1, 1, 11), g4_s);
    play(move_to(45, 1, 1, 12), e4_s);

    play(move_to(46, 1, 1,  1), b4_s);
    play(move_to(46, 1, 1,  2), e4_s);
    play(move_to(46, 1, 1,  3), a4_s);
    play(move_to(46, 1, 1,  4), e4_s);
    play(move_to(46, 1, 1,  5), g4_s);
    play(move_to(46, 1, 1,  6), e4_s);
    play(move_to(46, 1, 1,  7), e5_s);
    play(move_to(46, 1, 1,  8), e4_s);
    play(move_to(46, 1, 1,  9), b4_s);
    play(move_to(46, 1, 1, 10), e4_s);
    play(move_to(46, 1, 1, 11), g4_s);
    play(move_to(46, 1, 1, 12), e4_s);

    play(move_to(47, 1, 1,  1), a4_s, a3);
    play(move_to(47, 1, 1,  2), e4_s);
    play(move_to(47, 1, 1,  3), c5_s);
    play(move_to(47, 1, 1,  4), e4_s);
    play(move_to(47, 1, 1,  5), e5_s, e4);
    play(move_to(47, 1, 1,  6), e4_s);
    play(move_to(47, 1, 1,  7), b4_s);
    play(move_to(47, 1, 1,  8), e4_s);
    play(move_to(47, 1, 1,  9), c5_s, c4);
    play(move_to(47, 1, 1, 10), e4_s);
    play(move_to(47, 1, 1, 11), a4_s);
    play(move_to(47, 1, 1, 12), e4_s);

    play(move_to(48, 1, 1,  1), b4_s, b3);
    play(move_to(48, 1, 1,  2), e4_s);
    play(move_to(48, 1, 1,  3), g4_s);
    play(move_to(48, 1, 1,  4), e4_s);
    play(move_to(48, 1, 1,  5), a4_s, a3);
    play(move_to(48, 1, 1,  6), e4_s);
    play(move_to(48, 1, 1,  7), b4_s);
    play(move_to(48, 1, 1,  8), e4_s);
    play(move_to(48, 1, 1,  9), c5_s, c4);
    play(move_to(48, 1, 1, 10), e4_s);
    play(move_to(48, 1, 1, 11), a4_s);
    play(move_to(48, 1, 1, 12), e4_s);

    play(move_to(49, 1, 1,  1), c5_s, c4_hd);
    play(move_to(49, 1, 1,  2), e4_s);
    play(move_to(49, 1, 1,  3), b4_s);
    play(move_to(49, 1, 1,  4), e4_s);
    play(move_to(49, 1, 1,  5), a4_s);
    play(move_to(49, 1, 1,  6), e4_s);
    play(move_to(49, 1, 1,  7), b4_s);
    play(move_to(49, 1, 1,  8), e4_s);
    play(move_to(49, 1, 1,  9), c5_s);
    play(move_to(49, 1, 1, 10), e4_s);
    play(move_to(49, 1, 1, 11), a4_s);
    play(move_to(49, 1, 1, 12), e4_s);

    play(move_to(50, 1, 1,  1), b4_s, b3_2hd);
    play(move_to(50, 1, 1,  2), e4_s);
    play(move_to(50, 1, 1,  3), a4_s);
    play(move_to(50, 1, 1,  4), e4_s);
    play(move_to(50, 1, 1,  5), g4_s);
    play(move_to(50, 1, 1,  6), e4_s);
    play(move_to(50, 1, 1,  7), a4_s);
    play(move_to(50, 1, 1,  8), e4_s);
    play(move_to(50, 1, 1,  9), b4_s);
    play(move_to(50, 1, 1, 10), e4_s);
    play(move_to(50, 1, 1, 11), g4_s);
    play(move_to(50, 1, 1, 12), e4_s);

    play(move_to(51, 1, 1,  1), b4_s);
    play(move_to(51, 1, 1,  2), e4_s);
    play(move_to(51, 1, 1,  3), a4_s);
    play(move_to(51, 1, 1,  4), e4_s);
    play(move_to(51, 1, 1,  5), g4_s);
    play(move_to(51, 1, 1,  6), e4_s);
    play(move_to(51, 1, 1,  7), e5_s);
    play(move_to(51, 1, 1,  8), e4_s);
    play(move_to(51, 1, 1,  9), b4_s);
    play(move_to(51, 1, 1, 10), e4_s);
    play(move_to(51, 1, 1, 11), g4_s);
    play(move_to(51, 1, 1, 12), e4_s);

    play(move_to(52, 1, 1,  1), a4_s, a3);
    play(move_to(52, 1, 1,  2), e4_s);
    play(move_to(52, 1, 1,  3), c5_s);
    play(move_to(52, 1, 1,  4), e4_s);
    play(move_to(52, 1, 1,  5), e5_s, e4);
    play(move_to(52, 1, 1,  6), e4_s);
    play(move_to(52, 1, 1,  7), b4_s);
    play(move_to(52, 1, 1,  8), e4_s);
    play(move_to(52, 1, 1,  9), c5_s, c4);
    play(move_to(52, 1, 1, 10), e4_s);
    play(move_to(52, 1, 1, 11), a4_s);
    play(move_to(52, 1, 1, 12), e4_s);

    play(move_to(53, 1, 1,  1), b4_s, b3);
    play(move_to(53, 1, 1,  2), e4_s);
    play(move_to(53, 1, 1,  3), g4_s);
    play(move_to(53, 1, 1,  4), e4_s);
    play(move_to(53, 1, 1,  5), a4_s, a3);
    play(move_to(53, 1, 1,  6), e4_s);
    play(move_to(53, 1, 1,  7), c5_s);
    play(move_to(53, 1, 1,  8), e4_s);
    play(move_to(53, 1, 1,  9), b4_s, b3);
    play(move_to(53, 1, 1, 10), e4_s);
    play(move_to(53, 1, 1, 11), d5_s);
    play(move_to(53, 1, 1, 12), e4_s);

    play(move_to(54, 1, 1,  1), c5_s, c4_hd);
    play(move_to(54, 1, 1,  2), e4_s);
    play(move_to(54, 1, 1,  3), b4_s);
    play(move_to(54, 1, 1,  4), e4_s);
    play(move_to(54, 1, 1,  5), a4_s);
    play(move_to(54, 1, 1,  6), e4_s);
    play(move_to(54, 1, 1,  7), b4_s);
    play(move_to(54, 1, 1,  8), e4_s);
    play(move_to(54, 1, 1,  9), c5_s);
    play(move_to(54, 1, 1, 10), e4_s);
    play(move_to(54, 1, 1, 11), a4_s);
    play(move_to(54, 1, 1, 12), e4_s);

    play(move_to(55, 1, 1,  1), d5_s, d4_hd);
    play(move_to(55, 1, 1,  2), e4_s);
    play(move_to(55, 1, 1,  3), c5_s);
    play(move_to(55, 1, 1,  4), e4_s);
    play(move_to(55, 1, 1,  5), b4_s);
    play(move_to(55, 1, 1,  6), e4_s);
    play(move_to(55, 1, 1,  7), c5_s);
    play(move_to(55, 1, 1,  8), e4_s);
    play(move_to(55, 1, 1,  9), d5_s);
    play(move_to(55, 1, 1, 10), e4_s);
    play(move_to(55, 1, 1, 11), b4_s);
    play(move_to(55, 1, 1, 12), e4_s);

    v = linspace(1.0, 0.5, 12);

    play(move_to(56, 1, 1), e5_e * v[0], e4_e * v[0]);
    play(move_to(56, 1, 2), e4_e * v[1]);
    play(move_to(56, 1, 3), e5_e * v[2]);
    play(move_to(56, 1, 4), e4_e * v[3]);
    play(move_to(56, 1, 5), e5_e * v[4]);
    play(move_to(56, 1, 6), e4_e * v[5]);

    play(move_to(57, 1, 1), e5_e * v[6]);
    play(move_to(57, 1, 2), e4_e * v[7]);
    play(move_to(57, 1, 3), e5_e * v[8]);
    play(move_to(57, 1, 4), e4_e * v[9]);
    play(move_to(57, 1, 5), e5_e * v[10]);
    play(move_to(57, 1, 6), e4_e * v[11]);

    v = linspace(0.5, 1.0, 6);

    play(move_to(58, 1), e5 * v[0], e4 * v[0]);
    play(move_to(58, 2), e5 * v[1], e4 * v[1]);
    play(move_to(58, 3), e5 * v[2], e4 * v[2]);

    play(move_to(59, 1), e5 * v[3], e4 * v[3]);
    play(move_to(59, 2), e5 * v[4], e4 * v[4]);
    play(move_to(59, 3), e5 * v[5], e4 * v[5]);

    play(move_to(60, 1), a4, e4, f3_2hd);
    play(move_to(60, 2), e5, e4);
    play(move_to(60, 3), e4);

    play(move_to(61, 1), a4, e4);
    play(move_to(61, 2), e5, e4);
    play(move_to(61, 3), e4);

    play(move_to(62, 1), b4, e4, g3_2hd);
    play(move_to(62, 2), e5, e4);
    play(move_to(62, 3), e4);

    play(move_to(63, 1), b4, e4);
    play(move_to(63, 2), e5, e4);
    play(move_to(63, 3), e4);

    play(move_to(64, 1), c5, e4, a3_2hd);
    play(move_to(64, 2), e5, e4);
    play(move_to(64, 3), e4);

    play(move_to(65, 1), c5, e4);
    play(move_to(65, 2), e5, e4);
    play(move_to(65, 3), e4);

    play(move_to(66, 1), d5, e4, g3_2hd);
    play(move_to(66, 2), e5, e4);
    play(move_to(66, 3), e4);

    play(move_to(67, 1), d5, e4);
    play(move_to(67, 2), e5, e4);
    play(move_to(67, 3), b4, e4);

    play(move_to(68, 1, 1), a4_e, e4, f3_2hd, f2_2hd);
    play(move_to(68, 1, 2), e5_e);
    play(move_to(68, 1, 3), e5_e, e4);
    play(move_to(68, 1, 4), e5_e);
    play(move_to(68, 1, 5), e4);
    play(move_to(68, 1, 6), e5_e);

    play(move_to(69, 1, 1), a4_e, e4);
    play(move_to(69, 1, 2), e5_e);
    play(move_to(69, 1, 3), e5_e, e4);
    play(move_to(69, 1, 4), e5_e);
    play(move_to(69, 1, 5), e4);
    play(move_to(69, 1, 6), e5_e);

    play(move_to(70, 1, 1), b4_e, e4, g3_2hd, g2_2hd);
    play(move_to(70, 1, 2), e5_e);
    play(move_to(70, 1, 3), e5_e, e4);
    play(move_to(70, 1, 4), e5_e);
    play(move_to(70, 1, 5), e4);
    play(move_to(70, 1, 6), e5_e);

    play(move_to(71, 1, 1), b4_e, e4);
    play(move_to(71, 1, 2), e5_e);
    play(move_to(71, 1, 3), e5_e, e4);
    play(move_to(71, 1, 4), e5_e);
    play(move_to(71, 1, 5), e4);
    play(move_to(71, 1, 6), e5_e);

    play(move_to(72, 1, 1), c5_e, e4, a3_2hd, a2_2hd);
    play(move_to(72, 1, 2), e5_e);
    play(move_to(72, 1, 3), e5_e, e4);
    play(move_to(72, 1, 4), e5_e);
    play(move_to(72, 1, 5), e4);
    play(move_to(72, 1, 6), e5_e);

    play(move_to(73, 1, 1), c5_e, e4);
    play(move_to(73, 1, 2), e5_e);
    play(move_to(73, 1, 3), e5_e, e4);
    play(move_to(73, 1, 4), e5_e);
    play(move_to(73, 1, 5), e4);
    play(move_to(73, 1, 6), e5_e);

    play(move_to(74, 1, 1), d5_e, e4, g3_2hd, g2_2hd);
    play(move_to(74, 1, 2), e5_e);
    play(move_to(74, 1, 3), e5_e, e4);
    play(move_to(74, 1, 4), e5_e);
    play(move_to(74, 1, 5), e4);
    play(move_to(74, 1, 6), e5_e);

    play(move_to(75, 1, 1), d5_e, e4);
    play(move_to(75, 1, 2), e5_e);
    play(move_to(75, 1, 3), e5_e, e4);
    play(move_to(75, 1, 4), e5_e);
    play(move_to(75, 1, 5), e4);
    play(move_to(75, 1, 6), e5_e);

    play(move_to(76, 1, 1,  1), b5_s, a4_s);
    play(move_to(76, 1, 1,  2), c6_s, c5_s);
    play(move_to(76, 1, 1,  3), b5_s, e5_s);
    play(move_to(76, 1, 1,  4), c6_s);
    play(move_to(76, 1, 1,  5), b5_s, a4_s);
    play(move_to(76, 1, 1,  6), c6_s, c5_s);
    play(move_to(76, 1, 1,  7), b5_s, e5_s);
    play(move_to(76, 1, 1,  8), c6_s);
    play(move_to(76, 1, 1,  9), b5_s, a4_s);
    play(move_to(76, 1, 1, 10), c6_s, c5_s);
    play(move_to(76, 1, 1, 11), b5_s, e5_s);
    play(move_to(76, 1, 1, 12), c6_s);

    play(move_to(77, 1, 1,  1), b5_s, b4_s);
    play(move_to(77, 1, 1,  2), c6_s, d5_s);
    play(move_to(77, 1, 1,  3), b5_s, e5_s);
    play(move_to(77, 1, 1,  4), c6_s);
    play(move_to(77, 1, 1,  5), b5_s, b4_s);
    play(move_to(77, 1, 1,  6), c6_s, d5_s);
    play(move_to(77, 1, 1,  7), b5_s, e5_s);
    play(move_to(77, 1, 1,  8), c6_s);
    play(move_to(77, 1, 1,  9), b5_s, b4_s);
    play(move_to(77, 1, 1, 10), c6_s, d5_s);
    play(move_to(77, 1, 1, 11), b5_s, e5_s);
    play(move_to(77, 1, 1, 12), c6_s);

    play(move_to(78, 1, 1,  1), b5_s, b4_s);
    play(move_to(78, 1, 1,  2), c6_s, d5_s);
    play(move_to(78, 1, 1,  3), b5_s, e5_s);
    play(move_to(78, 1, 1,  4), c6_s);
    play(move_to(78, 1, 1,  5), b5_s, b4_s);
    play(move_to(78, 1, 1,  6), c6_s, d5_s);
    play(move_to(78, 1, 1,  7), b5_s, e5_s);
    play(move_to(78, 1, 1,  8), c6_s);
    play(move_to(78, 1, 1,  9), b5_s, b4_s);
    play(move_to(78, 1, 1, 10), c6_s, d5_s);
    play(move_to(78, 1, 1, 11), b5_s, e5_s);
    play(move_to(78, 1, 1, 12), c6_s);

    play(move_to(79, 1, 1,  1), a5_s, a4_s);
    play(move_to(79, 1, 1,  2), c6_s, c5_s);
    play(move_to(79, 1, 1,  3), b5_s, e5_s);
    play(move_to(79, 1, 1,  4), c6_s);
    play(move_to(79, 1, 1,  5), b5_s, g4_s);
    play(move_to(79, 1, 1,  6), c6_s, b4_s);
    play(move_to(79, 1, 1,  7), b5_s, e5_s);
    play(move_to(79, 1, 1,  8), c6_s);
    play(move_to(79, 1, 1,  9), b5_s, a4_s);
    play(move_to(79, 1, 1, 10), c6_s, c5_s);
    play(move_to(79, 1, 1, 11), b5_s, e5_s);
    play(move_to(79, 1, 1, 12), c6_s);

    play(move_to(80, 1, 1,  1), b5_s, b4_s);
    play(move_to(80, 1, 1,  2), c6_s, d5_s);
    play(move_to(80, 1, 1,  3), b5_s, e5_s);
    play(move_to(80, 1, 1,  4), c6_s);
    play(move_to(80, 1, 1,  5), b5_s, a4_s);
    play(move_to(80, 1, 1,  6), c6_s, d5_s);
    play(move_to(80, 1, 1,  7), b5_s, e5_s);
    play(move_to(80, 1, 1,  8), c6_s);
    play(move_to(80, 1, 1,  9), b5_s, g4_s);
    play(move_to(80, 1, 1, 10), c6_s, b4_s);
    play(move_to(80, 1, 1, 11), b5_s, e5_s);
    play(move_to(80, 1, 1, 12), c6_s);

    play(move_to(81, 1, 1,  1), f3_s, a5, a4);
    play(move_to(81, 1, 1,  2), c4_s);
    play(move_to(81, 1, 1,  3), e4_s);
    play(move_to(81, 1, 1,  4), a4_s);
    play(move_to(81, 1, 1,  5), e4_s, e6, e5);
    play(move_to(81, 1, 1,  6), c4_s);
    play(move_to(81, 1, 1,  7), f3_s);
    play(move_to(81, 1, 1,  8), c4_s);
    play(move_to(81, 1, 1,  9), e4_s);
    play(move_to(81, 1, 1, 10), a4_s);
    play(move_to(81, 1, 1, 11), e4_s);
    play(move_to(81, 1, 1, 12), c4_s);

    play(move_to(82, 1, 1,  1), f3_s, a5, a4);
    play(move_to(82, 1, 1,  2), c4_s);
    play(move_to(82, 1, 1,  3), e4_s);
    play(move_to(82, 1, 1,  4), a4_s);
    play(move_to(82, 1, 1,  5), e4_s, e6, e5);
    play(move_to(82, 1, 1,  6), c4_s);
    play(move_to(82, 1, 1,  7), f3_s);
    play(move_to(82, 1, 1,  8), c4_s);
    play(move_to(82, 1, 1,  9), e4_s);
    play(move_to(82, 1, 1, 10), a4_s);
    play(move_to(82, 1, 1, 11), e4_s);
    play(move_to(82, 1, 1, 12), c4_s);

    play(move_to(83, 1, 1,  1), g3_s, b5, b4);
    play(move_to(83, 1, 1,  2), d4_s);
    play(move_to(83, 1, 1,  3), e4_s);
    play(move_to(83, 1, 1,  4), b4_s);
    play(move_to(83, 1, 1,  5), e4_s, e6, e5);
    play(move_to(83, 1, 1,  6), d4_s);
    play(move_to(83, 1, 1,  7), f3_s);
    play(move_to(83, 1, 1,  8), d4_s);
    play(move_to(83, 1, 1,  9), e4_s);
    play(move_to(83, 1, 1, 10), b4_s);
    play(move_to(83, 1, 1, 11), e4_s);
    play(move_to(83, 1, 1, 12), d4_s);

    play(move_to(84, 1, 1,  1), g3_s, b5, b4);
    play(move_to(84, 1, 1,  2), d4_s);
    play(move_to(84, 1, 1,  3), e4_s);
    play(move_to(84, 1, 1,  4), b4_s);
    play(move_to(84, 1, 1,  5), e4_s, e6, e5);
    play(move_to(84, 1, 1,  6), d4_s);
    play(move_to(84, 1, 1,  7), f3_s);
    play(move_to(84, 1, 1,  8), d4_s);
    play(move_to(84, 1, 1,  9), e4_s);
    play(move_to(84, 1, 1, 10), b4_s);
    play(move_to(84, 1, 1, 11), e4_s);
    play(move_to(84, 1, 1, 12), d4_s);

    play(move_to(85, 1, 1,  1), a3_s, c6, c5);
    play(move_to(85, 1, 1,  2), c4_s);
    play(move_to(85, 1, 1,  3), e4_s);
    play(move_to(85, 1, 1,  4), a4_s);
    play(move_to(85, 1, 1,  5), e4_s, e6, e5);
    play(move_to(85, 1, 1,  6), c4_s);
    play(move_to(85, 1, 1,  7), f3_s);
    play(move_to(85, 1, 1,  8), c4_s);
    play(move_to(85, 1, 1,  9), e4_s);
    play(move_to(85, 1, 1, 10), a4_s);
    play(move_to(85, 1, 1, 11), e4_s);
    play(move_to(85, 1, 1, 12), c4_s);

    play(move_to(86, 1, 1,  1), a3_s, c6, c5);
    play(move_to(86, 1, 1,  2), c4_s);
    play(move_to(86, 1, 1,  3), e4_s);
    play(move_to(86, 1, 1,  4), a4_s);
    play(move_to(86, 1, 1,  5), e4_s, e6, e5);
    play(move_to(86, 1, 1,  6), c4_s);
    play(move_to(86, 1, 1,  7), f3_s);
    play(move_to(86, 1, 1,  8), c4_s);
    play(move_to(86, 1, 1,  9), e4_s);
    play(move_to(86, 1, 1, 10), a4_s);
    play(move_to(86, 1, 1, 11), e4_s);
    play(move_to(86, 1, 1, 12), c4_s);

    play(move_to(87, 1, 1,  1), g3_s, d6, d5);
    play(move_to(87, 1, 1,  2), d4_s);
    play(move_to(87, 1, 1,  3), e4_s);
    play(move_to(87, 1, 1,  4), b4_s);
    play(move_to(87, 1, 1,  5), e4_s, e6, e5);
    play(move_to(87, 1, 1,  6), d4_s);
    play(move_to(87, 1, 1,  7), g3_s);
    play(move_to(87, 1, 1,  8), d4_s);
    play(move_to(87, 1, 1,  9), e4_s);
    play(move_to(87, 1, 1, 10), b4_s);
    play(move_to(87, 1, 1, 11), e4_s);
    play(move_to(87, 1, 1, 12), d4_s);

    play(move_to(88, 1, 1,  1), g3_s, d6, d5);
    play(move_to(88, 1, 1,  2), d4_s);
    play(move_to(88, 1, 1,  3), e4_s);
    play(move_to(88, 1, 1,  4), b4_s);
    play(move_to(88, 1, 1,  5), e4_s, e6, e5);
    play(move_to(88, 1, 1,  6), d4_s);
    play(move_to(88, 1, 1,  7), g3_s);
    play(move_to(88, 1, 1,  8), d4_s);
    play(move_to(88, 1, 1,  9), e4_s, b4, b5);
    play(move_to(88, 1, 1, 10), b4_s);
    play(move_to(88, 1, 1, 11), e4_s);
    play(move_to(88, 1, 1, 12), d4_s);


    //-------------------------------------------------------------------------
    // Finish up.

    auto dt = Toc();

    cout << "\nRendering took: " << dt
         << "\nFinishing up." << std::endl;

//~    auto t0 = move_to(77);
//~    auto t1 = move_to(89);

//~    out = out.substream(t0, t1 - t0);
#endif
    //-------------------------------------------------------------------------
    // Pad with silence, filter, save, play.

    out = gen.silence(1.0) << out << gen.silence(3.0);

//~    out = filter.filter(out);
    out = reverb.filter(out);

    out.normalize();
    out *= 0.666;

    out >> "interstellar-theme.wav";

    AudioPlayback pb(sr, 2, 16);
    out >> pb;

    return 0;
}

