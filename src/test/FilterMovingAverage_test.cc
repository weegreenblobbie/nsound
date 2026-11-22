//-----------------------------------------------------------------------------
//
//  FilterMovingAverage tests.
//
//-----------------------------------------------------------------------------

#include <Nsound/AudioStream.h>
#include <Nsound/Buffer.h>
#include <Nsound/FilterMovingAverage.h>

#include "filter_signals.h"
#include "gold_check.h"

#include <catch2/catch_test_macros.hpp>

using namespace Nsound;

TEST_CASE("FilterMovingAverage averages the last n samples")
{
    FilterMovingAverage f(4);

    SECTION("a constant averages to itself")
    {
        check_close(f.filter(Buffer::ones(6)), Buffer::ones(6), 0.0);
    }

    SECTION("the window starts full of the first sample")
    {
        // [1 1 1 1] [1 1 1 2] [1 1 2 3] [1 2 3 4] [2 3 4 5] [3 4 5 6]
        check_close(
            f.filter(Buffer({1, 2, 3, 4, 5, 6})),
            Buffer({1.0, 1.25, 1.75, 2.5, 3.5, 4.5}),
            1e-15);
    }
}

TEST_CASE("FilterMovingAverage overloads agree, the frequency is ignored")
{
    FilterSignals s = make_filter_signals();

    Buffer x = s.input.subbuffer(0, 1000);
    Buffer constant = 440.0 * Buffer::ones(1000);

    FilterMovingAverage f(8);

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
}

TEST_CASE("FilterMovingAverage copies")
{
    FilterMovingAverage f(4);

    Buffer x({1, 2, 3, 4, 5, 6});
    Buffer expected = f.filter(x);

    SECTION("the copy constructor")
    {
        FilterMovingAverage copy(f);

        check_close(copy.filter(x), expected, 0.0);
    }

    SECTION("operator=")
    {
        FilterMovingAverage other(8);
        other = f;

        check_close(other.filter(x), expected, 0.0);

        FilterMovingAverage & same = other;
        other = same;

        check_close(other.filter(x), expected, 0.0);
    }
}
