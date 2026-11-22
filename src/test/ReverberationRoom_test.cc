//-----------------------------------------------------------------------------
//
//  ReverberationRoom tests, a Freeverb style room of parallel low pass comb
//  filters into series all pass filters, per channel.
//
//-----------------------------------------------------------------------------

#include <Nsound/AudioStream.h>
#include <Nsound/Buffer.h>
#include <Nsound/ReverberationRoom.h>

#include "gold_check.h"

#include <catch2/catch_test_macros.hpp>

using namespace Nsound;

// The room adds noise of up to 1e-23 to keep the filters out of denormals.
static const float64 NOISE = 1e-20;

static
Buffer
impulse(uint32 n)
{
    Buffer b = Buffer::zeros(n);
    b[0] = 1.0;
    return b;
}

TEST_CASE("ReverberationRoom dry and wet")
{
    Buffer x = impulse(4000);

    SECTION("all dry is the input in both channels")
    {
        ReverberationRoom room(8000.0, 0.5, 0.0, 1.0);

        AudioStream y = room.filter(x);

        REQUIRE(y.getNChannels() == 2);
        check_close(y[0], x, NOISE);
        check_close(y[1], x, NOISE);
    }

    SECTION("wet reverberates and decays")
    {
        ReverberationRoom room(8000.0, 0.5, 1.0, 0.0);

        Buffer y = room.filter(x)[0];

        // Nothing until the shortest comb delay, 1116 / 44100 seconds.
        CHECK(y.subbuffer(0, 200).getAbs().getMax() < NOISE);

        Buffer early = y.subbuffer(200, 1000);
        Buffer late = y.subbuffer(3000, 1000);

        early *= early;
        late *= late;

        CHECK(early.getSum() > 0.0);
        CHECK(late.getSum() < early.getSum());
    }

    SECTION("wet and dry are limited to [0, 1]")
    {
        ReverberationRoom limited(8000.0, 0.5, 2.0, -1.0);
        ReverberationRoom wet(8000.0, 0.5, 1.0, 0.0);

        check_close(limited.filter(x)[0], wet.filter(x)[0], NOISE);

        ReverberationRoom limited2(8000.0, 0.5, -1.0, 2.0);
        ReverberationRoom dry(8000.0, 0.5, 0.0, 1.0);

        check_close(limited2.filter(x)[0], dry.filter(x)[0], NOISE);
    }
}

TEST_CASE("ReverberationRoom left and right")
{
    Buffer x = impulse(4000);

    SECTION("with no spread, the same input gives the same reverb")
    {
        ReverberationRoom room(8000.0, 0.5, 1.0, 0.0, 100.0, 0.0);

        AudioStream y = room.filter(x);

        check_close(y[1], y[0], NOISE);
    }

    SECTION("each channel reverberates its own input")
    {
        // The right channel used to be the left one's reverb.
        ReverberationRoom room(8000.0, 0.5, 1.0, 0.0, 100.0, 0.0);

        AudioStream right_only(8000.0, 2);
        right_only[0] = Buffer::zeros(4000);
        right_only[1] = x;

        AudioStream y = room.filter(right_only);

        CHECK(y[0].getAbs().getMax() < NOISE);
        CHECK(y[1].getAbs().getMax() > 0.01);

        // The same as the left channel's reverb of it.
        check_close(y[1], room.filter(x)[0], NOISE);
    }

    SECTION("one channel is filtered like a Buffer")
    {
        ReverberationRoom room(8000.0, 0.5, 1.0, 0.5);

        AudioStream mono(8000.0, 1);
        mono << x;

        AudioStream y = room.filter(mono);
        AudioStream expected = room.filter(x);

        check_close(y[0], expected[0], NOISE);
        check_close(y[1], expected[1], NOISE);
    }
}

TEST_CASE("ReverberationRoom copies")
{
    ReverberationRoom room(8000.0, 0.6, 0.7, 0.3, 200.0);

    Buffer x = impulse(3000);

    AudioStream expected = room.filter(x);

    SECTION("the copy constructor")
    {
        // It crashed, assigning to filters it hadn't made.
        ReverberationRoom copy(room);

        AudioStream y = copy.filter(x);

        CHECK(copy.getSampleRate() == 8000.0);
        check_close(y[0], expected[0], NOISE);
        check_close(y[1], expected[1], NOISE);
    }

    SECTION("operator=")
    {
        ReverberationRoom other(8000.0, 0.1, 0.2, 0.8, 1000.0);

        other = room;

        ReverberationRoom & same = other;
        other = same;

        AudioStream y = other.filter(x);

        check_close(y[0], expected[0], NOISE);
        check_close(y[1], expected[1], NOISE);
    }
}

TEST_CASE("ReverberationRoom a negative low pass or spread is 0")
{
    Buffer x = impulse(3000);

    ReverberationRoom limited(8000.0, 0.5, 1.0, 0.0, -100.0, -1.0);
    ReverberationRoom zero(8000.0, 0.5, 1.0, 0.0, 0.0, 0.0);

    AudioStream y = limited.filter(x);
    AudioStream expected = zero.filter(x);

    check_close(y[0], expected[0], NOISE);
    check_close(y[1], expected[1], NOISE);
}
