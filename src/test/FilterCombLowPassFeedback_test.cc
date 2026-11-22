//-----------------------------------------------------------------------------
//
//  FilterCombLowPassFeedback tests, compared with the gold files.
//
//-----------------------------------------------------------------------------

#include <Nsound/AudioStream.h>
#include <Nsound/Buffer.h>
#include <Nsound/FilterCombLowPassFeedback.h>
#include <Nsound/Sine.h>
#include <Nsound/Wavefile.h>

#include "filter_signals.h"
#include "gold_check.h"

#include <catch2/catch_test_macros.hpp>

#include <cmath>

using namespace Nsound;

static const float64 GAMMA = 1.5e-14;

TEST_CASE("FilterCombLowPassFeedback")
{
    Wavefile::setDefaultSampleSize(48);

    FilterCombLowPassFeedback f(100.0, 0.1, 0.66, 12.0);
    Sine sine(100);

    Buffer noise("gold/Filter_noise.wav");

    SECTION("filter(input)")
    {
        Buffer data = f.filter(noise);
        data.normalize();

        check_gold(data, "gold/FilterCombLowPassFeedback_out1.wav", GAMMA);
    }

    SECTION("filter(input, freqs)")
    {
        Buffer freqs = sine.drawLine(1.0, 6.0, 50.0);

        Buffer data = f.filter(noise, freqs);
        data.normalize();

        check_gold(data, "gold/FilterCombLowPassFeedback_out2.wav", GAMMA);
    }
}

//! An impulse n samples long.
static
Buffer
make_impulse(uint32 n)
{
    Buffer impulse = Buffer::zeros(n);
    impulse[0] = 1.0;
    return impulse;
}

TEST_CASE("FilterCombLowPassFeedback impulse response")
{
    // A 0.3 second delay at 10 Hz is 3 samples, the feedback comes round
    // every 4.
    SECTION("no low pass, a 0 Hz cutoff, halves every loop")
    {
        FilterCombLowPassFeedback f(10.0, 0.3, 0.5, 0.0);

        Buffer expected = Buffer::zeros(16);
        expected[3] = 1.0;
        expected[7] = 0.5;
        expected[11] = 0.25;
        expected[15] = 0.125;

        check_close(f.filter(make_impulse(16)), expected, 0.0);
    }

    SECTION("a 1 Hz cutoff low passes the feedback")
    {
        // damp1 = 1 / 10, the feedback is 0.5 * 0.9 * 1.
        FilterCombLowPassFeedback f(10.0, 0.3, 0.5, 1.0);

        Buffer y = f.filter(make_impulse(16));

        CHECK(y[3] == 1.0);
        CHECK(std::fabs(y[7] - 0.45) < 1e-15);
    }

    SECTION("the feedback gain is limited to [0, 0.999999]")
    {
        FilterCombLowPassFeedback none(10.0, 0.3, -1.0, 0.0);
        FilterCombLowPassFeedback most(10.0, 0.3, 2.0, 0.0);

        Buffer expected = Buffer::zeros(16);
        expected[3] = 1.0;

        check_close(none.filter(make_impulse(16)), expected, 0.0);
        CHECK(std::fabs(most.filter(make_impulse(16))[7] - 0.999999) < 1e-15);
    }

    SECTION("the cutoff is limited to half the sample rate")
    {
        FilterCombLowPassFeedback half(10.0, 0.3, 0.5, 5.0);
        FilterCombLowPassFeedback above(10.0, 0.3, 0.5, 9.0);

        check_close(above.filter(make_impulse(16)), half.filter(make_impulse(16)), 0.0);

        Buffer y;
        above.reset();
        for(uint32 i = 0; i < 16; ++i) y << above.filter(make_impulse(16)[i], 9.0);

        check_close(y, half.filter(make_impulse(16)), 0.0);
    }
}

TEST_CASE("FilterCombLowPassFeedback overloads agree")
{
    FilterSignals s = make_filter_signals();

    Buffer x = s.input.subbuffer(0, 1000);
    Buffer constant = 440.0 * Buffer::ones(1000);

    FilterCombLowPassFeedback f(s.sample_rate, 0.01, 0.5, 440.0);

    Buffer y = f.filter(x);

    Buffer per_sample;
    Buffer per_sample_f;

    f.reset();
    for(uint32 i = 0; i < 1000; ++i) per_sample << f.filter(x[i]);

    f.reset();
    for(uint32 i = 0; i < 1000; ++i) per_sample_f << f.filter(x[i], 440.0);

    check_close(per_sample, y, 0.0);
    check_close(per_sample_f, y, 0.0);
    check_close(f.filter(x, constant), y, 0.0);

    AudioStream as(s.sample_rate, 2);
    as << x;

    check_every_channel(f.filter(as), y);
    check_every_channel(f.filter(as, constant), y);
}

TEST_CASE("FilterCombLowPassFeedback copies")
{
    FilterCombLowPassFeedback f(1000.0, 0.01, 0.5, 100.0);

    Buffer x = make_impulse(100);

    // Part way through, so the copies have history to carry over.
    for(uint32 i = 0; i < 15; ++i) f.filter(x[i]);

    Buffer expected;
    {
        FilterCombLowPassFeedback copy(f);
        for(uint32 i = 0; i < 100; ++i) expected << copy.filter(x[i]);
    }

    CHECK(expected.getAbs().getMax() > 0.1);

    SECTION("operator=")
    {
        FilterCombLowPassFeedback other(1000.0, 0.005, 0.9, 10.0);
        other = f;

        Buffer y;
        for(uint32 i = 0; i < 100; ++i) y << other.filter(x[i]);

        check_close(y, expected, 0.0);
    }

    SECTION("self assignment")
    {
        FilterCombLowPassFeedback & same = f;
        f = same;

        Buffer y;
        for(uint32 i = 0; i < 100; ++i) y << f.filter(x[i]);

        check_close(y, expected, 0.0);
    }
}

TEST_CASE("FilterCombLowPassFeedback Filter base")
{
    check_filter_base<FilterCombLowPassFeedback>(1000.0, 0.01, 0.5, 100.0);

    FilterCombLowPassFeedback f(1000.0, 0.01, 0.5, 100.0);

    CHECK_NOTHROW(f.plot(true, true));
}
