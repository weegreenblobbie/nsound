//-----------------------------------------------------------------------------
//
//  Mixer tests, AudioStreams placed on beats.
//
//-----------------------------------------------------------------------------

#include <Nsound/AudioStream.h>
#include <Nsound/Buffer.h>
#include <Nsound/Mixer.h>
#include <Nsound/MixerNode.h>

#include "gold_check.h"

#include <catch2/catch_test_macros.hpp>

using namespace Nsound;

// At 10 Hz, so times are tenths of a second.
static const float64 SR = 10.0;

//! A mono AudioStream of the samples.
static
AudioStream
stream(const Buffer & b)
{
    AudioStream as(SR, 1);
    as << b;
    return as;
}

TEST_CASE("Mixer places a stream once")
{
    // A beats per minute of 0 plays it once.
    Mixer mixer;
    mixer.add(0.3, 0.0, stream(Buffer({1, 2, 3})));

    CHECK(mixer.size() == 1);

    AudioStream y = mixer.getStream(1.0);

    REQUIRE(y.getLength() == 10);
    check_close(y[0], Buffer({0, 0, 0, 1, 2, 3, 0, 0, 0, 0}), 0.0);
}

TEST_CASE("Mixer repeats a stream on its beats")
{
    // 120 beats a minute is every half second.
    Mixer mixer;
    mixer.add(0.0, 120.0, stream(Buffer({1, 2})));

    AudioStream y = mixer.getStream(2.0);

    check_close(
        y[0],
        Buffer({1, 2, 0, 0, 0, 1, 2, 0, 0, 0, 1, 2, 0, 0, 0, 1, 2, 0, 0, 0}),
        0.0);

    SECTION("starting inside a beat plays the rest of it")
    {
        // It played the wrong part of the beat.
        Mixer m2;
        m2.add(0.0, 0.0, stream(Buffer({1, 2, 3, 4, 5})));

        check_close(m2.getStream(0.2, 1.0)[0], Buffer({3, 4, 5, 0, 0, 0, 0, 0}), 0.0);
    }

    SECTION("a window later on")
    {
        check_close(mixer.getStream(0.5, 1.5)[0], Buffer({1, 2, 0, 0, 0, 1, 2, 0, 0, 0}), 0.0);
    }

    SECTION("a beat cut off at the end")
    {
        check_close(mixer.getStream(0.6)[0], Buffer({1, 2, 0, 0, 0, 1}), 0.0);
    }
}

TEST_CASE("Mixer mixes several streams")
{
    // Added out of time order, so the nodes' times and ids disagree; the
    // ordering said each was before the other.
    Mixer mixer;
    mixer.add(0.4, 0.0, stream(Buffer({10.0})));
    mixer.add(0.0, 0.0, stream(Buffer({1.0})));
    mixer.add(0.2, 0.0, stream(Buffer({100.0})));
    mixer.add(0.0, 0.0, stream(Buffer({2.0})));

    REQUIRE(mixer.size() == 4);

    check_close(mixer.getStream(0.5)[0], Buffer({3, 0, 100, 0, 10}), 0.0);

    SECTION("as many channels as the widest")
    {
        AudioStream stereo(SR, 2);
        stereo[0] << 5.0;
        stereo[1] << 7.0;

        mixer.add(0.1, 0.0, stereo);

        AudioStream y = mixer.getStream(0.5);

        REQUIRE(y.getNChannels() == 2);
        check_close(y[0], Buffer({3, 5, 100, 0, 10}), 0.0);
        check_close(y[1], Buffer({0, 7, 0, 0, 0}), 0.0);
    }

    SECTION("clear()")
    {
        mixer.clear();

        CHECK(mixer.size() == 0);
        CHECK_THROWS_AS(mixer.getStream(1.0), Exception);
    }
}

TEST_CASE("Mixer keeps its own copy of the streams")
{
    // It kept a pointer to the stream, here a temporary that is gone.
    Mixer mixer;
    mixer.add(0.0, 0.0, stream(Buffer({1, 2, 3})));

    check_close(mixer.getStream(0.3)[0], Buffer({1, 2, 3}), 0.0);
}

TEST_CASE("Mixer checks its times")
{
    Mixer mixer;

    CHECK_THROWS_AS(mixer.add(-1.0, 60.0, stream(Buffer({1.0}))), Exception);

    mixer.add(0.0, -60.0, stream(Buffer({1.0, 2.0})));

    // A negative beats per minute plays once.
    check_close(mixer.getStream(0.4)[0], Buffer({1, 2, 0, 0}), 0.0);

    CHECK_THROWS_AS(mixer.getStream(1.0, 0.5), Exception);
}

TEST_CASE("MixerNode")
{
    AudioStream as = stream(Buffer({1, 2, 3}));

    MixerNode a(0.5, 60.0, as);
    MixerNode b(0.5, 60.0, as);

    CHECK(a == a);
    CHECK_FALSE(a == b);

    // Same time, so made first is first.
    CHECK(a < b);
    CHECK_FALSE(b < a);

    CHECK(a.printNode().find("0.3 seconds") != std::string::npos);
}
