//-----------------------------------------------------------------------------
//
//  EnvelopeAdsr tests, attack, delay, sustain and release.
//
//-----------------------------------------------------------------------------

#include <Nsound/AudioStream.h>
#include <Nsound/Buffer.h>
#include <Nsound/EnvelopeAdsr.h>

#include "gold_check.h"

#include <catch2/catch_test_macros.hpp>

#include <cmath>

using namespace Nsound;

TEST_CASE("EnvelopeAdsr shape() of a Buffer")
{
    // 10 samples of attack, 10 of delay to 0.5, 20 of release at 100 Hz.
    EnvelopeAdsr e(100.0, 0.1, 0.1, 0.5, 0.2);

    Buffer y = e.shape(Buffer::ones(100));

    SECTION("as long as the input")
    {
        // It stopped at the end of the release, 85 samples.
        CHECK(y.getLength() == 100);
    }

    SECTION("each phase lasts its time")
    {
        // The attack rises a tenth a sample to the peak.
        CHECK(std::fabs(y[0] - 0.1) < 1e-12);
        CHECK(std::fabs(y[4] - 0.5) < 1e-12);
        CHECK(y[9] > 0.99);

        // The delay falls to the sustain level in its 10 samples, it fell
        // at the full scale rate, in 5.
        CHECK(y[14] > 0.7);
        CHECK(std::fabs(y[20] - 0.5) < 1e-12);

        // Sustain until the release.
        check_close(y.subbuffer(20, 60), 0.5 * Buffer::ones(60), 1e-12);

        // The release starts after 80 samples and ends with the input, it
        // ended in 10 samples.
        CHECK(std::fabs(y[80] - 0.475) < 1e-12);
        CHECK(y[89] > 0.2);
        CHECK(y[99] == 0.0);
    }

    SECTION("an input shorter than the phases is cut short")
    {
        // The attack and delay, and half the release.
        Buffer short_one = e.shape(Buffer::ones(30));

        REQUIRE(short_one.getLength() == 30);
        CHECK(std::fabs(short_one[29] - 0.25) < 1e-12);
    }
}

TEST_CASE("EnvelopeAdsr no time is instant")
{
    SECTION("no attack starts at the peak")
    {
        // It was silent: the sample count wrapped round.
        EnvelopeAdsr e(100.0, 0.0, 0.1, 0.5, 0.2);

        Buffer y = e.shape(Buffer::ones(100));

        CHECK(y[0] > 0.99);
    }

    SECTION("a sample of attack")
    {
        // It threw.
        EnvelopeAdsr e(100.0, 0.01, 0.1, 0.5, 0.2);

        CHECK(e.shape(Buffer::ones(100))[0] > 0.99);
    }

    SECTION("no delay goes to the sustain level")
    {
        EnvelopeAdsr e(100.0, 0.1, 0.0, 0.5, 0.2);

        Buffer y = e.shape(Buffer::ones(100));

        CHECK(std::fabs(y[11] - 0.5) < 1e-12);
    }
}

TEST_CASE("EnvelopeAdsr one sample at a time")
{
    EnvelopeAdsr e(100.0, 0.1, 0.1, 0.5, 0.2);

    // Hold the key down past the delay, then let go.
    Buffer y;
    for(uint32 i = 0; i < 30; ++i) y << e.shape(1.0, true);

    CHECK(std::fabs(y[29] - 0.5) < 1e-12);
    CHECK_FALSE(e.is_done());

    for(uint32 i = 0; i < 25; ++i) y << e.shape(1.0, false);

    CHECK(e.is_done());
    CHECK(y[54] == 0.0);

    SECTION("letting go during the attack releases")
    {
        e.reset();

        e.shape(1.0, true);
        float64 first = e.shape(1.0, false);

        CHECK(first > 0.0);
        CHECK(e.shape(1.0, false) < first);
    }
}

TEST_CASE("EnvelopeAdsr setters")
{
    EnvelopeAdsr e(100.0, 0.1, 0.1, 0.5, 0.2);

    e.setAttackTime(0.2);
    e.setSustainAmplitude(0.25);
    e.setDelayTime(0.05);
    e.setReleaseTime(0.1);

    Buffer y = e.shape(Buffer::ones(100));

    CHECK(std::fabs(y[19] - 1.0) < 0.01);
    CHECK(std::fabs(y[30] - 0.25) < 1e-12);
    CHECK(y[99] < 0.03);

    CHECK_THROWS_AS(e.setAttackTime(-1.0), Exception);
    CHECK_THROWS_AS(e.setSustainAmplitude(0.0), Exception);
    CHECK_THROWS_AS(e.setSustainAmplitude(1.5), Exception);
}

TEST_CASE("EnvelopeAdsr shape() of an AudioStream and its checks")
{
    EnvelopeAdsr e(100.0, 0.1, 0.1, 0.5, 0.2);

    AudioStream as(100.0, 2);
    as << Buffer::ones(100);

    AudioStream y = e.shape(as);
    Buffer expected = e.shape(Buffer::ones(100));

    check_close(y[0], expected, 0.0);
    check_close(y[1], expected, 0.0);

    CHECK_THROWS_AS(EnvelopeAdsr(0.0, 0.1, 0.1, 0.5, 0.2), Exception);
    CHECK_THROWS_AS(EnvelopeAdsr(100.0, 0.0, 0.0, 0.5, 0.0), Exception);
    CHECK_THROWS_AS(EnvelopeAdsr(100.0, 0.1, 0.1, 0.0, 0.2), Exception);
}
