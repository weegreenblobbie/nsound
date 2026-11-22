//-----------------------------------------------------------------------------
//
//  Hat tests, a hi-hat cymbal from six high passed square waves.
//
//-----------------------------------------------------------------------------

#include <Nsound/AudioStream.h>
#include <Nsound/Buffer.h>
#include <Nsound/Hat.h>

#include "gold_check.h"

#include <catch2/catch_test_macros.hpp>

#include <cmath>

using namespace Nsound;

// The high pass is at 4872 Hz, so a sample rate well above twice that.
static const float64 SR = 44100.0;

TEST_CASE("Hat play() a hit")
{
    Hat hat(SR);

    SECTION("closed rings for 6% of the time")
    {
        AudioStream y = hat.play(0.5, 1.0, true);

        // It returned only the 6% that rings.
        REQUIRE(std::fabs(y.getLength() - 0.5 * SR) <= 2.0);

        uint32 ringing = static_cast<uint32>(0.03 * SR);

        CHECK(y[0].subbuffer(0, ringing).getAbs().getMax() > 0.1);
        CHECK(y[0].subbuffer(ringing + 1).getAbs().getMax() == 0.0);
    }

    SECTION("open rings for 45% of the time")
    {
        AudioStream y = hat.play(0.5, 1.0, false);

        REQUIRE(std::fabs(y.getLength() - 0.5 * SR) <= 2.0);

        uint32 ringing = static_cast<uint32>(0.225 * SR);

        CHECK(y[0].subbuffer(ringing / 2, ringing / 2).getAbs().getMax() > 0.01);
        CHECK(y[0].subbuffer(ringing + 1).getAbs().getMax() == 0.0);
    }

    SECTION("play(duration, tune) is closed")
    {
        check_close(hat.play(0.5, 1.0)[0], hat.play(0.5, 1.0, true)[0], 0.0);
    }

    SECTION("tuning changes it")
    {
        CHECK((hat.play(0.1, 0.5, true)[0] - hat.play(0.1, 1.0, true)[0]).getAbs().getMax() > 0.01);
    }
}

TEST_CASE("Hat play() the pattern")
{
    Hat hat(SR);

    AudioStream y = hat.play();

    // 31 hits 0.124 seconds apart, the last one a second long.
    CHECK(std::fabs(y.getDuration() - (31 * 0.124 + 1.0)) < 0.01);
    CHECK(y.getAbs().getMax() > 0.1);

    CHECK_FALSE(hat.getInfo().empty());
}

TEST_CASE("Hat copies")
{
    Hat hat(SR);

    Buffer expected = hat.play(0.1, 1.0, false)[0];

    Hat copy(hat);
    check_close(copy.play(0.1, 1.0, false)[0], expected, 0.0);

    Hat other(48000.0);
    other = hat;
    check_close(other.play(0.1, 1.0, false)[0], expected, 0.0);

    // It deleted its parts and then copied them.
    Hat & same = other;
    other = same;
    check_close(other.play(0.1, 1.0, false)[0], expected, 0.0);
}
