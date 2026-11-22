//-----------------------------------------------------------------------------
//
//  CircularBuffer tests.
//
//-----------------------------------------------------------------------------

#include <Nsound/AudioStream.h>
#include <Nsound/Buffer.h>
#include <Nsound/CircularBuffer.h>

#include "gold_check.h"

#include <catch2/catch_test_macros.hpp>

#include <type_traits>

using namespace Nsound;

static_assert(!std::is_copy_constructible_v<CircularBuffer>);
static_assert(!std::is_copy_assignable_v<CircularBuffer>);

TEST_CASE("CircularBuffer")
{
    CircularBuffer c(4);

    SECTION("starts full of zeros")
    {
        check_close(c.read(), Buffer::zeros(4), 0.0);
        CHECK(c.read_head() == 0.0);
    }

    SECTION("read() is oldest first, read_head() the oldest")
    {
        c.write(Buffer({1, 2, 3}));

        check_close(c.read(), Buffer({0, 1, 2, 3}), 0.0);
        CHECK(c.read_head() == 0.0);

        // Wrapping round keeps the last 4.
        c.write(4.0);
        c.write(5.0);

        check_close(c.read(), Buffer({2, 3, 4, 5}), 0.0);
        CHECK(c.read_head() == 2.0);
    }

    SECTION("an AudioStream is written as its mean")
    {
        AudioStream as(10.0, 2);
        as[0] << 2.0 << 4.0;
        as[1] << 6.0 << 8.0;

        c.write(as);

        check_close(c.read(), Buffer({0, 0, 4, 6}), 0.0);
    }

    SECTION("no samples throws")
    {
        CHECK_THROWS_AS(CircularBuffer(0), Exception);
    }
}
