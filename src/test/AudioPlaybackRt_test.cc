//-----------------------------------------------------------------------------
//
//  AudioPlaybackRt tests, it plays in real time through portaudio into the
//  capture file of playback_capture.h, the tests check the samples that
//  arrive.  The capture device plays as fast as it is given samples.
//
//-----------------------------------------------------------------------------

#include <Nsound/AudioPlaybackRt.h>
#include <Nsound/AudioStream.h>
#include <Nsound/Buffer.h>

#include "playback_capture.h"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <string>
#include <thread>
#include <vector>

using namespace Nsound;

using playback_capture::captured;

//! Skips the test where playback can't be captured.
#define START_CAPTURE()                                                    \
    if(! playback_capture::start())                                        \
    {                                                                      \
        SKIP("Playback is captured through portaudio and ALSA, on Linux"); \
    }

namespace
{

const float64 SR = 8000.0;

// 10 ms buffers of 80 frames.
const float64 BUFFER_SECONDS = 0.01;
const uint32 N = 80;

//! Plays silence until playing starts, then waits for the buffers waiting to
//! be played.
void
play_until_played(AudioPlaybackRt & pb, uint32 channels)
{
    for(uint32 i = 0; i < 1000 * N && ! pb.get_debug_info().is_streaming; ++i)
    {
        if(channels == 2) pb.play(0.0, 0.0);
        else pb.play(0.0);
    }

    REQUIRE(pb.get_debug_info().is_streaming);

    for(int i = 0; i < 200 && pb.get_debug_info().n_ready > 0; ++i)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    CHECK(pb.get_debug_info().n_ready == 0);
}

//! Returns the captured samples after portaudio's leading silence, as many
//! as expected has.
std::vector<int16_t>
after_silence(const std::vector<int16_t> & expected)
{
    std::vector<int16_t> samples = captured<int16_t>();

    std::size_t start = 0;
    while(start < samples.size() && samples[start] == 0) ++start;

    REQUIRE(samples.size() >= start + expected.size());

    return std::vector<int16_t>(
        samples.begin() + start,
        samples.begin() + start + expected.size());
}

//! Returns the captured samples that aren't 0: the samples played without
//! portaudio's leading silence and the underrun silence the capture device
//! adds between buffers when it plays faster than they are written.
std::vector<int16_t>
not_silence()
{
    std::vector<int16_t> samples;

    for(int16_t x : captured<int16_t>())
    {
        if(x != 0) samples.push_back(x);
    }

    return samples;
}

//! Returns distinct samples, none 0, from -0.78 to 0.77.
Buffer
ramp(uint32 n)
{
    Buffer b;

    for(uint32 i = 0; i < n; ++i)
    {
        b << ((i % 199) - 99.5) / 128.0;
    }

    return b;
}

//! The 16 bit samples AudioPlaybackRt plays, truncated toward 0.
std::vector<int16_t>
to_int16(const Buffer & b)
{
    std::vector<int16_t> out;

    for(float64 x : b) out.push_back(static_cast<int16_t>(32767.0 * x));

    return out;
}

} // namespace

TEST_CASE("AudioPlaybackRt plays every sample, in order")
{
    START_CAPTURE();

    // More samples than the 4 buffers hold, so writing waits for playing.
    Buffer samples = ramp(10 * N);

    {
        AudioPlaybackRt pb(SR, 1, 4, BUFFER_SECONDS);

        REQUIRE(pb.getSamplesPerBuffer() == N);

        pb.play(samples);

        play_until_played(pb, 1);
        pb.stop();

        CHECK(pb.get_debug_info().unknown_error_count == 0);
    }

    CHECK(not_silence() == to_int16(samples));
}

TEST_CASE("AudioPlaybackRt waits for free buffers, buffer after buffer")
{
    START_CAPTURE();

    const uint32 n_buffers = 500;
    Buffer samples = ramp(n_buffers * N);

    {
        AudioPlaybackRt pb(SR, 1, 4, BUFFER_SECONDS);

        pb.play(samples);

        // It waits at most once for each buffer written.
        CHECK(pb.get_debug_info().overrun_count > 0);
        CHECK(pb.get_debug_info().overrun_count <= n_buffers);

        play_until_played(pb, 1);
        pb.stop();
    }

    CHECK(not_silence() == to_int16(samples));
}

TEST_CASE("AudioPlaybackRt plays stereo interleaved")
{
    START_CAPTURE();

    Buffer left = ramp(4 * N);
    Buffer right = left * -1.0;

    AudioStream stereo(SR, 2);
    stereo[0] = left;
    stereo[1] = right;

    {
        AudioPlaybackRt pb(SR, 2, 4, BUFFER_SECONDS);

        pb.play(stereo);

        // A mono sample plays on both channels, a buffer of them.
        for(uint32 i = 0; i < N; ++i) pb.play(0.5);

        play_until_played(pb, 2);
        pb.stop();
    }

    std::vector<int16_t> expected;

    for(uint32 i = 0; i < left.getLength(); ++i)
    {
        expected.push_back(static_cast<int16_t>(32767.0 * left[i]));
        expected.push_back(static_cast<int16_t>(32767.0 * right[i]));
    }

    for(uint32 i = 0; i < 2 * N; ++i) expected.push_back(16383);

    CHECK(not_silence() == expected);
}

TEST_CASE("AudioPlaybackRt plays the underrun mode when it runs out")
{
    START_CAPTURE();

    BufferUnderrunMode mode = BUM_SILENCE;

    SECTION("silence") { mode = BUM_SILENCE; }
    SECTION("noise") { mode = BUM_NOISE; }
    SECTION("a tone") { mode = BUM_TONE; }

    // 5 buffers of samples and 1 of silence fill the pool of 6 before
    // playing starts, so they play one after the other, then the underruns.
    Buffer samples = ramp(5 * N);

    {
        AudioPlaybackRt pb(SR, 1, 6, BUFFER_SECONDS);

        pb.setBufferUnderrunMode(mode);

        pb.play(samples);

        play_until_played(pb, 1);

        // A few buffers of underruns.
        for(int i = 0; i < 200 && pb.get_debug_info().underrun_count < 4; ++i)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        pb.stop();

        CHECK(pb.get_debug_info().underrun_count >= 4);
    }

    std::vector<int16_t> all = captured<int16_t>();

    std::vector<int16_t> expected = to_int16(samples);

    REQUIRE(after_silence(expected) == expected);

    std::size_t start = 0;
    while(all[start] == 0) ++start;

    // After the samples and the buffer of silence.
    REQUIRE(all.size() > start + 6 * N);

    std::vector<int16_t> underrun(all.begin() + start + 6 * N, all.end());

    REQUIRE(underrun.size() >= 3 * N);

    int n_zeros = 0;
    int largest = 0;

    for(int16_t x : underrun)
    {
        if(x == 0) ++n_zeros;
        largest = std::max(largest, std::abs(static_cast<int>(x)));
    }

    // Noise and the tone are at 0.666 of full scale.
    CHECK(largest <= 21823);

    if(mode == BUM_SILENCE) CHECK(n_zeros == static_cast<int>(underrun.size()));
    else CHECK(n_zeros < static_cast<int>(underrun.size()) / 2);
}

TEST_CASE("AudioPlaybackRt plays what was written before it's destroyed")
{
    START_CAPTURE();

    // A buffer and a half: the pool of 4 never fills, so playing never
    // started, and the last buffer isn't full.
    Buffer samples = ramp(N + N / 2);

    {
        AudioPlaybackRt pb(SR, 1, 4, BUFFER_SECONDS);

        pb.play(samples);

        REQUIRE_FALSE(pb.get_debug_info().is_streaming);
    }

    CHECK(not_silence() == to_int16(samples));
}

TEST_CASE("AudioPlaybackRt plays through the host API named")
{
    START_CAPTURE();

    // On Linux portaudio has ALSA.
    std::vector<std::string> apis = AudioPlaybackRt::getHostApis();

    REQUIRE(std::find(apis.begin(), apis.end(), "ALSA") != apis.end());

    Buffer samples = ramp(2 * N);

    AudioPlaybackRt::use_host_api("alsa");

    {
        AudioPlaybackRt pb(SR, 1, 2, BUFFER_SECONDS);

        CHECK(pb.getInfo().find("ALSA") != std::string::npos);

        pb.play(samples);
    }

    CHECK(not_silence() == to_int16(samples));

    // An unknown name lists the ones there are.
    AudioPlaybackRt::use_host_api("no such api");

    try
    {
        AudioPlaybackRt pb(SR, 1, 2, BUFFER_SECONDS);
        FAIL("no exception");
    }
    catch(const Exception & e)
    {
        CHECK(std::string(e.what()).find("'ALSA'") != std::string::npos);
    }

    AudioPlaybackRt::use_host_api("");
}

TEST_CASE("AudioPlaybackRt clips at full scale")
{
    START_CAPTURE();

    // A buffer each of mono and stereo, full scale and past it.
    Buffer samples;
    for(uint32 i = 0; i < N / 4; ++i) samples << 1.0 << -1.0 << 2.0 << -2.0;

    std::vector<int16_t> expected;
    for(uint32 i = 0; i < N / 4; ++i)
    {
        expected.insert(expected.end(), {32767, -32767, 32767, -32767});
    }

    SECTION("mono")
    {
        {
            AudioPlaybackRt pb(SR, 1, 2, BUFFER_SECONDS);

            pb.play(samples);
            play_until_played(pb, 1);
            pb.stop();
        }

        CHECK(not_silence() == expected);
    }

    SECTION("stereo")
    {
        {
            AudioPlaybackRt pb(SR, 2, 2, BUFFER_SECONDS);

            for(uint32 i = 0; i < samples.getLength(); i += 2)
            {
                pb.play(samples[i], samples[i + 1]);
            }

            play_until_played(pb, 2);
            pb.stop();
        }

        // The full buffers played, a partial one waits for more samples.
        std::vector<int16_t> played = not_silence();

        REQUIRE_FALSE(played.empty());
        CHECK(played == std::vector<int16_t>(expected.begin(), expected.begin() + played.size()));
    }
}

TEST_CASE("AudioPlaybackRt checks its arguments, tells its state")
{
    START_CAPTURE();

    AudioPlaybackRt pb(SR, 1, 3, BUFFER_SECONDS);

    AudioStream three(SR, 3);
    three << 0.0;

    CHECK_THROWS_AS(pb.play(three), Exception);

    AudioPlaybackRtDebug info = pb.get_debug_info();

    CHECK(info.pool_size == 3);
    CHECK(info.samplerate == SR);
    CHECK(info.samples_per_buffer == N);
    CHECK_FALSE(info.is_streaming);
    CHECK(info.__str__().find("pool_size") != std::string::npos);

    CHECK(pb.getActualLatencySeconds() > 0.0);
    CHECK_FALSE(pb.getInfo().empty());
    CHECK_FALSE(pb.debug_print().empty());

    // The mode can't change while playing.
    pb.play(ramp(4 * N));
    REQUIRE(pb.get_debug_info().is_streaming);

    CHECK_THROWS_AS(pb.setBufferUnderrunMode(BUM_NOISE), Exception);

    pb.stop();

    CHECK_FALSE(pb.get_debug_info().is_streaming);
}
