//-----------------------------------------------------------------------------
//
//  Generator tests, compared with the gold files.
//
//-----------------------------------------------------------------------------

#include <Nsound/Buffer.h>
#include <Nsound/Cosine.h>
#include <Nsound/Generator.h>
#include <Nsound/Wavefile.h>
#include <Nsound/WindowType.h>

#include "gold_check.h"

#include <catch2/catch_test_macros.hpp>

using namespace Nsound;

static const float64 GAMMA = 8e-7;

TEST_CASE("Generator drawLine()")
{
    Wavefile::setDefaultSampleSize(32);
    Wavefile::setIEEEFloat(true);

    Generator gen(100);

    SECTION("drawLine(1.0, 1.0, 0.0)")
    {
        check_gold(gen.drawLine(1.0, 1.0, 0.0), "gold/Generator_line1.wav", GAMMA);
    }

    SECTION("drawLine(1.26, -0.26, 0.26)")
    {
        check_gold(gen.drawLine(1.26, -0.26, 0.26), "gold/Generator_line2.wav", GAMMA);
    }
}

TEST_CASE("Generator buzz()")
{
    Wavefile::setDefaultSampleSize(32);
    Wavefile::setIEEEFloat(true);

    Cosine cos(100);

    SECTION("buzz(1.0, 4.0, 3, 0.0)")
    {
        check_gold(cos.buzz(1.0, 4.0, 3, 0.0), "gold/Generator_buzz1.wav", GAMMA);
    }

    SECTION("buzz(1.0, 4.0, 3, 0.5)")
    {
        check_gold(cos.buzz(1.0, 4.0, 3, 0.5), "gold/Generator_buzz2.wav", GAMMA);
    }

    SECTION("buzz(1.0, 4.0, 4, 0.5)")
    {
        check_gold(cos.buzz(1.0, 4.0, 4, 0.5), "gold/Generator_buzz3.wav", GAMMA);
    }
}

TEST_CASE("Generator drawSine() and drawSine2()")
{
    Wavefile::setDefaultSampleSize(32);
    Wavefile::setIEEEFloat(true);

    Generator gen(600.0);

    Buffer phase = gen.drawLine(1.0, 0.0, 1.0);

    SECTION("drawSine(1.0, 10.0)")
    {
        check_gold(gen.drawSine(1.0, 10.0), "gold/Generator_sine1.wav", GAMMA);
    }

    SECTION("drawSine(1.0, freqs)")
    {
        Buffer freqs = gen.drawLine(1.0, 1.0, 10.0);

        check_gold(gen.drawSine(1.0, freqs), "gold/Generator_sine2.wav", GAMMA);
    }

    SECTION("drawSine2(1.0, 3.0, 0.5)")
    {
        check_gold(gen.drawSine2(1.0, 3.0, 0.5), "gold/Generator_sine3.wav", GAMMA);
    }

    SECTION("drawSine2(1.0, 3.0, phase)")
    {
        check_gold(gen.drawSine2(1.0, 3.0, phase), "gold/Generator_sine4.wav", GAMMA);
    }

    SECTION("drawSine2(1.0, freqs, 0.5)")
    {
        Buffer freqs = gen.drawLine(1.0, 0.0, 10.0);

        check_gold(gen.drawSine2(1.0, freqs, 0.5), "gold/Generator_sine5.wav", GAMMA);
    }

    SECTION("drawSine2(1.0, freqs, phase)")
    {
        Buffer freqs = gen.drawLine(1.0, 0.0, 10.0);

        check_gold(gen.drawSine2(1.0, freqs, phase), "gold/Generator_sine6.wav", GAMMA);
    }
}

TEST_CASE("Generator generate() with chorus")
{
    Wavefile::setDefaultSampleSize(32);
    Wavefile::setIEEEFloat(true);

    Cosine cos(100);

    cos.setSeed(457745);
    cos.setChorus(2, 0.50);

    check_gold(cos.generate(1.0, 5), "gold/Generator_chorus1.wav", GAMMA);
}

//! Reaches the protected default constructor, as subclasses do.
struct DefaultGenerator : public Generator
{
    DefaultGenerator() : Generator() {}
};

//! One cycle of a ramp, 0 to 7 / 8, for a Generator at 8 Hz.
static
Buffer
make_ramp_waveform()
{
    return Buffer({0.0, 0.125, 0.25, 0.375, 0.5, 0.625, 0.75, 0.875});
}

TEST_CASE("Generator constructors")
{
    SECTION("the default constructor makes an empty Generator")
    {
        CHECK_NOTHROW(DefaultGenerator());
    }

    SECTION("a waveform plays back at 1 Hz, sample by sample")
    {
        Generator gen(8, make_ramp_waveform());

        check_close(gen.generate(1.0, 1.0), make_ramp_waveform(), 0.0);

        // At 2 Hz every other sample, twice.
        check_close(
            gen.generate(1.0, 2.0),
            Buffer({0.0, 0.25, 0.5, 0.75, 0.0, 0.25, 0.5, 0.75}),
            0.0);
    }

    SECTION("a waveform must be one sample rate long")
    {
        CHECK_THROWS_AS(Generator(10, make_ramp_waveform()), Exception);
    }

    SECTION("a waveform from a wave file, its length is the sample rate")
    {
        Wavefile::setDefaultSampleSize(64);
        Wavefile::setIEEEFloat(true);

        make_ramp_waveform() >> "test_waveform.wav";

        Generator gen(std::string("test_waveform.wav"));

        check_close(gen.generate(1.0, 1.0), make_ramp_waveform(), 0.0);
    }
}

TEST_CASE("Generator copies")
{
    Generator gen(8, make_ramp_waveform());
    gen.setSeed(42);

    SECTION("the copy constructor copies the random state")
    {
        Generator copy(gen);

        check_close(copy.whiteNoise(1.0), gen.whiteNoise(1.0), 0.0);
        check_close(copy.generate(1.0, 1.0), make_ramp_waveform(), 0.0);
    }

    SECTION("operator= copies the random state and the waveform")
    {
        Generator other(4);
        other.setSeed(7);

        other = gen;

        check_close(other.whiteNoise(1.0), gen.whiteNoise(1.0), 0.0);
        check_close(other.generate(1.0, 1.0), make_ramp_waveform(), 0.0);

        // Onto a Generator that has a waveform, then one that has none.
        Generator with_waveform(8, Buffer::zeros(8));
        with_waveform = gen;
        check_close(with_waveform.generate(1.0, 1.0), make_ramp_waveform(), 0.0);

        with_waveform = Generator(8);
        CHECK_THROWS_AS(with_waveform.generate(1.0, 1.0), Exception);

        Generator & same = gen;
        gen = same;
        check_close(gen.generate(1.0, 1.0), make_ramp_waveform(), 0.0);
    }

    SECTION("operator= onto and from a default constructed Generator")
    {
        DefaultGenerator empty;
        static_cast<Generator &>(empty) = gen;

        check_close(empty.generate(1.0, 1.0), make_ramp_waveform(), 0.0);

        Generator target(8);
        target = DefaultGenerator();

        // It has the default's sample rate of 0, so lines have no samples.
        CHECK(target.silence(0.5).getLength() == 0);
    }
}

TEST_CASE("Generator slave sync")
{
    // The master at 3 Hz passes the sample rate on its 3rd sample, so the
    // slave starts its waveform over on its 3rd sample.
    Generator master(8, make_ramp_waveform());
    Generator slave(8, make_ramp_waveform());

    master.addSlaveSync(slave);
    master.addSlaveSync(slave);

    Buffer out;

    for(uint32 i = 0; i < 4; ++i)
    {
        master.generate(3.0);
        out << slave.generate(1.0);
    }

    check_close(out, Buffer({0.0, 0.125, 0.0, 0.125}), 0.0);

    SECTION("removeSlaveSync() stops it")
    {
        master.removeSlaveSync(slave);
        master.removeSlaveSync(slave);

        Generator plain(8, make_ramp_waveform());

        Buffer synced;
        Buffer expected;

        for(uint32 i = 0; i < 4; ++i)
        {
            master.generate(3.0);
            synced << slave.generate(1.0);
            expected << plain.generate(1.0);
        }

        // The slave goes on from where it was, without starting over.
        check_close(synced, Buffer({0.25, 0.375, 0.5, 0.625}), 0.0);
        check_close(expected, Buffer({0.0, 0.125, 0.25, 0.375}), 0.0);
    }
}

TEST_CASE("Generator tell()")
{
    Generator gen(8, make_ramp_waveform());

    gen.generate(2.0);
    gen.generate(2.0);
    gen.generate(2.0);

    // 6 samples into 8.
    CHECK(gen.tell() == 0.75);

    // The position stays within the waveform, however long it plays: 11
    // samples into 8 is 3, then 14 back from there is 5.
    gen.generate(5.0);

    CHECK(gen.tell() == 0.375);

    gen.generate(-14.0);

    CHECK(gen.tell() == 0.625);
}

TEST_CASE("Generator plays on after a long run")
{
    Generator gen(8, make_ramp_waveform());

    for(uint32 i = 0; i < 1000001; ++i) gen.generate(3.0);

    // 3000003 samples into 8 is 3.
    CHECK(gen.tell() == 0.375);
    CHECK(gen.generate(3.0) == 0.375);
}

//! Checks that realtime generate2() in chunks of uneven lengths matches one
//! call, with frequency and phase both ramping through negative values.
static
void
check_chunks_match_one_call(Generator & whole, Generator & chunked)
{
    // A power of 2, so a chunk's duration gives its exact number of samples.
    const uint32 sample_rate = 1024;

    Buffer frequencies = whole.drawLine(3.0, -300.0, 500.0);
    Buffer phases = whole.drawLine(3.0, -1.5, 2.5);

    whole.setRealtime(true);
    chunked.setRealtime(true);

    Buffer expected = whole.generate2(3.0, frequencies, phases);
    Buffer out;

    const uint32 lengths[] = {1, 7, 64, 13, 500};

    for(uint32 start = 0, i = 0; start < expected.getLength(); ++i)
    {
        uint32 length = std::min(
            lengths[i % 5], expected.getLength() - start);

        out << chunked.generate2(
            static_cast<float64>(length) / sample_rate,
            frequencies.subbuffer(start, length),
            phases.subbuffer(start, length));

        start += length;
    }

    REQUIRE(expected.getAbs().getMax() > 0.5);

    check_close(out, expected, 0.0);
    CHECK(chunked.tell() == whole.tell());
}

TEST_CASE("Generator realtime chunks match one call")
{
    Cosine whole(1024);
    Cosine chunked(1024);

    SECTION("one voice")
    {
        check_chunks_match_one_call(whole, chunked);
    }

    SECTION("chorus")
    {
        whole.setSeed(457745);
        whole.setChorus(3, 0.25);

        chunked.setSeed(457745);
        chunked.setChorus(3, 0.25);

        check_chunks_match_one_call(whole, chunked);
    }
}

TEST_CASE("Generator negative frequency and phase")
{
    Generator gen(8, make_ramp_waveform());

    SECTION("a negative frequency plays the waveform backwards")
    {
        check_close(
            gen.generate(1.0, -1.0),
            Buffer({0.0, 0.875, 0.75, 0.625, 0.5, 0.375, 0.25, 0.125}),
            0.0);
    }

    SECTION("a negative phase starts earlier in the waveform")
    {
        // A phase of -0.5 is a quarter of the waveform, 2 samples.
        check_close(
            gen.generate2(1.0, 1.0, -0.5),
            Buffer({0.75, 0.875, 0.0, 0.125, 0.25, 0.375, 0.5, 0.625}),
            0.0);
    }

    SECTION("a chorus plays backwards too")
    {
        // One voice, not detuned.  The chorus reads a step ahead.
        gen.setChorus(1, 0.0);

        check_close(
            gen.generate(1.0, -1.0),
            Buffer({0.875, 0.75, 0.625, 0.5, 0.375, 0.25, 0.125, 0.0}),
            0.0);
    }
}

TEST_CASE("Generator buzz() with Buffers matches buzz() with numbers")
{
    Cosine cos(100);

    Buffer constant = cos.buzz(1.0, Buffer({4.0}), Buffer({3.0}), Buffer({0.5}));

    check_close(constant, Cosine(100).buzz(1.0, 4.0, 3, 0.5), 0.0);
}

TEST_CASE("Generator draw functions")
{
    // drawLine() makes duration * sample_rate samples from y1 up to, but not
    // including, y2.
    Generator gen(4);

    SECTION("drawDecay() is exp(-alpha t) for t from 0 to 1")
    {
        check_close(
            gen.drawDecay(1.0, 2.0),
            Buffer({1.0, 0.6065306597126334, 0.36787944117144233, 0.22313016014842982}),
            1e-15);

        CHECK(gen.drawDecay(0.0, 2.0).getLength() == 0);
    }

    SECTION("drawGaussian()")
    {
        Generator gen10(10);

        Buffer g = gen10.drawGaussian(1.0, 0.5, 0.1, false);

        // The peak at t = mu is 1 / sqrt(2 pi sigma^2), and it is symmetric.
        CHECK(std::fabs(g[5] - 3.989422804014327) < 1e-12);
        CHECK(std::fabs(g[4] - g[6]) < 1e-12);
        CHECK(g.argmax() == 5);

        CHECK(std::fabs(gen10.drawGaussian(1.0, 0.5, 0.1, true).getMax() - 1.0) < 1e-15);

        CHECK(gen10.drawGaussian(0.0, 0.5, 0.1).getLength() == 0);
    }

    SECTION("drawFatGaussian() is flat in the pass band")
    {
        Generator gen100(100);

        // The middle half is the pass band.
        Buffer g = gen100.drawFatGaussian(1.0, 0.5);

        REQUIRE(g.getLength() == 100);

        check_close(g.subbuffer(25, 50), Buffer::ones(50), 0.0);
        CHECK(g[0] < 0.1);
        CHECK(g[99] < 0.1);
    }

    SECTION("drawParabola() goes through (0, y1) and (x2, y2)")
    {
        Generator gen10(10);

        Buffer p = gen10.drawParabola(1.0, 1.0, 0.5, 3.0, 2.0);

        REQUIRE(p.getLength() == 10);

        CHECK(std::fabs(p[0] - 1.0) < 1e-12);
        CHECK(std::fabs(p[5] - 3.0) < 1e-12);
    }

    SECTION("drawSine(frequency) draws one sample")
    {
        Buffer y;

        for(uint32 i = 0; i < 4; ++i)
        {
            y << gen.drawSine(1.0);
        }

        // sin(2 pi t / 4) for t = 0, 1, 2, 3.
        check_close(y, Buffer({0.0, 1.0, 0.0, -1.0}), 1e-15);
    }

    SECTION("tanh() goes from tanh(-pi) towards tanh(pi)")
    {
        check_close(
            gen.tanh(1.0),
            Buffer({-0.99627207622075, -0.9171523356672744, 0.0, 0.9171523356672744}),
            1e-14);
    }
}

TEST_CASE("Generator windows")
{
    Generator gen(100);

    SECTION("known values")
    {
        Buffer hanning = gen.drawWindow(1.0, HANNING);
        CHECK(std::fabs(hanning[0]) < 1e-15);
        CHECK(std::fabs(hanning[50] - 1.0) < 1e-15);

        Buffer hamming = gen.drawWindow(1.0, HAMMING);
        CHECK(std::fabs(hamming[0] - 0.08) < 1e-15);
        CHECK(std::fabs(hamming[50] - 1.0) < 1e-15);

        Buffer blackman = gen.drawWindow(1.0, BLACKMAN);
        CHECK(std::fabs(blackman[0]) < 1e-15);
        CHECK(std::fabs(blackman[50] - 1.0) < 1e-15);

        Buffer bartlett = gen.drawWindow(1.0, BARTLETT);
        CHECK(bartlett[0] == 0.0);
        CHECK(bartlett[25] == 0.5);
        CHECK(bartlett[50] == 1.0);

        Buffer parzen = gen.drawWindow(1.0, PARZEN);
        CHECK(std::fabs(parzen[0]) < 1e-15);
        CHECK(std::fabs(parzen[1] - parzen[98]) < 1e-15);

        check_close(gen.drawWindow(1.0, RECTANGULAR), Buffer::ones(100), 0.0);
    }

    SECTION("every window type")
    {
        for(int type = BARTLETT; type <= RECTANGULAR; ++type)
        {
            INFO("window type " << type);

            Buffer w = gen.drawWindow(1.0, static_cast<WindowType>(type));

            CHECK(w.getLength() == 100);
            CHECK(w.getMax() <= 1.0 + 1e-12);
            CHECK(w.getMin() >= -1e-12);
            CHECK(w.getMax() > 0.3);
        }
    }

    SECTION("an unknown window type throws")
    {
        CHECK_THROWS_AS(gen.drawWindow(1.0, static_cast<WindowType>(999)), Exception);
    }
}

TEST_CASE("Generator gaussianNoise()")
{
    Generator gen(1000);
    gen.setSeed(1234);

    // An odd number of samples, the samples come in pairs.
    Buffer noise = gen.gaussianNoise(10.001, 2.0, 0.5);

    CHECK(noise.getLength() == 10001);
    CHECK(std::fabs(noise.getMean() - 2.0) < 0.05);
    CHECK(std::fabs(noise.getStd() - 0.5) < 0.05);
}

TEST_CASE("Generator setChorus(0) turns the chorus off")
{
    Cosine plain(100);

    Cosine chorus(100);
    chorus.setChorus(3, 0.5);
    chorus.setChorus(0, 0.0);

    check_close(chorus.generate(1.0, 5.0), plain.generate(1.0, 5.0), 0.0);
}

TEST_CASE("Generator edge cases")
{
    SECTION("drawSine2() with a chorus of 1 voice and no detune is plain")
    {
        Generator plain(100);
        Generator chorus(100);
        chorus.setChorus(1, 0.0);

        check_close(chorus.drawSine2(1.0, 5.0, 0.25), plain.drawSine2(1.0, 5.0, 0.25), 1e-12);
    }

    SECTION("a negative phase wraps around the waveform")
    {
        Generator gen(8, make_ramp_waveform());

        // A phase of -0.25 is 1 sample back, so it starts on the last one.
        check_close(
            gen.generate2(1.0, 1.0, -0.25),
            Buffer({0.875, 0.0, 0.125, 0.25, 0.375, 0.5, 0.625, 0.75}),
            0.0);
    }

    SECTION("gaussianNoise() with an even number of samples")
    {
        Generator gen(100);

        CHECK(gen.gaussianNoise(1.0, 0.0, 1.0).getLength() == 100);
    }

    SECTION("drawDecay() with the default alpha of 2 pi")
    {
        Generator gen(4);

        check_close(gen.drawDecay(1.0), gen.drawDecay(1.0, 2.0 * M_PI), 0.0);
    }

    SECTION("a Kaiser beta below 1 is 1")
    {
        Generator gen(100);

        check_close(gen.drawWindowKaiser(1.0, 0.5), gen.drawWindowKaiser(1.0, 1.0), 0.0);
    }
}
