//-----------------------------------------------------------------------------
//
//  Tests of the physical model and drum instruments: Clarinet, DrumBD01,
//  DrumKickBass, FluteSlide and GuitarBass.
//
//-----------------------------------------------------------------------------

#include <Nsound/AudioStream.h>
#include <Nsound/Buffer.h>
#include <Nsound/Clarinet.h>
#include <Nsound/DrumBD01.h>
#include <Nsound/DrumKickBass.h>
#include <Nsound/FluteSlide.h>
#include <Nsound/GuitarBass.h>

#include "gold_check.h"

#include <catch2/catch_test_macros.hpp>

#include <cmath>

using namespace Nsound;

static const float64 SR = 8000.0;

//! Checks a note is mono, as long as asked, finite and louder than loudest.
static
void
check_note(const AudioStream & y, float64 duration, float64 loudest = 0.1)
{
    REQUIRE(y.getNChannels() == 1);
    CHECK(std::fabs(y.getLength() - duration * SR) <= 1.0);

    for(uint32 i = 0; i < y.getLength(); ++i)
    {
        if(!std::isfinite(y[0][i])) FAIL("sample " << i << " is " << y[0][i]);
    }

    CHECK(y.getAbs().getMax() > loudest);
}

TEST_CASE("Clarinet")
{
    Clarinet c(SR);

    SECTION("a note as long as asked")
    {
        // It was a period short, 3963 of 4000 samples.
        check_note(c.play(0.5, 220.0), 0.5);
        check_note(c.play(0.25, 440.0), 0.25);
    }

    SECTION("pressure, filter and reed stiffness")
    {
        for(float64 pressure : {0.0, 0.5, 1.0})
        {
            // A stiffness of 2 or -2 read past the reed table.
            for(float64 stiffness : {-2.0, -0.5, -0.3, 0.0, 0.5, 2.0})
            {
                INFO("pressure " << pressure << ", stiffness " << stiffness);
                // Some of these are quiet once the first period is dropped.
                check_note(c.play(0.2, 220.0, pressure, 2000.0, stiffness), 0.2, 0.0);
            }
        }
    }

    SECTION("the tune")
    {
        CHECK(c.play().getLength() > 0);
        CHECK_FALSE(c.getInfo().empty());
    }
}

TEST_CASE("DrumBD01")
{
    DrumBD01 d(SR);

    check_note(d.play(0.5, 60.0), 0.5);
    check_note(d.play(0.5, 60.0, 0.5), 0.5);

    // Softer is quieter before normalizing? Both are normalized, so the
    // velocity only shapes the sound.
    CHECK(d.play().getLength() > 0);
    CHECK_FALSE(d.getInfo().empty());
}

TEST_CASE("DrumKickBass")
{
    DrumKickBass d(SR, 266.0, 0.0);

    check_note(d.play(0.5, 266.0), 0.5);
    check_note(d.play(0.5, 266.0, 50.0), 0.5);
    check_note(d.play(0.5, Buffer({266.0, 200.0}), Buffer({50.0})), 0.5);

    CHECK(d.play().getLength() > 0);
    CHECK_FALSE(d.getInfo().empty());

    SECTION("a kick dies away, peaking at 0.98")
    {
        Buffer kick = d.play(0.6, 120.0, 51.0)[0];

        CHECK(std::fabs(kick.getAbs().getMax() - 0.98) < 1e-12);

        uint32 n = kick.getLength();

        auto rms = [&](uint32 start, uint32 length)
        {
            Buffer part = kick.subbuffer(start, length);
            part *= part;
            return std::sqrt(part.getMean());
        };

        // atan() saturates its start and lifts its tail, a tone stays level.
        CHECK(rms(n - n / 10, n / 10) < 0.2 * rms(0, n / 4));
    }
}

TEST_CASE("FluteSlide")
{
    FluteSlide f(SR);

    check_note(f.play(0.5, 440.0), 0.5);
    check_note(f.play(0.5, 440.0, 0.9, 0.036), 0.5);

    CHECK(f.play().getLength() > 0);
    CHECK_FALSE(f.getInfo().empty());
}

TEST_CASE("GuitarBass")
{
    GuitarBass g(SR);

    check_note(g.play(0.5, 110.0), 0.5);

    CHECK(g.play().getLength() > 0);
    CHECK_FALSE(g.getInfo().empty());
}
