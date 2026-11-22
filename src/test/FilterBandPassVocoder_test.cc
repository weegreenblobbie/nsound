//-----------------------------------------------------------------------------
//
//  FilterBandPassVocoder tests, two state variable filters centered between
//  the low and high frequencies.
//
//-----------------------------------------------------------------------------

#include <Nsound/AudioStream.h>
#include <Nsound/Buffer.h>
#include <Nsound/FilterBandPassVocoder.h>
#include <Nsound/Sine.h>

#include "filter_signals.h"
#include "gold_check.h"

#include <catch2/catch_test_macros.hpp>

#include <cmath>
#include <type_traits>

using namespace Nsound;

static_assert(!std::is_copy_constructible_v<FilterBandPassVocoder>);
static_assert(!std::is_copy_assignable_v<FilterBandPassVocoder>);

//! The root mean square of the second half of b, after the filter settles.
static
float64
settled_rms(const Buffer & b)
{
    Buffer half = b.subbuffer(b.getLength() / 2);
    half *= half;
    return std::sqrt(half.getMean());
}

TEST_CASE("FilterBandPassVocoder passes its band")
{
    Sine sine(8000.0);

    FilterBandPassVocoder f(8000.0, 800.0, 1200.0);

    float64 center = settled_rms(f.filter(sine.generate(0.5, 1000.0)));

    CHECK(center > 0.5 / std::sqrt(2.0));

    for(float64 hz : {100.0, 500.0, 2000.0, 3500.0})
    {
        INFO(hz << " Hz");
        CHECK(settled_rms(f.filter(sine.generate(0.5, hz))) < 0.2 * center);
    }
}

TEST_CASE("FilterBandPassVocoder overloads agree")
{
    FilterSignals s = make_filter_signals();

    Buffer x = s.input.subbuffer(0, 2000);
    Buffer low = 800.0 * Buffer::ones(2000);
    Buffer high = 1200.0 * Buffer::ones(2000);

    FilterBandPassVocoder f(s.sample_rate, 800.0, 1200.0);

    Buffer y = f.filter(x);

    Buffer per_sample;
    Buffer per_sample_f;
    Buffer per_sample_band;

    f.reset();
    for(uint32 i = 0; i < 2000; ++i) per_sample << f.filter(x[i]);

    f.reset();
    for(uint32 i = 0; i < 2000; ++i) per_sample_f << f.filter(x[i], 440.0);

    f.reset();
    for(uint32 i = 0; i < 2000; ++i) per_sample_band << f.filter(x[i], 800.0, 1200.0);

    check_close(per_sample, y, 0.0);
    check_close(per_sample_f, y, 0.0);
    check_close(per_sample_band, y, 0.0);
    check_close(f.filter(x, low), y, 0.0);

    // Each call starts from the beginning.
    check_close(f.filter(x, low, high), y, 0.0);
    check_close(f.filter(x, low, high), y, 0.0);

    AudioStream as(s.sample_rate, 2);
    as << x;

    check_every_channel(f.filter(as), y);
    check_every_channel(f.filter(as, low), y);
    check_every_channel(f.filter(as, low, high), y);

    // Another band filters differently, 40 to 60 Hz where these first
    // samples of the sweep are.
    Buffer other = f.filter(x, 0.05 * low, 0.05 * high);

    CHECK((other - y).getAbs().getMax() > 0.01);
}

TEST_CASE("FilterBandPassVocoder in real time keeps its state between calls")
{
    FilterSignals s = make_filter_signals();

    Buffer x = s.input.subbuffer(0, 2000);
    Buffer low = 800.0 * Buffer::ones(1000);
    Buffer high = 1200.0 * Buffer::ones(1000);

    FilterBandPassVocoder whole(s.sample_rate, 800.0, 1200.0);

    FilterBandPassVocoder f(s.sample_rate, 800.0, 1200.0);
    f.setRealtime(true);

    Buffer y;
    y << f.filter(x.subbuffer(0, 1000), low, high)
      << f.filter(x.subbuffer(1000), low, high);

    check_close(y, whole.filter(x), 0.0);

    AudioStream stereo(s.sample_rate, 2);
    stereo << x;

    CHECK_THROWS_AS(f.filter(stereo, low, high), Exception);
}

TEST_CASE("FilterBandPassVocoder Filter base")
{
    check_filter_base<FilterBandPassVocoder>(8000.0, 800.0, 1200.0);
}
