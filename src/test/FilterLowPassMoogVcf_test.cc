//-----------------------------------------------------------------------------
//
//  FilterLowPassMoogVcf tests, four one pole low passes with resonant
//  feedback and a soft clipper.
//
//-----------------------------------------------------------------------------

#include <Nsound/AudioStream.h>
#include <Nsound/Buffer.h>
#include <Nsound/FilterLowPassMoogVcf.h>
#include <Nsound/Sine.h>

#include "filter_signals.h"
#include "gold_check.h"

#include <catch2/catch_test_macros.hpp>

#include <cmath>

using namespace Nsound;

//! The root mean square of the second half of b, after the filter settles.
static
float64
settled_rms(const Buffer & b)
{
    Buffer half = b.subbuffer(b.getLength() / 2);
    half *= half;
    return std::sqrt(half.getMean());
}

TEST_CASE("FilterLowPassMoogVcf low passes")
{
    Sine sine(8000.0);

    // Small, so the clipper barely acts.
    float64 amplitude = 0.01;
    float64 rms = amplitude / std::sqrt(2.0);

    FilterLowPassMoogVcf f(8000.0, 500.0, 0.0);

    SECTION("DC passes")
    {
        Buffer y = f.filter(amplitude * Buffer::ones(2000));

        CHECK(std::fabs(y[1999] - amplitude) < 1e-6);
    }

    SECTION("a tone below the cutoff passes, one far above is cut")
    {
        CHECK(settled_rms(f.filter(amplitude * sine.generate(0.5, 100.0))) > 0.9 * rms);
        CHECK(settled_rms(f.filter(amplitude * sine.generate(0.5, 3000.0))) < 0.001 * rms);
    }

    SECTION("resonance peaks at the cutoff")
    {
        FilterLowPassMoogVcf resonant(8000.0, 500.0, 0.9);

        Buffer x = amplitude * sine.generate(0.5, 500.0);

        CHECK(settled_rms(resonant.filter(x)) > 1.5 * settled_rms(f.filter(x)));
    }
}

TEST_CASE("FilterLowPassMoogVcf overloads agree")
{
    FilterSignals s = make_filter_signals();

    Buffer x = 0.1 * s.input.subbuffer(0, 2000);

    FilterLowPassMoogVcf f(s.sample_rate, 500.0, 0.5);

    Buffer y = f.filter(x);

    Buffer per_sample;
    Buffer per_sample_fc;
    Buffer per_sample_fc_r;

    f.reset();
    for(uint32 i = 0; i < 2000; ++i) per_sample << f.filter(x[i]);

    f.reset();
    for(uint32 i = 0; i < 2000; ++i) per_sample_fc << f.filter(x[i], 500.0);

    f.reset();
    for(uint32 i = 0; i < 2000; ++i) per_sample_fc_r << f.filter(x[i], 500.0, 0.5);

    check_close(per_sample, y, 0.0);
    check_close(per_sample_fc, y, 0.0);
    check_close(per_sample_fc_r, y, 0.0);

    // The Buffer overloads with a cutoff, from Filter.
    check_close(f.filter(x, 500.0), y, 0.0);
    check_close(f.filter(x, 500.0 * Buffer::ones(2000)), y, 0.0);

    AudioStream as(s.sample_rate, 2);
    as << x;

    check_every_channel(f.filter(as), y);
    check_every_channel(f.filter(as, 500.0), y);
}

TEST_CASE("FilterLowPassMoogVcf reset() goes back to the first cutoff")
{
    FilterSignals s = make_filter_signals();

    Buffer x = 0.1 * s.input.subbuffer(0, 2000);

    FilterLowPassMoogVcf f(s.sample_rate, 500.0, 0.5);

    Buffer expected = f.filter(x);

    // Another cutoff and resonance, then back.
    for(uint32 i = 0; i < 100; ++i) f.filter(x[i], 2000.0, 0.9);

    CHECK((f.filter(x, 2000.0) - expected).getAbs().getMax() > 0.01);

    f.reset();

    Buffer y;
    for(uint32 i = 0; i < 2000; ++i) y << f.filter(x[i]);

    check_close(y, expected, 0.0);
}

TEST_CASE("FilterLowPassMoogVcf Filter base")
{
    check_filter_base<FilterLowPassMoogVcf>(8000.0, 500.0, 0.5);
}
