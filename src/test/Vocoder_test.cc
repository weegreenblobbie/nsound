//-----------------------------------------------------------------------------
//
//  Vocoder tests, a carrier shaped by the voice's envelope in each band.
//
//-----------------------------------------------------------------------------

#include <Nsound/Buffer.h>
#include <Nsound/Sine.h>
#include <Nsound/Vocoder.h>

#include "gold_check.h"

#include <catch2/catch_test_macros.hpp>

#include <cmath>
#include <type_traits>

using namespace Nsound;

// It owns its filters.
static_assert(!std::is_copy_constructible_v<Vocoder>);
static_assert(!std::is_copy_assignable_v<Vocoder>);

//! The amplitude of the f Hz part of b's second half.
static
float64
amplitude(const Buffer & b, float64 f, float64 sr)
{
    uint32 start = b.getLength() / 2;
    uint32 n = b.getLength() - start;
    float64 re = 0.0;
    float64 im = 0.0;
    for(uint32 i = 0; i < n; ++i)
    {
        re += b[start + i] * std::cos(2.0 * M_PI * f * i / sr);
        im += b[start + i] * std::sin(2.0 * M_PI * f * i / sr);
    }
    return 2.0 * std::hypot(re, im) / n;
}

TEST_CASE("Vocoder passes the carrier where the voice is")
{
    Sine sine(8000.0);

    // A carrier with a 500 Hz and a 3000 Hz part, a voice at 500 Hz.
    Buffer carrier = sine.generate(1.0, 500.0) + sine.generate(1.0, 3000.0);

    for(bool mel : {false, true})
    {
        INFO("mel " << mel);

        Vocoder v(8000.0, 0.02, 16, 4000.0, mel);

        SECTION("a voice at 500 Hz")
        {
            Buffer y = v.filter(sine.generate(1.0, 500.0), carrier);

            REQUIRE(y.getLength() == 8000);
            CHECK(amplitude(y, 500.0, 8000.0) > 10.0 * amplitude(y, 3000.0, 8000.0));
        }

        SECTION("a voice at 3000 Hz")
        {
            Buffer y = v.filter(sine.generate(1.0, 3000.0), carrier);

            CHECK(amplitude(y, 3000.0, 8000.0) > 10.0 * amplitude(y, 500.0, 8000.0));
        }

        SECTION("no voice, no output")
        {
            Buffer y = v.filter(Buffer::zeros(8000), carrier);

            CHECK(y.getAbs().getMax() == 0.0);
        }
    }
}

TEST_CASE("Vocoder overloads and reset()")
{
    Sine sine(8000.0);

    Buffer voice = sine.generate(0.25, 500.0);
    Buffer carrier = sine.generate(0.25, 520.0);

    Vocoder v(8000.0, 0.02, 8, 4000.0);

    Buffer y = v.filter(voice, carrier);

    v.reset();

    Buffer per_sample;
    for(uint32 i = 0; i < carrier.getLength(); ++i)
    {
        per_sample << v.filter(voice[i], carrier[i]);
    }

    check_close(per_sample, y, 0.0);

    // Without reset() it carries on.
    CHECK((v.filter(voice, carrier) - y).getAbs().getMax() > 0.0);

    // A shorter voice repeats.
    v.reset();

    Buffer doubled;
    doubled << voice << voice;

    Buffer long_carrier;
    long_carrier << carrier << carrier;

    Buffer y2 = v.filter(voice, long_carrier);

    v.reset();

    check_close(y2, v.filter(doubled, long_carrier), 0.0);
}

TEST_CASE("Vocoder checks its arguments")
{
    CHECK_THROWS_AS(Vocoder(0.0, 0.02, 8, 4000.0), Exception);

    // At least a 5 ms window, 4 bands and 5% of the sample rate.
    CHECK_THROWS_AS(Vocoder(8000.0, 0.004, 8, 4000.0), Exception);
    CHECK_THROWS_AS(Vocoder(8000.0, 0.02, 3, 4000.0), Exception);
    CHECK_THROWS_AS(Vocoder(8000.0, 0.02, 8, 399.0), Exception);

    Vocoder v(8000.0, 0.02, 8, 4000.0, false, true);

    CHECK(v.getSampleRate() == 8000.0);
}
