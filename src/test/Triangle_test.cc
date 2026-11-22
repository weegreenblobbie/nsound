//-----------------------------------------------------------------------------
//
//  Triangle tests, compared with the gold files.
//
//-----------------------------------------------------------------------------

#include <Nsound/Buffer.h>
#include <Nsound/Triangle.h>
#include <Nsound/Wavefile.h>

#include "gold_check.h"

#include <catch2/catch_test_macros.hpp>

using namespace Nsound;

static const float64 GAMMA = 1.5e-14;

TEST_CASE("Triangle")
{
    Wavefile::setDefaultSampleSize(64);
    Wavefile::setIEEEFloat(true);

    Triangle tri(100);

    SECTION("generate(1.01, 2.0)")
    {
        check_gold(tri.generate(1.01, 2.0), "gold/Triangle_2Hz.wav", GAMMA);
    }

    SECTION("generate(1.01, 1.0)")
    {
        check_gold(tri.generate(1.01, 1.0), "gold/Triangle_1.0Hz.wav", GAMMA);
    }

    SECTION("generate(1.01, frequencies)")
    {
        Buffer freqs = tri.drawLine(1.01, 1.0, 5.0);

        check_gold(tri.generate(1.01, freqs), "gold/Triangle_1_to_5Hz.wav", GAMMA);
    }
}

TEST_CASE("Triangle with an attack and release")
{
    SECTION("rises, falls through the middle and comes back to 0")
    {
        Triangle tri(100.0, 0.25, 0.25);

        Buffer y = tri.generate(1.0, 1.0);

        REQUIRE(y.getLength() == 100);
        CHECK(y[0] == 0.0);
        CHECK(y[25] == 1.0);
        CHECK(y[75] == -1.0);
        CHECK(y.getMax() == 1.0);
        CHECK(y.getMin() == -1.0);
    }

    SECTION("times between samples")
    {
        Triangle tri(100.0, 0.123, 0.456);

        CHECK(tri.generate(1.0, 1.0).getLength() == 100);
    }

    SECTION("the times are checked")
    {
        CHECK_THROWS_AS(Triangle(100.0, -0.1, 0.1), Exception);
        CHECK_THROWS_AS(Triangle(100.0, 0.1, -0.1), Exception);
        CHECK_THROWS_AS(Triangle(100.0, 0.5, 0.5), Exception);
    }
}

TEST_CASE("Triangle drawn from its corners")
{
    SECTION("whole samples")
    {
        Triangle tri(100.0, 0.125, 0.25, 1.0, 0.375, 0.625, 0.75, -1.0, 0.875);

        Buffer y = tri.generate(1.0, 1.0);

        // The lines used to add up to 103 samples.
        REQUIRE(y.getLength() == 100);

        // 0 until the rise, up to the peak at a quarter, 0 again until the
        // fall, down to the trough at three quarters and back to 0.
        check_close(y.subbuffer(0, 13), Buffer::zeros(13), 0.0);
        CHECK(y[13] == 0.0);
        CHECK(y[25] == 1.0);
        check_close(y.subbuffer(38, 25), Buffer::zeros(25), 0.0);
        CHECK(y[75] == -1.0);
        check_close(y.subbuffer(88), Buffer::zeros(12), 0.0);
    }

    SECTION("corners between samples")
    {
        Triangle tri(100.0, 0.11, 0.22, 1.0, 0.33, 0.55, 0.66, -1.0, 0.77);

        CHECK(tri.generate(1.0, 1.0).getLength() == 100);
    }
}
