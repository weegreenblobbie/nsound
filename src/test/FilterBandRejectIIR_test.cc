//-----------------------------------------------------------------------------
//
//  FilterBandRejectIIR tests, compared with the gold files test_filters.py uses.
//
//-----------------------------------------------------------------------------

#include <Nsound/AudioStream.h>
#include <Nsound/Buffer.h>
#include <Nsound/FilterBandRejectIIR.h>

#include "filter_signals.h"
#include "gold_check.h"

#include <catch2/catch_test_macros.hpp>

using namespace Nsound;

TEST_CASE("FilterBandRejectIIR filter()")
{
    FilterSignals s = make_filter_signals();

    FilterBandRejectIIR f(s.sample_rate, 4, 440.0, 880.0, 0.01);

    check_gold(f.filter(s.input), "gold/filter_iir_band_reject.wav", FILTER_GAMMA);

    // With cutoffs that change every sample.
    check_gold(f.filter(s.input, s.freqs_lo, s.freqs_hi), "gold/filter_iir_band_reject_dynamic.wav", FILTER_GAMMA);
}

TEST_CASE("FilterBandRejectIIR overloads agree")
{
    FilterSignals s = make_filter_signals();

    FilterBandRejectIIR f(s.sample_rate, 4, 440.0, 880.0, 0.01);

    CHECK(f.getFrequencyLow() == 440.0);
    CHECK(f.getFrequencyHigh() == 880.0);

    check_two_cutoff_overloads(f, s.input.subbuffer(0, 4000), 440.0, 880.0);
}

TEST_CASE("FilterBandRejectIIR Filter base methods")
{
    check_filter_base<FilterBandRejectIIR>(8000.0, 4, 440.0, 880.0, 0.01);
}
