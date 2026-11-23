//-----------------------------------------------------------------------------
//
//  Plays into a file instead of a sound card, for the playback tests.  On
//  Linux portaudio plays through ALSA, and alsa_capture.conf makes ALSA's
//  default device write the samples to a file.
//
//-----------------------------------------------------------------------------

#pragma once

#include <Nsound/Nsound.h>

#include <catch2/catch_test_macros.hpp>

#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

namespace playback_capture
{

//! The file ALSA writes the played samples to, in the test's temporary
//! folder.
inline
std::filesystem::path
capture_file()
{
    const char * tmpdir = std::getenv("TEST_TMPDIR");

    REQUIRE(tmpdir != nullptr);

    return std::filesystem::path(tmpdir) / "capture.raw";
}

//! Sends playback to capture_file(), an empty file, and returns true, or
//! returns false where playback can't be captured: off Linux or without
//! portaudio.
//
//! ALSA reads its configuration once, at the first playback, the file name
//! at each.
inline
bool
start()
{
    #if defined(NSOUND_PLATFORM_OS_LINUX) && defined(NSOUND_LIBPORTAUDIO)

        const char * srcdir = std::getenv("TEST_SRCDIR");

        REQUIRE(srcdir != nullptr);

        std::filesystem::path config =
            std::filesystem::path(srcdir) / "_main" / "src" / "test" / "alsa_capture.conf";

        setenv("ALSA_CONFIG_PATH", config.c_str(), 1);
        setenv("NSOUND_ALSA_CAPTURE", capture_file().c_str(), 1);

        std::filesystem::remove(capture_file());

        return true;

    #else

        return false;

    #endif
}

//! Returns the samples played since start(), interleaved, as T.
template <class T>
std::vector<T>
captured()
{
    std::ifstream file(capture_file(), std::ios::binary);

    std::vector<char> bytes(
        (std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>());

    REQUIRE(bytes.size() % sizeof(T) == 0);

    std::vector<T> samples(bytes.size() / sizeof(T));

    std::memcpy(samples.data(), bytes.data(), bytes.size());

    return samples;
}

} // namespace
