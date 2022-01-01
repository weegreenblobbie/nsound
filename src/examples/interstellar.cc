#include <iostream>
#include <string>
#include <map>
#include <Nsound/NsoundAll.h>


using namespace Nsound;
using std::cout;

// The 88 white keys on a piano:
//     https://en.wikipedia.org/wiki/Piano_key_frequencies

const auto tune = 1.0;

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


int main(void)
{
    cout << "Constructing pipe organ and filters ..."; fflush(stdout);
    const auto sr = 22050.0;

    auto out = AudioStream(sr, 2);
    auto gen = Generator(sr);
    auto po = OrganPipe(sr);
    auto filter = FilterLowPassIIR(sr, 3, 2500.0, 0.05);
//~    auto reverb = ReverberationRoom(sr, 0.750, 1.00, 0.50, 2500.0, 0.1000);
//~    auto reverb = ReverberationRoom(sr, 0.750, 0.50, 0.50, 1500.0, 0.1000);
    auto reverb = ReverberationRoom(sr, 0.750, 0.50, 0.50, 1000.0, 0.1000);

    // Interstellar Main Theme Arranged by Patrik Pietschmann.
    //
    // Quarter notes are 100 BPM.
    //
    const auto _quarter = 60.0 / 100.0;
    const auto _eighth = _quarter / 2.0;
    const auto _sixth  = _eighth / 2.0;

    // Actual note durations.
    const auto sixth = _sixth - 0.00;
    const auto eighth = _eighth - 0.1;
    const auto quarter = _quarter - 0.1;
    const auto half = 2 * _quarter - 0.1;
    const auto half_dot = 3 * _quarter - 0.1;
    const auto half_dot_2 = 6 * _quarter - 0.1;

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

    auto env = Asdr{0.1, 0.25, 0.80, 0.25};

    auto play = [&](float64 time, const auto&... args)
    {
        play_n(out, time, args...);
    };

    cout << "\nRendering measures ..." << std::endl;

    Tic();

    //-------------------------------------------------------------------------
    // Measures 1 - 7

    const auto note_e4 = po.play(quarter, E4) * env;

    auto env_hd = Asdr{0.04, 0.10, 0.80, 0.10};

    const auto note_a4_hd = po.play(half_dot, A4) * env_hd * 1.05;
    const auto note_a3_hd = po.play(half_dot, A3) * env_hd * 1.05;

    auto env_2hd = Asdr{0.02, 0.05, 0.80, 0.05};

    const auto note_b4_2hd = po.play(half_dot_2, B4) * env_2hd * 1.20;
    const auto note_b3_2hd = po.play(half_dot_2, B3) * env_2hd * 1.20;

    // The first 12 E4's are marked with <, start soft and increase volume.
    auto v = linspace(0.1, 1.0, 12);
    for (std::size_t i = 0; i < 12; ++i) play(move_to(1, i + 1.0), note_e4 * v[i]);

    play(move_to(5, 1), note_a4_hd, note_a3_hd, note_e4);
    play(move_to(5, 2), note_e4);
    play(move_to(5, 3), note_e4);

    play(move_to(6, 1), note_b4_2hd, note_b3_2hd, note_e4);
    play(move_to(6, 2), note_e4);
    play(move_to(6, 3), note_e4);

    play(move_to(7, 1), note_e4);
    play(move_to(7, 2), note_e4);
    play(move_to(7, 3), note_e4);

    //-------------------------------------------------------------------------
    // Measures 8 - 16

    auto note_e5 = po.play(quarter, E5) * env;
    auto note_a4 = po.play(quarter, A4) * env;
    auto note_a3 = po.play(quarter, A3) * env;
    auto note_b4 = po.play(quarter, B4) * env;
    auto note_b3 = po.play(quarter, B3) * env;
    auto note_c5 = po.play(quarter, C5) * env;
    auto note_c4 = po.play(quarter, C4) * env;
    auto note_c3 = po.play(quarter, C3) * env;

    auto note_a4_h = po.play(half, A4) * env * 1.05;
    auto note_a3_h = po.play(half, A3) * env * 1.05;

    auto note_d5_hd = po.play(half_dot, D5) * env_hd * 1.10;
    auto note_d4_hd = po.play(half_dot, D4) * env_hd * 1.10;
    auto note_d3_hd = po.play(half_dot, D3) * env_hd * 1.10;
    auto note_c5_hd = po.play(half_dot, C5) * env_hd * 1.10;
    auto note_c4_hd = po.play(half_dot, C4) * env_hd * 1.10;
    auto note_c3_hd = po.play(half_dot, C3) * env_hd * 1.10;

    play(move_to(8, 1), note_a4,note_e4, note_a3);
    play(move_to(8, 2), note_b4,note_e4, note_b3);
    play(move_to(8, 3), note_c5,note_e4, note_c4);

    play(move_to(9, 1), note_b4, note_e4, note_b3);
    play(move_to(9, 2), note_a4, note_e4, note_a3);
    play(move_to(9, 3), note_b4, note_e4, note_b3);

    play(move_to(10, 1), note_c5_hd, note_e4, note_c4_hd);
    play(move_to(10, 2), note_e4);
    play(move_to(10, 3), note_e4);

    play(move_to(11, 1), note_b4_2hd, note_e4, note_b3_2hd);
    play(move_to(11, 2), note_e4);
    play(move_to(11, 3), note_e4);

    play(move_to(12, 1), note_e4);
    play(move_to(12, 2), note_e4);
    play(move_to(12, 3), note_e4);

    play(move_to(13, 1), note_a4_h, note_e4, note_a3_h);
    play(move_to(13, 2), note_e4);
    play(move_to(13, 3), note_e5, note_e4);

    play(move_to(14, 1), note_c5_hd, note_e4, note_c4_hd);
    play(move_to(14, 2), note_e4);
    play(move_to(14, 3), note_e4);

    play(move_to(15, 1), note_b4_2hd, note_e4, note_b3_2hd);
    play(move_to(15, 2), note_e4);
    play(move_to(15, 3), note_e4);

    play(move_to(16, 1), note_e4);
    play(move_to(16, 2), note_e4);
    play(move_to(16, 3), note_e4);


    //---------------------------------------------------------------------
    // Measures 17 - 25

    play(move_to(17, 1), note_a4, note_e4, note_a3);
    play(move_to(17, 2), note_e5, note_e4);
    play(move_to(17, 3), note_c5, note_e4, note_c4);

    play(move_to(18, 1), note_b4, note_e4, note_b3);
    play(move_to(18, 2), note_a4, note_e4, note_a3);
    play(move_to(18, 3), note_b4, note_e4, note_b3);

    play(move_to(19, 1), note_c5_hd, note_e4, note_c4_hd);
    play(move_to(19, 2), note_e4);
    play(move_to(19, 3), note_e4);

    play(move_to(20, 1), note_b4_2hd, note_e4, note_b3_2hd);
    play(move_to(20, 2), note_e4);
    play(move_to(20, 3), note_e4);

    play(move_to(21, 1), note_e4);
    play(move_to(21, 2), note_e4);
    play(move_to(21, 3), note_e4);

    play(move_to(22, 1), note_a4, note_e4, note_a3);
    play(move_to(22, 2), note_e5, note_e4);
    play(move_to(22, 3), note_c5, note_e4, note_c4);

    play(move_to(23, 1), note_b4, note_e4, note_b3);
    play(move_to(23, 2), note_a4, note_e4, note_a3);
    play(move_to(23, 3), note_b4, note_e4, note_b3);

    play(move_to(24, 1), note_c5_hd, note_e4, note_c4_hd);
    play(move_to(24, 2), note_e4);
    play(move_to(24, 3), note_e4);

    play(move_to(25, 1), note_d5_hd, note_e4, note_d4_hd);
    play(move_to(25, 2), note_e4);
    play(move_to(25, 3), note_e4);

    //-------------------------------------------------------------------------
    // Measures 26 - 31

    auto note_e5_hd = po.play(half_dot, E5) * env;

    play(move_to(26, 1), note_e5_hd, note_e4);
    play(move_to(26, 2), note_e4);
    play(move_to(26, 3), note_e4);

    auto note_g5_e = po.play(eighth, G5) * env * 0.75;
    auto note_e5_e = po.play(eighth, E5) * env * 0.75;
    auto note_d5_e = po.play(eighth, D5) * env * 0.75;
    auto note_c5_e = po.play(eighth, C5) * env * 0.75;
    auto note_b4_e = po.play(eighth, B4) * env * 0.75;
    auto note_a4_e = po.play(eighth, A4) * env * 0.75;
    auto note_g4_e = po.play(eighth, G4) * env * 0.75;

    play(move_to(27, 1, 1), note_a4_e, note_e4, note_a3_hd);
    play(move_to(27, 1, 2), note_b4_e);
    play(move_to(27, 1, 3), note_c5_e, note_e4);
    play(move_to(27, 1, 4), note_b4_e);
    play(move_to(27, 1, 5), note_a4_e, note_e4);
    play(move_to(27, 1, 6), note_c5_e);

    play(move_to(28, 1, 1), note_b4_e, note_e4, note_b3_2hd);
    play(move_to(28, 1, 2), note_a4_e);
    play(move_to(28, 1, 3), note_g4_e, note_e4);
    play(move_to(28, 1, 4), note_a4_e);
    play(move_to(28, 1, 5), note_b4_e, note_e4);
    play(move_to(28, 1, 6), note_g4_e);

    play(move_to(29, 1, 1), note_b4_e, note_e4);
    play(move_to(29, 1, 2), note_a4_e);
    play(move_to(29, 1, 3), note_g4_e, note_e4);
    play(move_to(29, 1, 4), note_a4_e);
    play(move_to(29, 1, 5), note_b4_e, note_e4);
    play(move_to(29, 1, 6), note_g4_e);

    play(move_to(30, 1, 1), note_a4_e, note_e4, note_a3);
    play(move_to(30, 1, 2), note_c5_e);
    play(move_to(30, 1, 3), note_b4_e, note_e4, note_b3);
    play(move_to(30, 1, 4), note_d5_e);
    play(move_to(30, 1, 5), note_c5_e, note_e4, note_c4);
    play(move_to(30, 1, 6), note_a4_e);

    play(move_to(31, 1, 1), note_b4_e, note_e4, note_b3);
    play(move_to(31, 1, 2), note_g4_e);
    play(move_to(31, 1, 3), note_a4_e, note_e4, note_a3);
    play(move_to(31, 1, 4), note_c5_e);
    play(move_to(31, 1, 5), note_b4_e, note_e4, note_b3);
    play(move_to(31, 1, 6), note_a4_e);

    //---------------------------------------------------------------------
    // Measures 32 - 36

    auto env_s = Asdr{0.2, 0.40, 0.80, 0.40};

    auto note_e5_s = po.play(sixth, E5) * env_s * 0.75;
    auto note_d5_s = po.play(sixth, D5) * env_s * 0.75;
    auto note_c5_s = po.play(sixth, C5) * env_s * 0.75;
    auto note_b4_s = po.play(sixth, B4) * env_s * 0.75;
    auto note_a4_s = po.play(sixth, A4) * env_s * 0.75;
    auto note_g4_s = po.play(sixth, G4) * env_s * 0.75;
    auto note_e4_s = po.play(sixth, E4) * env_s * 0.75;

    play(move_to(32, 1, 1), note_c5_e, note_e4, note_c4_hd);
    play(move_to(32, 1, 2), note_d5_e);
    play(move_to(32, 1, 3), note_c5_e, note_e4);
    play(move_to(32, 1, 4), note_b4_e);
    play(move_to(32, 1, 5), note_a4_e, note_e4);
    play(move_to(32, 1, 6), note_c5_e);

    play(move_to(33, 1, 1), note_b4_e, note_e4, note_b3_2hd);
    play(move_to(33, 1, 2), note_a4_e);
    play(move_to(33, 1, 3), note_g4_e, note_e4);
    play(move_to(33, 1, 4), note_a4_e);
    play(move_to(33, 1, 5), note_b4_e, note_e4);
    play(move_to(33, 1, 6), note_g4_e);

    play(move_to(34, 1, 1), note_b4_e, note_e4);
    play(move_to(34, 1, 2), note_a4_e);
    play(move_to(34, 1, 3), note_g4_e, note_e4);
    play(move_to(34, 1, 4), note_e5_e);
    play(move_to(34, 1, 5), note_b4_e, note_e4);
    play(move_to(34, 1, 6), note_g4_e);

    play(move_to(35, 1, 1,  1), note_a4_s, note_a3_hd);
    play(move_to(35, 1, 1,  2), note_e4_s);
    play(move_to(35, 1, 1,  3), note_b4_s);
    play(move_to(35, 1, 1,  4), note_e4_s);
    play(move_to(35, 1, 1,  5), note_c5_s);
    play(move_to(35, 1, 1,  6), note_e4_s);
    play(move_to(35, 1, 1,  7), note_b4_s);
    play(move_to(35, 1, 1,  8), note_e4_s);
    play(move_to(35, 1, 1,  9), note_a4_s);
    play(move_to(35, 1, 1, 10), note_e4_s);
    play(move_to(35, 1, 1, 11), note_c5_s);
    play(move_to(35, 1, 1, 12), note_e4_s);

    play(move_to(36, 1, 1,  1), note_b4_s, note_b3_2hd);
    play(move_to(36, 1, 1,  2), note_e4_s);
    play(move_to(36, 1, 1,  3), note_a4_s);
    play(move_to(36, 1, 1,  4), note_e4_s);
    play(move_to(36, 1, 1,  5), note_g4_s);
    play(move_to(36, 1, 1,  6), note_e4_s);
    play(move_to(36, 1, 1,  7), note_a4_s);
    play(move_to(36, 1, 1,  8), note_e4_s);
    play(move_to(36, 1, 1,  9), note_b4_s);
    play(move_to(36, 1, 1, 10), note_e4_s);
    play(move_to(36, 1, 1, 11), note_g4_s);
    play(move_to(36, 1, 1, 12), note_e4_s);

    //-------------------------------------------------------------------------
    // Measures 37 - 40

    play(move_to(37, 1, 1,  1), note_b4_s);
    play(move_to(37, 1, 1,  2), note_e4_s);
    play(move_to(37, 1, 1,  3), note_a4_s);
    play(move_to(37, 1, 1,  4), note_e4_s);
    play(move_to(37, 1, 1,  5), note_g4_s);
    play(move_to(37, 1, 1,  6), note_e4_s);
    play(move_to(37, 1, 1,  7), note_e5_s);
    play(move_to(37, 1, 1,  8), note_e4_s);
    play(move_to(37, 1, 1,  9), note_b4_s);
    play(move_to(37, 1, 1, 10), note_e4_s);
    play(move_to(37, 1, 1, 11), note_g4_s);
    play(move_to(37, 1, 1, 12), note_e4_s);

    play(move_to(38, 1, 1,  1), note_a4_s, note_a3);
    play(move_to(38, 1, 1,  2), note_e4_s);
    play(move_to(38, 1, 1,  3), note_c5_s);
    play(move_to(38, 1, 1,  4), note_e4_s);
    play(move_to(38, 1, 1,  5), note_b4_s, note_b3);
    play(move_to(38, 1, 1,  6), note_e4_s);
    play(move_to(38, 1, 1,  7), note_d5_s);
    play(move_to(38, 1, 1,  8), note_e4_s);
    play(move_to(38, 1, 1,  9), note_c5_s, note_c4);
    play(move_to(38, 1, 1, 10), note_e4_s);
    play(move_to(38, 1, 1, 11), note_a4_s);
    play(move_to(38, 1, 1, 12), note_e4_s);

    play(move_to(39, 1, 1,  1), note_b4_s, note_b3);
    play(move_to(39, 1, 1,  2), note_e4_s);
    play(move_to(39, 1, 1,  3), note_g4_s);
    play(move_to(39, 1, 1,  4), note_e4_s);
    play(move_to(39, 1, 1,  5), note_a4_s, note_a3);
    play(move_to(39, 1, 1,  6), note_e4_s);
    play(move_to(39, 1, 1,  7), note_c5_s);
    play(move_to(39, 1, 1,  8), note_e4_s);
    play(move_to(39, 1, 1,  9), note_b4_s, note_b3);
    play(move_to(39, 1, 1, 10), note_e4_s);
    play(move_to(39, 1, 1, 11), note_d5_s);
    play(move_to(39, 1, 1, 12), note_e4_s);

    play(move_to(40, 1, 1,  1), note_c5_s, note_c4_hd);
    play(move_to(40, 1, 1,  2), note_e4_s);
    play(move_to(40, 1, 1,  3), note_d5_s);
    play(move_to(40, 1, 1,  4), note_e4_s);
    play(move_to(40, 1, 1,  5), note_c5_s);
    play(move_to(40, 1, 1,  6), note_e4_s);
    play(move_to(40, 1, 1,  7), note_b4_s);
    play(move_to(40, 1, 1,  8), note_e4_s);
    play(move_to(40, 1, 1,  9), note_a4_s);
    play(move_to(40, 1, 1, 10), note_e4_s);
    play(move_to(40, 1, 1, 11), note_c5_s);
    play(move_to(40, 1, 1, 12), note_e4_s);

    //-------------------------------------------------------------------------
    // Finish up.

    auto dt = Toc();

    cout << "\nRendering took: " << dt
         << "\nFinishing up." << std::endl;

    auto t0 = move_to(26);
    auto t1 = move_to(41);

    out = out.substream(t0, t1 - t0);


    //-------------------------------------------------------------------------
    // Pad with silence, filter, save, play.

    out = gen.silence(1.0) << out << gen.silence(2.0);

    out = filter.filter(out);
    out = reverb.filter(out);

    out.normalize();
    out *= 0.666;

    out >> "interstellar-theme.wav";

    AudioPlayback pb(sr, 2, 16);
    out >> pb;

    return 0;
}

