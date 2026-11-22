//-----------------------------------------------------------------------------
//
//  FilterHighPassFIR tests, compared with the gold files test_filters.py uses.
//
//-----------------------------------------------------------------------------

#include <Nsound/AudioStream.h>
#include <Nsound/Buffer.h>
#include <Nsound/FilterHighPassFIR.h>

#include "filter_signals.h"
#include "gold_check.h"

#include <catch2/catch_test_macros.hpp>

using namespace Nsound;

TEST_CASE("FilterHighPassFIR filter()")
{
    FilterSignals s = make_filter_signals();

    FilterHighPassFIR f(s.sample_rate, 256, 880.0);

    check_gold(f.filter(s.input), "gold/filter_fir_high_pass.wav", FILTER_GAMMA);

    // With cutoffs that change every sample.
    check_gold(f.filter(s.input, s.freqs_hi), "gold/filter_fir_high_pass_dynamic.wav", FILTER_GAMMA);
}

TEST_CASE("FilterHighPassFIR overloads agree")
{
    FilterSignals s = make_filter_signals();

    FilterHighPassFIR f(s.sample_rate, 256, 880.0);

    CHECK(f.getFrequency() == 880.0);

    check_one_cutoff_overloads(f, s.input.subbuffer(0, 4000), 880.0);
}

TEST_CASE("FilterHighPassFIR Filter base methods")
{
    check_filter_base<FilterHighPassFIR>(8000.0, 256, 880.0);
}

TEST_CASE("FilterHighPassFIR getImpulseResponse()")
{
    FilterHighPassFIR f(8000.0, 256, 880.0);

    // The kernel is a low pass kernel mirrored about Nyquist, so its DC gain
    // is the low pass leakage at Nyquist, below -100 dB. 256 taps are made 257.
    Buffer kernel = f.getImpulseResponse().subbuffer(0, f.getKernelSize());

    CHECK(f.getKernelSize() == 257);
    CHECK(std::fabs(kernel.getSum()) < 1e-5);
}

TEST_CASE("FilterHighPassFIR below 0.1 Hz passes everything, delayed")
{
    FilterSignals s = make_filter_signals();

    Buffer x = s.input.subbuffer(0, 1000);

    // The all pass kernel delays by the center tap, 128 samples, like the
    // other kernels.
    FilterHighPassFIR f(s.sample_rate, 256, 0.05);

    Buffer y = f.filter(x);

    check_close(y.subbuffer(0, 128), Buffer::zeros(128), 0.0);
    check_close(y.subbuffer(128), x.subbuffer(0, 1000 - 128), 0.0);
}

TEST_CASE("FilterHighPassFIR delays a pass band tone by exactly 128 samples")
{
    // A linear phase kernel of 257 taps delays every frequency by 128.
    Sine sine(8000);

    Buffer x = sine.generate(0.5, 2000.0);

    FilterHighPassFIR f(8000, 256, 880.0);

    Buffer y = f.filter(x);

    check_close(y.subbuffer(1000, 2000), x.subbuffer(1000 - 128, 2000), 1e-5);
}
