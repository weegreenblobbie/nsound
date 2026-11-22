//-----------------------------------------------------------------------------
//
//  FilterPhaser tests, the input averaged with the mean of several swept
//  all pass stages.
//
//-----------------------------------------------------------------------------

#include <Nsound/AudioStream.h>
#include <Nsound/Buffer.h>
#include <Nsound/FilterPhaser.h>

#include "filter_signals.h"
#include "gold_check.h"

#include <catch2/catch_test_macros.hpp>

#include <cmath>

using namespace Nsound;

//! n samples from start towards finish, a step of (finish - start) / n.
static
Buffer
ramp(uint32 n, float64 start, float64 finish)
{
    Buffer b;

    for(uint32 i = 0; i < n; ++i)
    {
        b << start + (finish - start) * i / n;
    }

    return b;
}

TEST_CASE("FilterPhaser passes DC")
{
    // The all pass stages have a gain of 1 at DC, so once they fill a
    // constant comes out as itself.
    FilterPhaser f(1000.0, 3, 1.0, 0.5, 0.01);

    Buffer y = f.filter(Buffer::ones(2000));

    CHECK(y[0] < 1.0);
    CHECK(std::fabs(y[1999] - 1.0) < 1e-9);
}

TEST_CASE("FilterPhaser changes a tone")
{
    FilterSignals s = make_filter_signals();

    FilterPhaser f(s.sample_rate, 4, 1.0, 0.5, 0.002);

    Buffer x = s.input.subbuffer(0, 4000);

    CHECK((f.filter(x) - x).getAbs().getMax() > 0.1);
}

TEST_CASE("FilterPhaser checks its arguments")
{
    CHECK_THROWS_AS(FilterPhaser(1000.0, 0, 1.0, 0.5, 0.01), Exception);
    CHECK_THROWS_AS(FilterPhaser(1000.0, 3, 0.0, 0.5, 0.01), Exception);
    CHECK_THROWS_AS(FilterPhaser(1000.0, 3, 1.0, 0.0, 0.01), Exception);
    CHECK_THROWS_AS(FilterPhaser(1000.0, 3, 1.0, 0.5, 0.0), Exception);
}

TEST_CASE("FilterPhaser overloads agree, the frequency is ignored")
{
    FilterSignals s = make_filter_signals();

    Buffer x = s.input.subbuffer(0, 2000);

    FilterPhaser f(s.sample_rate, 3, 1.0, 0.5, 0.005);

    Buffer y = f.filter(x);

    Buffer per_sample;
    Buffer per_sample_f;

    f.reset();
    for(uint32 i = 0; i < 2000; ++i) per_sample << f.filter(x[i]);

    f.reset();
    for(uint32 i = 0; i < 2000; ++i) per_sample_f << f.filter(x[i], 440.0);

    check_close(per_sample, y, 0.0);
    check_close(per_sample_f, y, 0.0);

    AudioStream as(s.sample_rate, 2);
    as << x;

    check_every_channel(f.filter(as), y);
}

TEST_CASE("FilterPhaser copies carry the state")
{
    FilterPhaser f(1000.0, 3, 1.0, 0.5, 0.01);

    Buffer x = ramp(300, -1.0, 1.0);

    // Part way through, so the copies have state to carry over.
    for(uint32 i = 0; i < 150; ++i) f.filter(x[i]);

    Buffer expected;
    {
        FilterPhaser reference(1000.0, 3, 1.0, 0.5, 0.01);
        for(uint32 i = 0; i < 150; ++i) reference.filter(x[i]);
        for(uint32 i = 0; i < 300; ++i) expected << reference.filter(x[i]);
    }

    SECTION("the copy constructor")
    {
        FilterPhaser copy(f);

        Buffer y;
        for(uint32 i = 0; i < 300; ++i) y << copy.filter(x[i]);

        check_close(y, expected, 0.0);
    }

    SECTION("operator= onto a phaser with a different number of stages")
    {
        FilterPhaser other(1000.0, 5, 2.0, 1.0, 0.02);
        other = f;

        FilterPhaser & same = other;
        other = same;

        Buffer y;
        for(uint32 i = 0; i < 300; ++i) y << other.filter(x[i]);

        check_close(y, expected, 0.0);
    }
}

TEST_CASE("FilterPhaser Filter base")
{
    check_filter_base<FilterPhaser>(1000.0, 3, 1.0, 0.5, 0.01);
}

TEST_CASE("FilterPhaser sweep frequencies at or above the sample rate wrap")
{
    // The waveform is a second long, so sweeping at 1500 Hz steps through it
    // just as 500 Hz does, and never reads past its end.
    FilterPhaser above(1000.0, 2, 1500.0, 0.5, 0.01);
    FilterPhaser below(1000.0, 2, 500.0, 0.5, 0.01);

    Buffer x = ramp(100, -1.0, 1.0);

    check_close(above.filter(x), below.filter(x), 0.0);

    // At twice the sample rate it adds two whole seconds each sample.
    FilterPhaser twice(1000.0, 1, 2000.0, 0.5, 0.01);
    FilterPhaser still(1000.0, 1, 1000.0, 0.5, 0.01);

    check_close(twice.filter(x), still.filter(x), 0.0);
}
