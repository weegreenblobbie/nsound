//-----------------------------------------------------------------------------
//
//  DelayLine tests, compared with the gold files.
//
//-----------------------------------------------------------------------------

#include <Nsound/Buffer.h>
#include <Nsound/DelayLine.h>
#include <Nsound/Sine.h>
#include <Nsound/Wavefile.h>

#include "gold_check.h"

#include <catch2/catch_test_macros.hpp>

using namespace Nsound;

static const float64 GAMMA = 1.5e-14;

TEST_CASE("DelayLine write() and read()")
{
    Wavefile::setDefaultSampleSize(64);
    Wavefile::setIEEEFloat(true);

    DelayLine dl(1.0, 10.0);
    Sine sine(100.0);

    Buffer input = sine.generate(1.0, 2.0) * sine.drawLine(1.0, 1.0, 0.0);
    Buffer data;

    for(uint32 i = 0; i < 100; ++i)
    {
        dl.write(input[i]);
        data << dl.read();
    }

    check_gold(data, "gold/DelayLine_out1.wav", GAMMA);
}

//! An impulse n samples long.
static
Buffer
impulse(uint32 n)
{
    Buffer b = Buffer::zeros(n);
    b[0] = 1.0;
    return b;
}

TEST_CASE("DelayLine delay() by whole samples")
{
    // 10 Hz, so 0.3 seconds is 3 samples.
    DelayLine dl(10.0, 1.0);

    for(uint32 d : {1u, 3u, 10u})
    {
        INFO(d << " samples");

        // It was a sample short.
        Buffer expected = Buffer::zeros(14);
        expected[d] = 1.0;

        Buffer y;
        dl.reset();
        for(uint32 i = 0; i < 14; ++i) y << dl.delay(impulse(14)[i], d / 10.0);

        check_close(y, expected, 0.0);

        // The Buffer overload starts from silence.
        check_close(dl.delay(impulse(14), (d / 10.0) * Buffer::ones(14)), expected, 0.0);
    }

    SECTION("a delay of less than half a sample is a sample")
    {
        // It read the oldest sample, the whole delay line.
        Buffer expected = Buffer::zeros(14);
        expected[1] = 1.0;

        check_close(dl.delay(impulse(14), 0.01 * Buffer::ones(14)), expected, 0.0);
    }

    SECTION("a delay is limited to the maximum")
    {
        Buffer expected = Buffer::zeros(14);
        expected[10] = 1.0;

        check_close(dl.delay(impulse(14), 5.0 * Buffer::ones(14)), expected, 0.0);
    }

    SECTION("in real time it carries on")
    {
        dl.setRealtime(true);
        dl.reset();

        Buffer first = dl.delay(impulse(2), 0.3 * Buffer::ones(2));
        Buffer second = dl.delay(Buffer::zeros(3), 0.3 * Buffer::ones(3));

        check_close(first, Buffer({0, 0}), 0.0);
        check_close(second, Buffer({0, 1, 0}), 0.0);
    }

    SECTION("a delay must be more than 0")
    {
        CHECK_THROWS_AS(dl.read(0.0), Exception);
        CHECK_THROWS_AS(DelayLine(0.0, 1.0), Exception);
        CHECK_THROWS_AS(DelayLine(10.0, 0.0), Exception);
    }
}
