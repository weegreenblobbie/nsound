//-----------------------------------------------------------------------------
//
//  FilterBandRejectFIR tests, compared with the gold files test_filters.py uses.
//
//-----------------------------------------------------------------------------

#include <Nsound/AudioStream.h>
#include <Nsound/Buffer.h>
#include <Nsound/FilterBandRejectFIR.h>
#include <Nsound/FilterLowPassFIR.h>

#include "filter_signals.h"
#include "gold_check.h"

#include <catch2/catch_test_macros.hpp>

using namespace Nsound;

TEST_CASE("FilterBandRejectFIR filter()")
{
    FilterSignals s = make_filter_signals();

    FilterBandRejectFIR f(s.sample_rate, 512, 450.0, 870.0);

    check_gold(f.filter(s.input), "gold/filter_fir_band_reject.wav", FILTER_GAMMA);

    // With cutoffs that change every sample.
    check_gold(f.filter(s.input, s.freqs_lo, s.freqs_hi), "gold/filter_fir_band_reject_dynamic.wav", FILTER_GAMMA);
}

TEST_CASE("FilterBandRejectFIR overloads agree")
{
    FilterSignals s = make_filter_signals();

    FilterBandRejectFIR f(s.sample_rate, 512, 450.0, 870.0);

    CHECK(f.getFrequencyLow() == 450.0);
    CHECK(f.getFrequencyHigh() == 870.0);

    check_two_cutoff_overloads(f, s.input.subbuffer(0, 4000), 450.0, 870.0);
}

TEST_CASE("FilterBandRejectFIR Filter base methods")
{
    check_filter_base<FilterBandRejectFIR>(8000.0, 512, 450.0, 870.0);
}

TEST_CASE("FilterBandRejectFIR below 0.1 Hz passes everything, delayed")
{
    FilterSignals s = make_filter_signals();

    Buffer x = s.input.subbuffer(0, 1000);

    FilterBandRejectFIR f(s.sample_rate, 256, 450.0, 870.0);

    // Both cutoffs below 0.1 Hz is an all pass kernel, which delays by the
    // center tap, 128 samples, like the other kernels.
    Buffer low = 0.05 * Buffer::ones(1000);
    Buffer y = f.filter(x, low, low);

    check_close(y.subbuffer(0, 128), Buffer::zeros(128), 0.0);
    check_close(y.subbuffer(128), x.subbuffer(0, 1000 - 128), 0.0);
}

TEST_CASE("FilterBandRejectFIR with only a low cutoff is a low pass")
{
    FilterSignals s = make_filter_signals();

    Buffer x = s.input.subbuffer(0, 1000);

    FilterBandRejectFIR f(s.sample_rate, 512, 450.0, 870.0);

    // A high cutoff below 0.1 Hz leaves the low pass part.
    Buffer y = f.filter(x, 500.0 * Buffer::ones(1000), 0.05 * Buffer::ones(1000));

    FilterLowPassFIR low_pass(s.sample_rate, 512, 500.0);

    check_close(y, low_pass.filter(x), 0.0);
}
