#include <iostream>
#include <Nsound/NsoundAll.h>

using namespace Nsound;
using std::cout;

struct Asdr
{
    float64 _attack;
    float64 _delay;
    float64 _release;
    float64 _sustain_amp;
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


int main(void)
{
    auto sr = 48000.0;

    auto out = AudioStream(sr, 2);
    auto gen = Generator(sr);
    auto po = OrganPipe(sr);
    auto env = Asdr{0.5, 0.1, 0.4, 0.9};
    auto note_a = po.play(0.600,  880.0) * env;
    auto note_b = po.play(0.700, 1320.0) * env;
    auto note_c = po.play(0.600,  990.0) * env;
    auto note_d = po.play(0.600, 1050.0) * env;
    auto note_e = po.play(0.600, 1175.0) * env;
    auto filter = FilterLowPassIIR(sr, 3, 2500.0, 0.05);
    auto reverb = ReverberationRoom(sr, 0.750, 1.00, 0.50, 2500.0, 0.1000);

    auto pos = 0.0;

    for (int i = 0; i < 2; ++i)
    {
        cout << "loop " << (i+1) << " pos = " << pos << "\n";
        out.add(note_a, pos); pos += 0.650;
        out.add(note_b, pos); pos += 1.050;

        out.add(note_a, pos); pos += 0.650;
        out.add(note_b, pos); pos += 1.050;

        out.add(note_c, pos); pos += 0.650;
        out.add(note_b, pos); pos += 1.050;

        out.add(note_c, pos); pos += 0.650;
        out.add(note_b, pos); pos += 1.050;

        out.add(note_a, pos); pos += 0.650;
        out.add(note_b, pos); pos += 1.050;

        out.add(note_a, pos); pos += 0.650;
        out.add(note_b, pos); pos += 1.050;

        out.add(note_d, pos); pos += 0.650;
        out.add(note_b, pos); pos += 1.050;

        out.add(note_d, pos); pos += 0.650;
        out.add(note_b, pos); pos += 1.050;

        out.add(note_c, pos); pos += 0.650;
        out.add(note_b, pos); pos += 1.050;

        out.add(note_c, pos); pos += 0.650;
        out.add(note_b, pos); pos += 1.050;

        out.add(note_a, pos); pos += 0.650;
        out.add(note_b, pos); pos += 1.050;

        out.add(note_a, pos); pos += 0.650;
        out.add(note_b, pos); pos += 1.050;

        out.add(note_c, pos); pos += 0.650;
        out.add(note_b, pos); pos += 1.050;

        out.add(note_c, pos); pos += 0.650;
        out.add(note_b, pos); pos += 1.050;

        out.add(note_d, pos); pos += 0.650;
        out.add(note_b, pos); pos += 1.050;

        out.add(note_d, pos); pos += 0.650;
        out.add(note_b, pos); pos += 1.050;

        out.add(note_e, pos); pos += 0.650;
        out.add(note_b, pos); pos += 1.050;

        out.add(note_e, pos); pos += 0.650;
        out.add(note_b, pos); pos += 1.050;

        out.add(note_c, pos); pos += 0.650;
        out.add(note_b, pos); pos += 1.050;

        out.add(note_a, pos); pos += 0.650;
        out.add(note_b, pos); pos += 1.050;

        out.add(note_c, pos); pos += 0.650;
        out.add(note_b, pos); pos += 1.050;

        out.add(note_a, pos); pos += 0.650;
        out.add(note_b, pos); pos += 1.050;

        out.add(note_c, pos); pos += 0.650;
        out.add(note_b, pos); pos += 1.050;

        out.add(note_d, pos); pos += 0.650;
        out.add(note_b, pos); pos += 1.050;

        out.add(note_c, pos); pos += 0.650;
        out.add(note_b, pos); pos += 1.050;

        out.add(note_d, pos); pos += 0.650;
        out.add(note_b, pos); pos += 1.050;

        out.add(note_e, pos); pos += 0.650;
        out.add(note_b, pos); pos += 1.050;

        out.add(note_d, pos); pos += 0.650;
        out.add(note_b, pos); pos += 1.050;

        out.add(note_c, pos); pos += 0.650;
        out.add(note_b, pos); pos += 1.050;

        out.add(note_d, pos); pos += 0.650/2.0;
        out.add(note_c, pos); pos += 0.650/2.0;
    }

    out.add(note_a, pos); pos += 0.650;
    out.add(note_b, pos); pos += 1.050;

    out = gen.silence(0.5) << out << gen.silence(0.5);

    out = filter.filter(out);
    out = reverb.filter(out);

    out * 0.5 >> "interstellar-theme.wav";

    return 0;
}

