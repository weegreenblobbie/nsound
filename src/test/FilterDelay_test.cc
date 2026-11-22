//-----------------------------------------------------------------------------
//
//  FilterDelay tests, compared with the gold files.
//
//-----------------------------------------------------------------------------

#include <Nsound/AudioStream.h>
#include <Nsound/Buffer.h>
#include <Nsound/FilterDelay.h>
#include <Nsound/Sine.h>
#include <Nsound/Wavefile.h>

#include "filter_signals.h"
#include "gold_check.h"

#include <catch2/catch_test_macros.hpp>

using namespace Nsound;

static const float64 GAMMA = 1.5e-14;

TEST_CASE("FilterDelay filter(input)")
{
    Wavefile::setDefaultSampleSize(48);

    FilterDelay f(100.0, 3.0);
    Sine sine(100);

    AudioStream input(100, 1);

    input << sine.generate(0.25, 4.0)
          << sine.silence(0.75);

    SECTION("delay 0.333")
    {
        check_gold(f.filter(input, 0.333), "gold/FilterDelay_out1.wav", GAMMA);
    }

    SECTION("delay 0.666")
    {
        check_gold(f.filter(input, 0.666), "gold/FilterDelay_out2.wav", GAMMA);
    }
}

TEST_CASE("FilterDelay filter(impulse)")
{
    Wavefile::setDefaultSampleSize(48);

    FilterDelay f(1.0, 5.0);

    AudioStream input(1, 1);

    input << 1.0 << 0.0 << 0.0 << 0.0 << 0.0
          << 0.0 << 0.0 << 0.0 << 0.0 << 0.0;

    check_gold(f.filter(input, 60.0), "gold/FilterDelay_out3.wav", GAMMA);
}

TEST_CASE("FilterDelay delays by whole samples")
{
    FilterDelay f(10.0, 0.5);

    Buffer impulse = Buffer::zeros(8);
    impulse[0] = 1.0;

    // 0.3 seconds is 3 samples.
    Buffer expected = Buffer::zeros(8);
    expected[3] = 1.0;

    check_close(f.filter(impulse, 0.3), expected, 0.0);

    SECTION("filter(x) delays by the maximum")
    {
        expected = Buffer::zeros(8);
        expected[5] = 1.0;

        check_close(f.filter(impulse), expected, 0.0);
    }

    SECTION("a longer delay is limited to the maximum, a negative one to 0")
    {
        check_close(f.filter(impulse, 2.0), f.filter(impulse), 0.0);
        check_close(f.filter(impulse, -1.0), impulse, 0.0);
    }
}

TEST_CASE("FilterDelay overloads agree")
{
    FilterSignals s = make_filter_signals();

    FilterDelay f(s.sample_rate, 0.01);

    check_one_cutoff_overloads(f, s.input.subbuffer(0, 1000), 0.01);
}

TEST_CASE("FilterDelay Filter base")
{
    check_filter_base<FilterDelay>(1000.0, 0.01);
}
