//-----------------------------------------------------------------------------
//
//  Square tests.
//
//-----------------------------------------------------------------------------

#include <Nsound/Buffer.h>
#include <Nsound/Square.h>

#include "gold_check.h"

#include <catch2/catch_test_macros.hpp>

#include <cmath>

using namespace Nsound;

//! The number of samples in b equal to value.
static
uint32
count(const Buffer & b, float64 value)
{
    uint32 n = 0;
    for(auto x : b) if(x == value) ++n;
    return n;
}

TEST_CASE("Square default waveform")
{
    SECTION("half at 1, half at -1")
    {
        Buffer y = Square(100.0).generate(1.0, 1.0);

        check_close(y.subbuffer(0, 50), Buffer::ones(50), 0.0);
        check_close(y.subbuffer(50), -1.0 * Buffer::ones(50), 0.0);
    }

    SECTION("an odd sample rate")
    {
        // The two halves rounded up to 51 samples each, one too many.
        Buffer y = Square(101.0).generate(1.0, 1.0);

        REQUIRE(y.getLength() == 101);
        CHECK(count(y, 1.0) + count(y, -1.0) == 101);
    }
}

TEST_CASE("Square drawn from its corners")
{
    SECTION("whole samples")
    {
        Square sq(1000.0, 0.125, 1.0, 0.375, 0.625, -1.0, 0.875);

        Buffer y = sq.generate(1.0, 1.0);

        REQUIRE(y.getLength() == 1000);
        CHECK(count(y, 0.0) == 125 + 250 + 125);
        CHECK(count(y, 1.0) == 250);
        CHECK(count(y, -1.0) == 250);
        check_close(y.subbuffer(125, 250), Buffer::ones(250), 0.0);
    }

    SECTION("corners between samples round to the nearest")
    {
        // Corners at samples 12, 46, 50 and 79, the lines used to add up
        // to 99.
        Square sq(100.0, 0.123, 1.0, 0.456, 0.5, -1.0, 0.789);

        Buffer y = sq.generate(1.0, 1.0);

        REQUIRE(y.getLength() == 100);
        CHECK(count(y, 1.0) == 46 - 12);
        CHECK(count(y, -1.0) == 79 - 50);
        CHECK(y[11] == 0.0);
        CHECK(y[12] == 1.0);
        CHECK(y[78] == -1.0);
        CHECK(y[79] == 0.0);
    }

    SECTION("a corner at the start")
    {
        Square sq(100.0, 0.0, 1.0, 0.5, 0.5, -1.0, 1.0);

        check_close(sq.generate(1.0, 1.0), Square(100.0).generate(1.0, 1.0), 0.0);
    }
}

TEST_CASE("Square from harmonics")
{
    SECTION("one harmonic is a sine")
    {
        Buffer y = Square(1000.0, 1).generate(1.0, 1.0);

        CHECK(std::fabs(y.getMax() - 4.0 / M_PI) < 1e-6);
        CHECK(std::fabs(y[250] - 4.0 / M_PI) < 1e-12);
    }

    SECTION("many harmonics approach a square")
    {
        Buffer y = Square(1000.0, 100).generate(1.0, 1.0);

        CHECK(std::fabs(y[250] - 1.0) < 0.01);
        CHECK(std::fabs(y[750] + 1.0) < 0.01);
    }

    SECTION("0 and negative harmonics are one")
    {
        Buffer one = Square(1000.0, 1).generate(1.0, 1.0);

        check_close(Square(1000.0, 0).generate(1.0, 1.0), one, 0.0);
        check_close(Square(1000.0, -1).generate(1.0, 1.0), one, 0.0);
    }
}
