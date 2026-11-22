//-----------------------------------------------------------------------------
//
//  FilterLowPassFIR tests, compared with the gold files test_filters.py uses.
//
//-----------------------------------------------------------------------------

#include <Nsound/AudioStream.h>
#include <Nsound/Buffer.h>
#include <Nsound/FilterLowPassFIR.h>

#include "filter_signals.h"
#include "gold_check.h"

#include <catch2/catch_test_macros.hpp>

using namespace Nsound;

TEST_CASE("FilterLowPassFIR filter()")
{
    FilterSignals s = make_filter_signals();

    FilterLowPassFIR f(s.sample_rate, 256, 880.0);

    check_gold(f.filter(s.input), "gold/filter_fir_low_pass.wav", FILTER_GAMMA);

    // With cutoffs that change every sample.
    check_gold(f.filter(s.input, s.freqs_lo), "gold/filter_fir_low_pass_dynamic.wav", FILTER_GAMMA);
}

TEST_CASE("FilterLowPassFIR overloads agree")
{
    FilterSignals s = make_filter_signals();

    FilterLowPassFIR f(s.sample_rate, 256, 880.0);

    CHECK(f.getFrequency() == 880.0);

    check_one_cutoff_overloads(f, s.input.subbuffer(0, 4000), 880.0);
}

TEST_CASE("FilterLowPassFIR Filter base methods")
{
    check_filter_base<FilterLowPassFIR>(8000.0, 256, 880.0);
}

TEST_CASE("FilterLowPassFIR setCutoff(), getImpulseResponse()")
{
    FilterSignals s = make_filter_signals();

    Buffer x = s.input.subbuffer(0, 4000);

    FilterLowPassFIR f(s.sample_rate, 256, 880.0);

    SECTION("setCutoff() is the same as a new filter")
    {
        f.setCutoff(1200.0);

        CHECK(f.getFrequency() == 1200.0);

        FilterLowPassFIR expected(s.sample_rate, 256, 1200.0);

        check_close(f.filter(x), expected.filter(x), 0.0);
    }

    SECTION("the impulse response is the kernel, with a DC gain of 1")
    {
        // The kernel size is made odd: 256 taps become 257.
        REQUIRE(f.getKernelSize() == 257);

        Buffer h = f.getImpulseResponse();

        CHECK(std::fabs(h.subbuffer(0, 257).getSum() - 1.0) < 1e-12);
        CHECK(h.subbuffer(257).getAbs().getMax() == 0.0);

        // Symmetric about its center tap, so it is linear phase.
        Buffer kernel = h.subbuffer(0, 257);

        CHECK(h.argmax() == 128);
        CHECK((kernel - kernel.getReverse()).getAbs().getMax() < 1e-15);
    }
}

TEST_CASE("FilterLowPassFIR delays a pass band tone by exactly 128 samples")
{
    // A linear phase kernel of 257 taps delays every frequency by 128.
    Sine sine(8000);

    Buffer x = sine.generate(0.5, 300.0);

    FilterLowPassFIR f(8000, 256, 880.0);

    Buffer y = f.filter(x);

    check_close(y.subbuffer(1000, 2000), x.subbuffer(1000 - 128, 2000), 1e-5);
}
