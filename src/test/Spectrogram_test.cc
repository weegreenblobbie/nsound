//-----------------------------------------------------------------------------
//
//  Spectrogram tests.
//
//-----------------------------------------------------------------------------

#include <Nsound/AudioStream.h>
#include <Nsound/Buffer.h>
#include <Nsound/Sine.h>
#include <Nsound/Spectrogram.h>

#include "gold_check.h"

#include <catch2/catch_test_macros.hpp>

#include <cmath>

using namespace Nsound;

//! The index of the largest sample.
static
uint32
argmax(const Buffer & b)
{
    uint32 k = 0;
    for(uint32 i = 1; i < b.getLength(); ++i) if(b[i] > b[k]) k = i;
    return k;
}

// 64 sample Hanning windows every 32 samples at 1000 Hz, 15.625 Hz bins.
static const float64 SR = 1000.0;
static const float64 WINDOW = 0.064;
static const float64 STEP = 0.032;

TEST_CASE("Spectrogram axes")
{
    Sine sine(SR);
    Buffer x = sine.generate(1.0, 125.0);

    Spectrogram s(x, SR, WINDOW, STEP, HANNING);

    Buffer f = s.getFrequencyAxis();
    AudioStream m = s.getMagnitude();

    SECTION("a frequency for every bin, a time for every frame")
    {
        REQUIRE(f.getLength() == m.getLength());
        REQUIRE(s.getTimeAxis().getLength() == m.getNChannels());
    }

    SECTION("a tone peaks at its frequency")
    {
        // The axis was a bin high, 140.625 Hz here.
        for(uint32 k : {5u, 16u, 25u})
        {
            INFO("frame " << k);
            CHECK(f[argmax(m[k])] == 125.0);
        }
    }

    SECTION("the times are the window centers")
    {
        Buffer t = s.getTimeAxis();

        for(uint32 k = 0; k < t.getLength(); ++k)
        {
            INFO("frame " << k);
            CHECK(std::fabs(t[k] - k * STEP) < 1e-12);
        }
    }

    SECTION("a step between samples rounds, and the times follow it")
    {
        // 0.0333 seconds is 33 samples, the times used to step 0.0333.
        Spectrogram s2(x, SR, WINDOW, 0.0333, HANNING);

        Buffer t = s2.getTimeAxis();

        CHECK(std::fabs(t[t.getLength() - 1] - (t.getLength() - 1) * 0.033) < 1e-12);
    }
}

TEST_CASE("Spectrogram windows the edges like the middle")
{
    // A frame past either end is the same as the frame of the input padded
    // with zeros, the whole window applied once.
    Sine sine(SR);
    Buffer x = sine.generate(0.5, 125.0) + 0.5 * sine.generate(0.5, 300.0);

    Buffer padded;
    padded << Buffer::zeros(64) << x << Buffer::zeros(64);

    AudioStream m = Spectrogram(x, SR, WINDOW, STEP, HANNING).getMagnitude();
    AudioStream mp = Spectrogram(padded, SR, WINDOW, STEP, HANNING).getMagnitude();

    // padded is 2 steps ahead.
    for(uint32 k = 0; k < m.getNChannels(); ++k)
    {
        INFO("frame " << k);
        check_close(m[k], mp[k + 2], 1e-9);
    }
}

TEST_CASE("Spectrogram computeMagnitude() of the last window")
{
    Sine sine(SR);
    Buffer x = sine.generate(0.5, 250.0);

    Spectrogram s(x, SR, WINDOW, STEP, HANNING);

    // 250 Hz is bin 16.
    CHECK(argmax(s.computeMagnitude(x)) == 16);

    // Fewer samples than a window are padded with zeros.
    CHECK(argmax(s.computeMagnitude(x.subbuffer(0, 40))) == 16);
}

TEST_CASE("Spectrogram copies")
{
    Sine sine(SR);
    Buffer x = sine.generate(0.5, 125.0);

    Spectrogram s(x, SR, WINDOW, STEP, HANNING);

    Spectrogram copy(s);

    check_close(copy.getFrequencyAxis(), s.getFrequencyAxis(), 0.0);
    check_close(copy.getTimeAxis(), s.getTimeAxis(), 0.0);
    check_close(copy.getMagnitude()[3], s.getMagnitude()[3], 0.0);

    Spectrogram other(sine.generate(0.2, 50.0), SR, 0.032, 0.016, HAMMING);

    other = copy;

    check_close(other.getMagnitude()[3], s.getMagnitude()[3], 0.0);
    check_close(other.computeMagnitude(x), s.computeMagnitude(x), 0.0);

    Spectrogram & same = other;
    other = same;

    check_close(other.getTimeAxis(), s.getTimeAxis(), 0.0);
}

TEST_CASE("Spectrogram checks its arguments and plots")
{
    Sine sine(SR);
    Buffer x = sine.generate(0.5, 125.0);

    CHECK_THROWS_AS(Spectrogram(x, 0.0, WINDOW, STEP, HANNING), Exception);
    CHECK_THROWS_AS(Spectrogram(x, SR, 0.0, STEP, HANNING), Exception);
    CHECK_THROWS_AS(Spectrogram(x, SR, WINDOW, -1.0, HANNING), Exception);

    Spectrogram s(x, SR, WINDOW, STEP, HANNING);

    CHECK_NOTHROW(s.plot("dB"));
    CHECK_NOTHROW(s.plot("squashed", false, 0.5));
    CHECK_NOTHROW(s.plot("neither", false, 0.0));
}
