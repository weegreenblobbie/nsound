//-----------------------------------------------------------------------------
//
//  $Id: example1.cc 874 2014-09-08 02:21:29Z weegreenblobbie $
//
//-----------------------------------------------------------------------------

#include <Nsound/NsoundAll.h>

#include <algorithm>
#include <atomic>
#include <csignal>
#include <iostream>
#include <iomanip>
#include <ios>
#include <fstream>
#include <future>
#include <map>

using std::cout;
using std::endl;
using std::scientific;

using namespace Nsound;

const std::map<std::string, float64> notes = {
    {"C1",   32.70320},
    {"C1#",  34.64783},
    {"D1",   36.70810},
    {"D1#",  38.89087},
    {"E1",   41.20344},
    {"F1",   43.65353},
    {"F1#",  46.24930},
    {"G1",   48.99943},
    {"G1#",  51.91309},
    {"A1",   55.00000},
    {"A1#",  58.27047},
    {"B1",   61.73541},
    {"C2",   65.40639},
    {"C2#",  69.29566},
    {"D2",   73.41619},
    {"D2#",  77.78175},
    {"E2",   82.40689},
    {"F2",   87.30706},
    {"F2#",  92.49861},
    {"G2",   97.99886},
    {"G2#", 103.8262 },
    {"A2",  110.0000 },
    {"A2#", 116.5409 },
    {"B2",  123.4708 },
    {"C3",  130.8128 },
    {"C3#", 138.5913 },
    {"D3",  146.8324 },
    {"D3#", 155.5635 },
    {"E3",  164.8138 },
    {"F3",  174.6141 },
    {"F3#", 184.9972 },
    {"G3",  195.9977 },
    {"G3#", 207.6523 },
    {"A3",  220.0000 },
    {"A3#", 233.0819 },
    {"B3",  246.9417 },
    {"C4",  261.6256 },
    {"C4#", 277.1826 },
    {"D4",  293.6648 },
    {"D4#", 311.1270 },
    {"E4",  329.6276 },
    {"F4",  349.2282 },
    {"F4#", 369.9944 },
    {"G4",  391.9954 },
    {"G4#", 415.3047 },
    {"A4",  440.0000 },
    {"A4#", 466.1638 },
    {"B4",  493.8833 },
    {"C5",  523.2511 },
    {"C5#", 554.3653 },
    {"D5",  587.3295 },
    {"D5#", 622.2540 },
    {"E5",  659.2551 },
    {"F5",  698.4565 },
    {"F5#", 739.9888 },
    {"G5",  783.9909 },
    {"G5#", 830.6094 },
    {"A5",  880.0000 },
    {"A5#", 932.3275 },
    {"B5",  987.7666 },
    {"C6",  1046.502 },
    {"C6#", 1108.731 },
    {"D6",  1174.659 },
    {"D6#", 1244.508 },
    {"E6",  1318.510 },
    {"F6",  1396.913 },
    {"F6#", 1479.978 },
    {"G6",  1567.982 },
    {"G6#", 1661.219 },
    {"A6",  1760.000 },
    {"A6#", 1864.655 },
    {"B6",  1975.533 },
    {"C7",  2093.005 },
    {"C7#", 2217.461 },
    {"D7",  2349.318 },
    {"D7#", 2489.016 },
    {"E7",  2637.020 },
    {"F7",  2793.826 },
    {"F7#", 2959.955 },
    {"G7",  3135.963 },
    {"G7#", 3322.438 },
    {"A7",  3520.000 },
    {"A7#", 3729.310 },
    {"B7",  3951.066 },
    {"C8",  4186.009 },
    {"C8#", 4434.922 },
};

namespace
{
  volatile std::atomic_bool g_signal {false};
}

void signal_handler(int signal)
{
    g_signal = signal == SIGINT;
}

struct Organ
{
    static Organ from_txt(const std::string & filename, float64 freq_, std::size_t num_harmonics_)
    {
        std::ifstream fin(filename.c_str());

        if (not fin.good())
        {
            return Organ(48000.0, freq_, num_harmonics_);
        }

        std::string key;

        fin >> key;
        assert (key == "sample_rate");
        float64 sample_rate {0.0};
        fin >> sample_rate;

        fin >> key;
        assert (key == "frequency");
        float64 freq {0.0};
        fin >> freq;

        fin >> key;
        assert (key == "num_harmonics");

        std::size_t num_harmonics {0};
        fin >> num_harmonics;

        fin >> key;
        assert (key == "phases");

        std::vector<float64> phases;

        for (std::size_t i = 0; i < num_harmonics; ++i)
        {
            float64 p {0.0};
            fin >> p;
            phases.push_back(p);
        }

        fin >> key;
        assert (key == "weights");

        std::vector<float64> weights;

        for (std::size_t i = 0; i < num_harmonics; ++i)
        {
            float64 w {0.0};
            fin >> w;
            weights.push_back(w);
        }

        assert (sample_rate > 0.0);
        assert (num_harmonics > 0);
        assert (phases.size() > 0);
        assert (weights.size() > 0);
        assert (phases.size() == weights.size());

        auto organ = Organ(sample_rate, freq, weights.size());

        organ.phases = phases;
        organ.weights = weights;

        return organ;
    }

    void to_txt(const std::string & filename)
    {
        std::ofstream fout(filename.c_str());

        fout << "sample_rate " << sin.getSampleRate() << "\n";
        fout << "frequency " << freq << "\n";
        fout << "num_harmonics " << phases.size() << "\n";
        fout << "phases ";
        for (auto & p : phases)
        {
            fout << p << " ";
        }
        fout << "\n";
        fout << "weights ";
        const auto max_w = *std::max_element(weights.begin(), weights.end());
        for (auto & w : weights)
        {
            fout << (w / max_w) << " ";
        }
        fout << "\n";

        fout.close();

        cout << "Wrote " << filename << endl;
    }

    Organ(float64 sample_rate, float64 frequency, std::size_t num_harmonics)
        :
        sin(sample_rate),
        freq(frequency),
        phases(num_harmonics, 0.0),
        harmonics(num_harmonics, 0.0),
        weights(num_harmonics, 0.0)
    {
        for (std::size_t i = 0; i < num_harmonics; ++i)
        {
            harmonics[i] = (i + 1.0);
        }
        weights[0] = 1.0;
    }

    Organ(const Organ & copy)
        :
        sin(copy.sin.getSampleRate()),
        freq(copy.freq),
        phases(copy.phases),
        harmonics(copy.harmonics),
        weights(copy.weights)
    {}

    Organ & operator=(const Organ & rhs)
    {
        if (this == &rhs) return *this;

        freq = rhs.freq;
        phases = rhs.phases;
        harmonics = rhs.harmonics;
        weights = rhs.weights;

        return *this;
    }

    Buffer generate(float64 duration)
    {
        Buffer out = sin.silence(duration);

        for (std::size_t i = 0; i < harmonics.size(); ++i)
        {
            auto phase = phases[i];
            auto f = freq * harmonics[i];
            auto w = weights[i];

            sin.reset();

            if (w > 0.0)
            {
                out += sin.generate2(duration, f, phase) * w;
            }
        }

        out.normalize();

        return out * 0.80;
    }

    // Freq domain signal
    float64 score(const Buffer & reference)
    {
        float64 duration = 2 * reference.getLength() / sin.getSampleRate();

        auto signal = generate(duration);

        Spectrogram spec(signal, sin.getSampleRate(), 0.060, 0.010, HANNING);

        auto mag = spec.getMagnitude()[0];

        mag.normalize();

        // Compute root mean squared error.
        return std::sqrt(((reference - mag) ^ 2.0).getMean());
    }

    void mutate()
    {
        const std::size_t _1 = 1;

        float64 rnd = sin.whiteNoise(_1, 0.0, 1.0)[0];

        std::size_t num_harmonics = 1;

        if (rnd > 0.5) num_harmonics = 2;
        if (rnd > 0.75) num_harmonics = 3;
        if (rnd > 0.875) num_harmonics = 4;
        if (rnd > 0.9375) num_harmonics = 5;
        if (rnd > 0.96875) num_harmonics = 6;
        if (rnd > 0.984375) num_harmonics = 7;
        if (rnd > 0.9921875) num_harmonics = 8;
        if (rnd > 0.99609375) num_harmonics = 9;
        if (rnd > 0.998046875) num_harmonics = 10;

        for (std::size_t i = 0; i < num_harmonics; ++i)
        {
            auto index  = static_cast<std::size_t>(sin.whiteNoise(_1, 0.0, harmonics.size() - 1.0)[0] + 0.5);

            auto & phase = phases[index];
            auto & weight = weights[index];
//~            auto & harmonic = harmonics[index];

            freq += sin.gaussianNoise(_1, 0.0,  0.10)[0];
//~            harmonic += sin.gaussianNoise(_1, 0.0,  0.10)[0];
            phase += sin.gaussianNoise(_1, 0.0, 0.50)[0];
            weight += sin.gaussianNoise(_1, 0.0, 0.50)[0];

            if (phase < 0.0) phase = 0.0;
            if (phase > 1.0) phase = 1.0;
            if (weight < 1e-2) { weight = 0.0; phase = 0.0; }
            if (weight > 1.0) weight = 1.0;
        }
    }

    void print() const
    {
        cout << std::defaultfloat;
        cout << "freq = " << freq << "\n";
        cout << "phases = ";
        for (std::size_t i = 0; i < phases.size(); ++i)
        {
            cout << phases[i] << " ";
        }
        cout << "\n";

        cout << "weights = ";
        const auto max_w = *std::max_element(weights.begin(), weights.end());
        for (const auto & w : weights)
        {
            cout << (w / max_w) << " ";
        }
        cout << "\n";
    }

    Sine                 sin;
    float64              freq;
    std::vector<float64> phases;
    std::vector<float64> harmonics;
    std::vector<float64> weights;

};


int
main(int argc, char ** argv)
{
    if (argc != 3)
    {
        cout << "usage: example1 [NOTE] [WAVE REFERENCE]\n";
        return 1;
    }

    auto arg1 = std::string(argv[1]);
    auto arg2 = std::string(argv[2]);

    if (notes.count(arg2) == 0)
    {
        cout << "Could not find note " << arg2 << "\n";
        return 1;
    }

    const auto FREQ = notes.find(arg2)->second;

    std::signal(SIGINT, signal_handler);

    const auto params_txt = "organ-" + arg2 + ".txt";

    auto organ = Organ::from_txt(params_txt, FREQ, 23);

    const auto sr = organ.sin.getSampleRate();

    //-------------------------------------------------------------------------
    // Reference
    const auto reference = AudioStream(arg1  );

    assert (std::abs(reference.getSampleRate() - sr) < 1.0);

    const auto middle = reference.getDuration() / 2.0 - 0.10;
    const auto ref = reference.substream(middle, 0.20)[0].getNormalize();

    Spectrogram spec(ref, sr, 0.060, 0.010, HANNING);

    const auto freq_axis = spec.getFrequencyAxis();
    auto mag = spec.getMagnitude()[0];
    mag.normalize();
    const auto spec_ref = mag;

    auto score_0 = organ.score(spec_ref);

    Tic();

    cout << std::setw(4) << 0 << " score = " << std::setw(8) << std::fixed << score_0 << std::endl;

    auto child = Organ(organ);

    struct Parameters
    {
        Organ organ;
        float64 score;
    };

    auto params = Parameters{organ, score_0};

    auto worker = [&](const Parameters & par) {
        auto organ = par.organ;
        auto child = par.organ;
        auto score_0 = par.score;
        for (int i = 0; i < 250; ++i)
        {
            if (g_signal) break;
            child.mutate();
            auto score = child.score(spec_ref);

            if (score < score_0)
            {
                organ = child;
                score_0 = score;
            }
            else
            {
                child = organ;
            }
        }

        return Parameters{organ, score_0};
    };

    for (int i = 1; i <= 50'000; ++i)
    {
        if (g_signal) break;

        auto futures = std::vector<std::shared_future<Parameters>>{
            std::async(worker, params),
            std::async(worker, params),
            std::async(worker, params),
            std::async(worker, params),
        };

        bool once = true;

        for (auto & result : futures)
        {
            result.wait();

            auto res = result.get();

            if (res.score < score_0)
            {
                if (once)  // Once per thread.
                {
                    auto delta = score_0 - res.score;
                    cout << std::setw(4) << i
                         << " score = " << std::setw(8) << std::fixed << res.score
                         << ", diff = " << std::setw(8) << std::fixed << delta
                         << std::endl;
                    score_0 = res.score;
                    once = false;
                }
                params.organ = res.organ;
            }
        }
    }
    organ = params.organ;

    float64 runtime = Toc();

    cout << "\nruntime = " << runtime << endl;

    organ.print();
    organ.to_txt(params_txt);

    AudioStream aout(sr, 1);

    aout << organ.generate(3.0) * 0.33;

    aout >> "organ.wav";

    auto reverb = ReverberationRoom(sr, 1.00, 0.75, 0.50, 25.0, 0.10);

    auto aout2 = reverb.filter(aout);

    aout2 >> "organ-reverb.wav";

    // FFT.

    auto out = organ.generate(3600.0/sr);

    Spectrogram spec2(out, sr, 0.060, 0.010, HANNING);
    auto mag2 = spec2.getMagnitude()[0];
    mag2.normalize();

    Plotter plt;

    auto x0 = 0.0;
    auto x1 = std::min(1200.0, 3.5 * sr / FREQ);

    plt.figure();
    plt.plot(ref);
    plt.plot(out);
    plt.title("real vs synth");
    plt.xlim(x0, x1);

    x0 = 0.0;
    x1 = std::max(2000.0, 6.5 * FREQ);

    plt.figure("figsize=(16, 9)");
    plt.plot(freq_axis, mag);
    plt.plot(freq_axis, mag2);
    plt.xlim(x0, x1);
    plt.xlabel("freq HZ");
    plt.title("real vs synth");

    plt.show();

    return 0;
}
