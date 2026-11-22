//-----------------------------------------------------------------------------
//
//  FilterMedian tests, compared with hand-computed medians.
//
//-----------------------------------------------------------------------------

#include <Nsound/AudioStream.h>
#include <Nsound/Buffer.h>
#include <Nsound/FilterMedian.hpp>

#include "gold_check.h"

#include <catch2/catch_test_macros.hpp>

using namespace Nsound;

static const float64 GAMMA = 1.5e-14;

TEST_CASE("FilterMedian")
{
    SECTION("window 3, increasing input")
    {
        FilterMedian fm(3);

        Buffer data = fm.filter(Buffer({1, 2, 3, 4, 5, 6, 7}));

        // pool     median
        // {1 1 1}  1
        // {1 1 2}  1
        // {1 2 3}  2
        // {2 3 4}  3
        // {3 4 5}  4
        // {4 5 6}  5
        // {5 6 7}  6
        check_close(data, Buffer({1.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0}), GAMMA);
    }

    SECTION("window 3, unsorted input")
    {
        FilterMedian fm(3);

        Buffer data = fm.filter(Buffer({6, 4, 2, 1, 7, 3, 5}));

        // pool     median
        // {6 6 6}  6
        // {4 6 6}  6
        // {2 4 6}  4
        // {1 2 4}  2
        // {1 2 7}  2
        // {1 3 7}  3
        // {3 5 7}  5
        check_close(data, Buffer({6.0, 6.0, 4.0, 2.0, 2.0, 3.0, 5.0}), GAMMA);
    }

    SECTION("FilterMedian(4), unsorted input")
    {
        // The old test called this "w=5" and its pools have 5 values.
        FilterMedian fm(4);

        Buffer data = fm.filter(Buffer({6, 4, 2, 1, 7, 3, 5, 9, 4, 6, 2}));

        //     pool     median
        // {6 6 6 6 6}  6
        // {4 6 6 6 6}  6
        // {2 4 6 6 6}  6
        // {1 2 4 6 6}  4
        // {1 2 4 6 7}  4
        // {1 2 3 4 7}  3
        // {1 2 3 5 7}  3
        // {1 3 5 7 9}  5
        // {3 4 5 7 9}  5
        // {3 4 5 6 9}  5
        // {2 4 5 6 9}  5
        check_close(
            data,
            Buffer({6.0, 6.0, 6.0, 4.0, 4.0, 3.0, 3.0, 5.0, 5.0, 5.0, 5.0}),
            GAMMA);
    }
}

TEST_CASE("FilterMedian filters each channel from the start")
{
    FilterMedian fm(3);

    AudioStream as(10.0, 2);
    as << Buffer({6, 4, 2, 1, 7, 3, 5});

    AudioStream out = fm.filter(as);

    REQUIRE(out.getNChannels() == 2);

    for(uint32 channel = 0; channel < 2; ++channel)
    {
        INFO("channel " << channel);
        check_close(out[channel], Buffer({6.0, 6.0, 4.0, 2.0, 2.0, 3.0, 5.0}), GAMMA);
    }
}

TEST_CASE("FilterMedian checks its arguments")
{
    CHECK_THROWS_AS(FilterMedian(2), Exception);

    FilterMedian fm(3);

    CHECK_THROWS_AS(fm.filter(Buffer()), Exception);
}
