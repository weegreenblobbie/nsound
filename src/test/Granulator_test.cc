//-----------------------------------------------------------------------------
//
//  Granulator tests, enveloped sine grains at a rate.
//
//-----------------------------------------------------------------------------

#include <Nsound/Buffer.h>
#include <Nsound/Granulator.h>
#include <Nsound/Sine.h>

#include "gold_check.h"

#include <catch2/catch_test_macros.hpp>

#include <cmath>

using namespace Nsound;

//! The sample indices where b turns non-zero after at least gap zeros.
static
Buffer
onsets(const Buffer & b, uint32 gap)
{
    Buffer starts;
    uint32 zeros = gap;
    for(uint32 i = 0; i < b.getLength(); ++i)
    {
        if(b[i] == 0.0)
        {
            ++zeros;
            continue;
        }
        if(zeros >= gap) starts << i;
        zeros = 0;
    }
    return starts;
}

TEST_CASE("Granulator grains")
{
    // 2 waves of 100 Hz, 20 ms grains, 10 a second.
    Granulator g(1000.0, Granulator::GAUSSIAN);

    Buffer y = g.generate(1.0, 100.0, 2.0, 10.0);

    SECTION("a grain every 100 ms")
    {
        Buffer starts = onsets(y, 50);

        REQUIRE(starts.getLength() == 10);

        for(uint32 k = 0; k < 10; ++k)
        {
            INFO("grain " << k);
            CHECK(std::fabs(starts[k] - 100.0 * k) <= 2.0);
        }
    }

    SECTION("each grain is the sine under the envelope")
    {
        Sine sine(1000.0);
        Buffer wave = sine.generate(0.02, 100.0);

        // The Gaussian peaks in the middle of the grain, so it is the sine
        // there, 0 where the sine is.
        Buffer grain = y.subbuffer(0, 20);

        CHECK(grain.getAbs().getMax() <= 1.0);
        CHECK(std::fabs(grain[8]) > 0.5);
        CHECK(std::fabs(grain[5]) < 1e-9);
        CHECK((grain * wave).getSum() > 0.0);
    }

    SECTION("the Buffer overload with constant values agrees")
    {
        Buffer y2 = g.generate(
            1.0,
            Buffer({100.0}),
            Buffer({2.0}),
            10.0 * Buffer::ones(1000));

        check_close(y2.subbuffer(0, 1000), y.subbuffer(0, 1000), 1e-12);
    }
}

TEST_CASE("Granulator envelopes")
{
    Sine sine(1000.0);

    for(auto type : {
        Granulator::GAUSSIAN,
        Granulator::GAUSSIAN_90,
        Granulator::GAUSSIAN_70,
        Granulator::GAUSSIAN_50,
        Granulator::GAUSSIAN_30,
        Granulator::GAUSSIAN_10,
        Granulator::DECAY,
        Granulator::REVERSE_DECAY})
    {
        INFO("envelope " << type);

        Granulator g(1000.0, type);

        Buffer y = g.generate(0.5, 100.0, 2.0, 10.0);

        CHECK(y.getAbs().getMax() > 0.1);
        CHECK(y.getAbs().getMax() <= 1.0 + 1e-12);
    }

    SECTION("a custom envelope of ones leaves the sine")
    {
        Buffer ones = Buffer::ones(1000);

        Granulator g(1000.0, Granulator::CUSTOM, 0.0, &ones);

        Buffer y = g.generate(0.1, 100.0, 2.0, 10.0);

        check_close(y.subbuffer(0, 20), sine.generate(0.02, 100.0), 1e-12);
    }

    SECTION("a custom envelope must be a second long")
    {
        Buffer short_one = Buffer::ones(500);

        CHECK_THROWS_AS(
            Granulator(1000.0, Granulator::CUSTOM, 0.0, &short_one), Exception);

        // It crashed reading the length of no envelope for the message.
        CHECK_THROWS_AS(
            Granulator(1000.0, Granulator::CUSTOM, 0.0, nullptr), Exception);
    }
}

TEST_CASE("Granulator copies")
{
    Granulator g(1000.0, Granulator::GAUSSIAN_50);

    Buffer expected = g.generate(0.5, 100.0, 2.0, 10.0);

    Granulator copy(g);
    check_close(copy.generate(0.5, 100.0, 2.0, 10.0), expected, 0.0);

    Granulator other(1000.0, Granulator::DECAY);
    other = g;
    check_close(other.generate(0.5, 100.0, 2.0, 10.0), expected, 0.0);

    // It deleted its envelope and then copied it.
    Granulator & same = other;
    other = same;
    check_close(other.generate(0.5, 100.0, 2.0, 10.0), expected, 0.0);
}
