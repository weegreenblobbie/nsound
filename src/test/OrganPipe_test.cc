//-----------------------------------------------------------------------------
//
//  OrganPipe tests, an FM organ pipe.
//
//-----------------------------------------------------------------------------

#include <Nsound/AudioStream.h>
#include <Nsound/Buffer.h>
#include <Nsound/OrganPipe.h>

#include "gold_check.h"

#include <catch2/catch_test_macros.hpp>

#include <cmath>

using namespace Nsound;

//! The amplitude of the f Hz part of b.
static
float64
amplitude(const Buffer & b, float64 f, float64 sr)
{
    float64 re = 0.0;
    float64 im = 0.0;
    for(uint32 i = 0; i < b.getLength(); ++i)
    {
        re += b[i] * std::cos(2.0 * M_PI * f * i / sr);
        im += b[i] * std::sin(2.0 * M_PI * f * i / sr);
    }
    return 2.0 * std::hypot(re, im) / b.getLength();
}

TEST_CASE("OrganPipe play() a note")
{
    OrganPipe pipe(8000.0);

    AudioStream y = pipe.play(0.5, 220.0);

    SECTION("stereo, as long as the note")
    {
        REQUIRE(y.getNChannels() == 2);
        CHECK(std::fabs(y.getLength() - 4000.0) <= 2.0);
    }

    SECTION("centered by default")
    {
        check_close(y[1], y[0], 0.0);
    }

    SECTION("panned")
    {
        AudioStream panned = pipe.play(0.5, 220.0, 0.9);

        check_close(panned[0], 1.8 * y[0], 1e-12);
        check_close(panned[1], 0.2 * y[0], 1e-12);
    }

    SECTION("the note's frequency is in it")
    {
        Buffer left = y[0];

        CHECK(amplitude(left, 220.0, 8000.0) > 5.0 * amplitude(left, 330.0, 8000.0));
    }

    SECTION("it starts and ends at 0, without a click")
    {
        CHECK(y[0][0] == 0.0);
        CHECK(std::fabs(y[0][y.getLength() - 1]) < 0.01);
    }
}

TEST_CASE("OrganPipe short notes")
{
    OrganPipe pipe(8000.0);

    // Each envelope has a shorter shape below its own length.
    for(float64 d : {0.001, 0.01, 0.05, 0.1, 0.2, 0.3})
    {
        INFO("duration " << d);

        AudioStream y = pipe.play(d, 220.0);

        CHECK(std::fabs(y.getLength() - d * 8000.0) <= 2.0);
        CHECK(y[0].getAbs().getMax() > 0.0);
    }

    CHECK_THROWS_AS(pipe.play(0.0, 220.0), Exception);
}

TEST_CASE("OrganPipe play() the Toccata")
{
    OrganPipe pipe(2000.0);

    AudioStream y = pipe.play();

    // The last notes end at 15.9 seconds, then 0.3 seconds of silence.
    CHECK(std::fabs(y.getDuration() - 16.2) < 0.1);
    CHECK(std::fabs(y.getAbs().getMax() - 1.0) < 1e-12);

    CHECK_FALSE(pipe.getInfo().empty());
}
