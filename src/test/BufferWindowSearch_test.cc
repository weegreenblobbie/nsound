//-----------------------------------------------------------------------------
//
//  BufferWindowSearch tests, windows of a Buffer that end at zero crossings.
//
//-----------------------------------------------------------------------------

#include <Nsound/Buffer.h>
#include <Nsound/BufferWindowSearch.h>
#include <Nsound/Sine.h>

#include "gold_check.h"

#include <catch2/catch_test_macros.hpp>

using namespace Nsound;

TEST_CASE("BufferWindowSearch")
{
    Sine sine(1000.0);
    Buffer x = sine.generate(1.0, 7.0);

    BufferWindowSearch search(x, 100);

    // The windows cover the Buffer, in order.
    Buffer joined;
    uint32 n_windows = 0;
    while(search.getSamplesLeft() > 0)
    {
        joined << search.getNextWindow();
        ++n_windows;
    }

    check_close(joined, x, 0.0);
    CHECK(n_windows > 1);

    SECTION("copies give the same windows")
    {
        search.reset();
        Buffer first = search.getNextWindow();

        BufferWindowSearch copy(search);
        BufferWindowSearch other(Buffer::ones(10), 2);
        other = search;

        BufferWindowSearch & same = other;
        other = same;

        Buffer next = search.getNextWindow();

        check_close(copy.getNextWindow(), next, 0.0);
        check_close(other.getNextWindow(), next, 0.0);
    }

    SECTION("a window must be at least 2 samples")
    {
        CHECK_THROWS_AS(BufferWindowSearch(x, 1), Exception);
    }
}
