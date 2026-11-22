//-----------------------------------------------------------------------------
//
//  FilterLowPassIIR tests, compared with the gold files test_filters.py uses.
//
//-----------------------------------------------------------------------------

#include <Nsound/AudioStream.h>
#include <Nsound/Buffer.h>
#include <Nsound/FilterLowPassIIR.h>

#include "filter_signals.h"
#include "gold_check.h"

#include <catch2/catch_test_macros.hpp>

using namespace Nsound;

TEST_CASE("FilterLowPassIIR filter()")
{
    FilterSignals s = make_filter_signals();

    FilterLowPassIIR f(s.sample_rate, 4, 880.0, 0.01);

    check_gold(f.filter(s.input), "gold/filter_iir_low_pass.wav", FILTER_GAMMA);

    // With cutoffs that change every sample.
    check_gold(f.filter(s.input, s.freqs_lo), "gold/filter_iir_low_pass_dynamic.wav", FILTER_GAMMA);
}

TEST_CASE("FilterLowPassIIR overloads agree")
{
    FilterSignals s = make_filter_signals();

    FilterLowPassIIR f(s.sample_rate, 4, 880.0, 0.01);

    CHECK(f.getFrequency() == 880.0);

    check_one_cutoff_overloads(f, s.input.subbuffer(0, 4000), 880.0);
}

TEST_CASE("FilterLowPassIIR Filter base methods")
{
    check_filter_base<FilterLowPassIIR>(8000.0, 4, 880.0, 0.01);
}

TEST_CASE("FilterLowPassIIR setCutoff()")
{
    FilterSignals s = make_filter_signals();

    Buffer x = s.input.subbuffer(0, 4000);

    FilterLowPassIIR f(s.sample_rate, 4, 880.0, 0.01);

    SECTION("is the same as a new filter")
    {
        f.setCutoff(1200.0);

        CHECK(f.getFrequency() == 1200.0);

        FilterLowPassIIR expected(s.sample_rate, 4, 1200.0, 0.01);

        check_close(f.filter(x), expected.filter(x), 0.0);
    }

    SECTION("ignores 0 and frequencies at or past Nyquist")
    {
        Buffer before = f.filter(x);

        f.setCutoff(0.0);
        f.setCutoff(4000.0);

        CHECK(f.getFrequency() == 880.0);
        check_close(f.filter(x), before, 0.0);
    }
}
