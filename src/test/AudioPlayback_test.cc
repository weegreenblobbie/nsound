//-----------------------------------------------------------------------------
//
//  AudioPlayback tests, it plays through portaudio into the capture file of
//  playback_capture.h, the tests check the samples that arrive.
//
//-----------------------------------------------------------------------------

#include <Nsound/AudioBackendType.h>
#include <Nsound/AudioPlayback.h>
#include <Nsound/AudioStream.h>
#include <Nsound/Buffer.h>

#include "playback_capture.h"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>

using namespace Nsound;

using playback_capture::captured;

//! Skips the test where playback can't be captured.
#define START_CAPTURE()                                                    \
    if(! playback_capture::start())                                        \
    {                                                                      \
        SKIP("Playback is captured through portaudio and ALSA, on Linux"); \
    }

TEST_CASE("AudioPlayback lists the backends compiled in")
{
    #ifdef NSOUND_LIBPORTAUDIO
        CHECK(getBackends() == std::vector<std::string>({"portaudio"}));
        CHECK(getBackendTypes() == std::vector<AudioBackendType>({BACKEND_TYPE_LIBPORTAUDIO}));
        CHECK(AudioPlayback::getBackendType() == BACKEND_TYPE_LIBPORTAUDIO);
    #else
        CHECK(getBackends().empty());
        CHECK(getBackendTypes().empty());
        CHECK(AudioPlayback::getBackendType() == BACKEND_TYPE_NONE);
    #endif

    CHECK_NOTHROW(use("PortAudio"));
    CHECK_NOTHROW(use(BACKEND_TYPE_LIBPORTAUDIO));

    CHECK_THROWS_AS(use("ao"), Exception);
    CHECK_THROWS_AS(use(BACKEND_TYPE_NONE), Exception);
}

TEST_CASE("AudioPlayback plays 16 bit samples")
{
    START_CAPTURE();

    {
        AudioPlayback pb(8000.0, 1, 16);

        pb.play(Buffer({0.0, 0.5, -0.5, 0.25, -1.0}));

        CHECK(pb.getState() == AudioBackend::BACKEND_READY);
    }

    CHECK(captured<int16_t>() == std::vector<int16_t>({0, 16384, -16384, 8192, -32768}));
}

TEST_CASE("AudioPlayback clips full scale")
{
    START_CAPTURE();

    {
        AudioPlayback pb(8000.0, 1, 16);

        pb.play(Buffer({1.0, 2.0, -2.0}));
    }

    CHECK(captured<int16_t>() == std::vector<int16_t>({32767, 32767, -32768}));
}

TEST_CASE("AudioPlayback plays 32 bit samples")
{
    START_CAPTURE();

    {
        AudioPlayback pb(8000.0, 1, 32);

        pb.play(Buffer({0.0, 0.5, -0.25}));
    }

    CHECK(captured<int32_t>() == std::vector<int32_t>({0, 1073741824, -536870912}));
}

TEST_CASE("AudioPlayback plays the channels interleaved")
{
    START_CAPTURE();

    AudioStream stereo(8000.0, 2);
    stereo[0] << 0.5 << 0.25;
    stereo[1] << -0.5 << -0.25;

    {
        AudioPlayback pb(8000.0, 2, 16);

        pb.play(stereo);

        // The channels must match.
        AudioStream three(8000.0, 3);
        three << 0.0;

        CHECK_THROWS_AS(pb.play(three), Exception);
    }

    CHECK(captured<int16_t>() == std::vector<int16_t>({16384, -16384, 8192, -8192}));
}

TEST_CASE("AudioPlayback plays a mono AudioStream and with >>")
{
    START_CAPTURE();

    AudioStream mono(8000.0, 1);
    mono << 0.5;

    {
        AudioPlayback pb(8000.0, 1, 16);

        pb.play(mono);
        Buffer({0.25}) >> pb;
        mono >> pb;
    }

    CHECK(captured<int16_t>() == std::vector<int16_t>({16384, 8192, 16384}));
}

TEST_CASE("AudioPlayback plays 16 or 32 bits only")
{
    START_CAPTURE();

    AudioPlayback pb(8000.0, 1, 8);

    CHECK_THROWS_AS(pb.play(Buffer({0.5})), Exception);
}

TEST_CASE("AudioPlayback tells its settings and state")
{
    START_CAPTURE();

    AudioPlayback pb(8000.0, 2, 32);

    CHECK(pb.getSampleRate() == 8000);
    CHECK(pb.getChannels() == 2);
    CHECK(pb.getBitsPerSample() == 32);

    // The backend starts at the first play() or initialize().
    CHECK(pb.getState() == AudioBackend::BACKEND_NOT_INITIALIZED);
    CHECK(pb.getStateString() == "Backend Not Initialized");
    CHECK(pb.getStateString(AudioBackend::BACKEND_ERROR) == "Backend Error");
    CHECK_THROWS_AS(pb.getInfo(), Exception);

    pb.initialize();

    CHECK(pb.getState() == AudioBackend::BACKEND_READY);
    CHECK(pb.getStateString() == "Backend Ready");
    CHECK(pb.getInfo().find("libportaudio Default Driver Info") != std::string::npos);
    CHECK(pb.getError().empty());

    // Options are set before the backend starts.
    CHECK_THROWS_AS(pb.setOption("frames_per_buffer", "16"), Exception);

    pb.shutdown();

    CHECK(pb.getInfo().find("backend not initialized") != std::string::npos);
    CHECK(pb.getError().find("backend not initialized") != std::string::npos);
}

TEST_CASE("AudioPlayback options")
{
    START_CAPTURE();

    SECTION("frames_per_buffer: whole buffers, then the frames left")
    {
        Buffer samples;
        for(int i = 0; i < 40; ++i) samples << (i - 20) / 64.0;

        std::vector<int16_t> expected;
        for(float64 x : samples) expected.push_back(static_cast<int16_t>(x * 32768.0));

        {
            AudioPlayback pb(8000.0, 1, 16);

            pb.setOption("frames_per_buffer", "16");
            pb.play(samples);
        }

        CHECK(captured<int16_t>() == expected);
    }

    SECTION("the driver, a portaudio host API by name, plays its default device")
    {
        {
            AudioPlayback pb(8000.0, 1, 16);

            pb.setOption("driver", "ALSA");
            pb.play(Buffer({0.5, -0.5}));
        }

        CHECK(captured<int16_t>() == std::vector<int16_t>({16384, -16384}));

        AudioPlayback pb(8000.0, 1, 16);

        CHECK_THROWS_AS(pb.setOption("driver", "no such driver"), Exception);
    }

    SECTION("unknown options")
    {
        AudioPlayback pb(8000.0, 1, 16);

        CHECK_THROWS_AS(pb.setOption("volume", "11"), Exception);
    }
}

TEST_CASE("AudioPlayback plays 32 bit stereo interleaved")
{
    START_CAPTURE();

    AudioStream stereo(8000.0, 2);
    stereo[0] << 0.5 << -0.25;
    stereo[1] << -0.5 << 0.25;

    {
        AudioPlayback pb(8000.0, 2, 32);

        pb.play(stereo);
    }

    CHECK(captured<int32_t>() == std::vector<int32_t>({
        1073741824, -1073741824, -536870912, 536870912}));
}

TEST_CASE("AudioPlayback scanDevices() plays a clip through each device")
{
    START_CAPTURE();

    {
        AudioPlayback pb(8000.0, 1, 16);

        pb.scanDevices();
    }

    // The capture device is the only one, it got the clip.
    std::vector<int16_t> clip = captured<int16_t>();

    CHECK(clip.size() > 8000u);
    CHECK(std::count(clip.begin(), clip.end(), int16_t(0)) < static_cast<long>(clip.size()) / 2);
}
