//-----------------------------------------------------------------------------
//
//  FilterTone tests, the one pole low pass y[n] = b x[n] - a y[n-1].
//
//-----------------------------------------------------------------------------

#include <Nsound/AudioStream.h>
#include <Nsound/Buffer.h>
#include <Nsound/FilterTone.h>

#include "filter_signals.h"
#include "gold_check.h"

#include <catch2/catch_test_macros.hpp>

#include <cmath>

using namespace Nsound;

// At a quarter of the sample rate, cos() = 0, so a = -(2 - sqrt(3)) and
// b = 1 + a.
static const float64 A = -(2.0 - std::sqrt(3.0));
static const float64 B = 1.0 + A;

TEST_CASE("FilterTone impulse response is b (-a)^n")
{
    FilterTone f(8.0, 2.0);

    Buffer expected;

    for(int i = 0; i < 6; ++i)
    {
        expected << B * std::pow(-A, i);
    }

    check_close(f.filter(Buffer({1, 0, 0, 0, 0, 0})), expected, 1e-15);
}

TEST_CASE("FilterTone passes DC")
{
    FilterTone f(8000.0, 440.0);

    // b = 1 + a, so a constant settles to itself.
    Buffer y = f.filter(Buffer::ones(4000));

    CHECK(std::fabs(y[3999] - 1.0) < 1e-12);
}

TEST_CASE("FilterTone overloads agree")
{
    FilterSignals s = make_filter_signals();

    Buffer x = s.input.subbuffer(0, 1000);
    Buffer constant = 440.0 * Buffer::ones(1000);

    FilterTone f(s.sample_rate, 440.0);

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

    // A different cutoff filters differently.
    CHECK((f.filter(x, 880.0 * Buffer::ones(1000)) - y).getAbs().getMax() > 0.01);

    CHECK_NOTHROW(f.plot(true, true));
}
