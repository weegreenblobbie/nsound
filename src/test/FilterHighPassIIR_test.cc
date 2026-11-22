//-----------------------------------------------------------------------------
//
//  FilterHighPassIIR tests, compared with the gold files test_filters.py uses.
//
//-----------------------------------------------------------------------------

#include <Nsound/AudioStream.h>
#include <Nsound/Buffer.h>
#include <Nsound/FilterHighPassIIR.h>

#include "filter_signals.h"
#include "gold_check.h"

#include <catch2/catch_test_macros.hpp>

using namespace Nsound;

TEST_CASE("FilterHighPassIIR filter()")
{
    FilterSignals s = make_filter_signals();

    FilterHighPassIIR f(s.sample_rate, 4, 880.0, 0.01);

    check_gold(f.filter(s.input), "gold/filter_iir_high_pass.wav", FILTER_GAMMA);

    // With cutoffs that change every sample.
    check_gold(f.filter(s.input, s.freqs_hi), "gold/filter_iir_high_pass_dynamic.wav", FILTER_GAMMA);
}

TEST_CASE("FilterHighPassIIR overloads agree")
{
    FilterSignals s = make_filter_signals();

    FilterHighPassIIR f(s.sample_rate, 4, 880.0, 0.01);

    CHECK(f.getFrequency() == 880.0);

    check_one_cutoff_overloads(f, s.input.subbuffer(0, 4000), 880.0);
}

TEST_CASE("FilterHighPassIIR Filter base methods")
{
    check_filter_base<FilterHighPassIIR>(8000.0, 4, 880.0, 0.01);
}
