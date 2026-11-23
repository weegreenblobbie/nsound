//-----------------------------------------------------------------------------
//
//  Wavefile write and read tests.
//
//-----------------------------------------------------------------------------

#include <Nsound/AudioStream.h>
#include <Nsound/Buffer.h>
#include <Nsound/Sine.h>
#include <Nsound/Wavefile.h>

#include "gold_check.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

using namespace Nsound;

static const float64 GAMMA = 1.5e-14;

//! Overwrites bytes of a file at offset, to make a malformed wave file.
static
void
patch_file(const std::string & filename, std::streamoff offset, const std::string & bytes)
{
    std::fstream file(filename, std::ios::in | std::ios::out | std::ios::binary);

    REQUIRE(file.good());

    file.seekp(offset);
    file.write(bytes.data(), static_cast<std::streamsize>(bytes.size()));
}

//! A little endian 16 bit integer as 2 bytes.
static
std::string
uint16_bytes(std::uint16_t value)
{
    return std::string({static_cast<char>(value & 0xff), static_cast<char>(value >> 8)});
}

//! Restores the defaults the other tests in this file expect.
static
void
reset_defaults()
{
    Wavefile::setDefaults(44100.0, 16.0, false);
}

TEST_CASE("Wavefile write() and read()")
{
    Sine sin(100);

    // A 3 channel 13 Hz wave, each channel quieter than the last.
    AudioStream data1(100, 3);

    data1 << sin.generate(1.0, 13.0);
    data1[1] *= 0.666666666666666;
    data1[2] *= 0.333333333333333;

    SECTION("48-bit")
    {
        Wavefile::setDefaultSampleSize(48);

        data1 >> "test_wavefile.wav";

        AudioStream data(100, 3);
        data << "test_wavefile.wav";

        check_gold(data, "gold/Wavefile_out1.wav", GAMMA);
    }

    SECTION("24-bit")
    {
        Wavefile::setDefaultSampleSize(24);

        data1 >> "test_wavefile2.wav";

        AudioStream data(100, 3);
        data << "test_wavefile2.wav";

        check_gold(data, "gold/Wavefile_out2.wav", 0.5 * GAMMA);
    }
}

TEST_CASE("Wavefile round trips every sample size")
{
    Sine sine(1000);

    // 2 channels, amplitude 0.9 so nothing clips.
    AudioStream as(1000, 2);
    as[0] = 0.9 * sine.generate(0.1, 30.0);
    as[1] = -0.5 * sine.generate(0.1, 70.0);

    struct Format
    {
        uint32 bits;
        boolean ieee_float;
        float64 tolerance;
    };

    // PCM truncates to the sample size, so a sample is off by less than one
    // step of 1 / (2^(bits - 1) - 1).
    const std::vector<Format> formats = {
        { 8, false, 1.0 / 127.0},
        {16, false, 1.0 / 32767.0},
        {24, false, 1.0 / 8388607.0},
        {32, false, 1.0 / 2147483647.0},
        {48, false, 1.0 / 140737488355327.0 + 1e-15},
        {64, false, 1e-15},
        {32, true,  1e-7},
        {64, true,  0.0},
    };

    for(const auto & format : formats)
    {
        INFO("bits " << format.bits << ", IEEE float " << format.ieee_float);

        Wavefile::setDefaultSampleSize(format.bits);
        Wavefile::setIEEEFloat(format.ieee_float);

        as >> "test_round_trip.wav";

        AudioStream read("test_round_trip.wav");

        CHECK(read.getSampleRate() == 1000.0);
        check_gold(as, "test_round_trip.wav", format.tolerance);

        // A Buffer reads the first channel.
        check_gold(as[0], "test_round_trip.wav", format.tolerance);
    }

    reset_defaults();
}

TEST_CASE("Wavefile 8 bit samples are unsigned")
{
    // Silence is 128 in an 8 bit wave file, what other programs expect.
    AudioStream as(1000, 1);
    as << Buffer::zeros(4);

    CHECK(Wavefile::write("test_8bit.wav", as, 8));

    std::ifstream file("test_8bit.wav", std::ios::binary);
    file.seekg(44);

    std::string data(4, '\0');
    file.read(&data[0], 4);

    CHECK(data == std::string(4, static_cast<char>(128)));

    check_gold(as, "test_8bit.wav", 0.0);
}

TEST_CASE("Wavefile clamps samples to -1 and 1")
{
    AudioStream as(1000, 1);
    as << Buffer({1.0, -1.0, 1.5, -1.5});

    // Every integer sample size, with the 64 bit scale past the largest int64.
    for(uint32 bits : {8u, 16u, 24u, 32u, 48u, 64u})
    {
        INFO("bits " << bits);

        CHECK(Wavefile::write("test_clamp.wav", as, bits));

        check_gold(AudioStream("test_clamp.wav"), "test_clamp.wav", 0.0);
        check_close(AudioStream("test_clamp.wav")[0], Buffer({1.0, -1.0, 1.0, -1.0}), 1e-15);
    }
}

TEST_CASE("Wavefile::write() with a sample size")
{
    Sine sine(1000);

    Buffer b = 0.5 * sine.generate(0.05, 20.0);

    SECTION("a Buffer")
    {
        CHECK(Wavefile::write("test_write.wav", b, 24, 1000));

        AudioStream read("test_write.wav");

        CHECK(read.getSampleRate() == 1000.0);
        check_gold(b, "test_write.wav", 1.0 / 8388607.0);
    }

    SECTION("an AudioStream")
    {
        AudioStream as(1000, 1);
        as << b;

        CHECK(Wavefile::write("test_write.wav", as, 8));

        check_gold(as, "test_write.wav", 1.0 / 127.0);
    }

    SECTION("a sample size that isn't supported throws")
    {
        AudioStream as(1000, 1);
        as << b;

        CHECK_THROWS_AS(Wavefile::write("test_write.wav", as, 12), Exception);
        CHECK_THROWS_AS(Wavefile::write("test_write.wav", b, 12, 1000), Exception);
    }

    SECTION("a file that can't be opened throws")
    {
        AudioStream as(1000, 1);
        as << b;

        CHECK_THROWS_AS(Wavefile::write("no_such_dir/test.wav", as, 16), Exception);
        CHECK_THROWS_AS(Wavefile::write("no_such_dir/test.wav", b, 16, 1000), Exception);
    }
}

TEST_CASE("Wavefile defaults")
{
    SECTION("setDefaults()")
    {
        Wavefile::setDefaults(8000.0, 24.0, false);

        CHECK(Wavefile::getDefaultSampleRate() == 8000);
        CHECK(Wavefile::getDefaultSampleSize() == 24);
    }

    SECTION("setDefaultSampleRate()")
    {
        Wavefile::setDefaultSampleRate(22050);
        CHECK(Wavefile::getDefaultSampleRate() == 22050);

        Wavefile::setDefaultSampleRate(11025.0);
        CHECK(Wavefile::getDefaultSampleRate() == 11025);
    }

    SECTION("setDefaultSampleSize() only takes the supported sizes")
    {
        CHECK_THROWS_AS(Wavefile::setDefaultSampleSize(12), Exception);
    }

    SECTION("setIEEEFloat() picks 32 or 64 bits")
    {
        Wavefile::setDefaultSampleSize(16);
        Wavefile::setIEEEFloat(true);
        CHECK(Wavefile::getDefaultSampleSize() == 32);

        Wavefile::setDefaultSampleSize(48);
        Wavefile::setIEEEFloat(true);
        CHECK(Wavefile::getDefaultSampleSize() == 64);
    }

    reset_defaults();
}

TEST_CASE("Wavefile::decodeFormatTag()")
{
    CHECK(Wavefile::decodeFormatTag(0x0001) == "Microsoft PCM");
    CHECK(Wavefile::decodeFormatTag(0x0003) == "IEEE Float");

    // Every tag has a name, known or not.
    for(uint32 tag = 0; tag <= 0xffff; ++tag)
    {
        if(Wavefile::decodeFormatTag(static_cast<uint16>(tag)).empty())
        {
            FAIL("no name for format tag " << tag);
        }
    }
}

TEST_CASE("Wavefile::readHeader()")
{
    Wavefile::setDefaultSampleSize(16);
    Wavefile::setIEEEFloat(false);

    AudioStream as(1000, 2);
    as << Buffer::zeros(20);

    as >> "test_header.wav";

    std::string info;

    CHECK(Wavefile::readHeader("test_header.wav", info));

    CHECK(info.find("Microsoft PCM") != std::string::npos);
    CHECK(info.find("[bits_per_sample]      = [16") != std::string::npos);
    CHECK(info.find("channel[1] = ") != std::string::npos);

    SECTION("IEEE float")
    {
        Wavefile::setIEEEFloat(true);

        as >> "test_header.wav";

        CHECK(Wavefile::readHeader("test_header.wav", info));
        CHECK(info.find("IEEE Float") != std::string::npos);
    }

    reset_defaults();
}

TEST_CASE("Wavefile::read() rejects bad files")
{
    // The 44 byte header Wavefile writes:
    //   0 "RIFF", 8 "WAVE", 12 "fmt ", 20 format tag, 22 channels,
    //  34 bits per sample, 36 "data", 40 data length.
    Wavefile::setDefaultSampleSize(16);
    Wavefile::setIEEEFloat(false);

    AudioStream as(1000, 1);
    as << Buffer::ones(10);

    as >> "test_bad.wav";

    AudioStream read;

    SECTION("a missing file")
    {
        CHECK_THROWS_AS(read << "no_such_file.wav", Exception);
    }

    SECTION("not RIFF")
    {
        patch_file("test_bad.wav", 0, "RIFX");
        CHECK_THROWS_AS(read << "test_bad.wav", Exception);
    }

    SECTION("not WAVE")
    {
        patch_file("test_bad.wav", 8, "AVI ");
        CHECK_THROWS_AS(read << "test_bad.wav", Exception);
    }

    SECTION("a format that isn't PCM or IEEE float")
    {
        patch_file("test_bad.wav", 20, uint16_bytes(0x0002));
        CHECK_THROWS_AS(read << "test_bad.wav", Exception);
    }

    SECTION("an IEEE float sample size that isn't 32 or 64")
    {
        patch_file("test_bad.wav", 20, uint16_bytes(0x0003));
        CHECK_THROWS_AS(read << "test_bad.wav", Exception);

        Buffer buffer;
        CHECK_THROWS_AS(buffer << "test_bad.wav", Exception);

        std::string info;
        CHECK_FALSE(Wavefile::readHeader("test_bad.wav", info));
    }

    SECTION("an unsupported sample size")
    {
        patch_file("test_bad.wav", 34, uint16_bytes(12));
        CHECK_THROWS_AS(read << "test_bad.wav", Exception);
    }

    SECTION("0 channels")
    {
        patch_file("test_bad.wav", 22, uint16_bytes(0));
        CHECK_THROWS_AS(read << "test_bad.wav", Exception);
    }

    SECTION("0 bits per sample")
    {
        patch_file("test_bad.wav", 34, uint16_bytes(0));
        CHECK_THROWS_AS(read << "test_bad.wav", Exception);
    }

    SECTION("no data")
    {
        patch_file("test_bad.wav", 40, std::string(4, '\0'));
        CHECK_THROWS_AS(read << "test_bad.wav", Exception);
    }

    SECTION("no data chunk")
    {
        patch_file("test_bad.wav", 36, "junk");
        CHECK_THROWS_AS(read << "test_bad.wav", Exception);
    }

    reset_defaults();
}

TEST_CASE("ID3v1Tag")
{
    Wavefile::setDefaultSampleSize(16);
    Wavefile::setIEEEFloat(false);

    AudioStream as(1000, 1);
    as << Buffer::ones(10);

    as >> "test_tag.wav";

    SECTION("write() then read() gets the fields back")
    {
        ID3v1Tag tag;
        tag.title = "a title";
        tag.artist = "an artist";
        tag.album = "an album";
        tag.year = "2026";
        tag.comment = "a comment";
        tag.genre = 7;

        CHECK(tag.write("test_tag.wav"));

        ID3v1Tag read("test_tag.wav");

        CHECK(read.title == "a title");
        CHECK(read.artist == "an artist");
        CHECK(read.album == "an album");
        CHECK(read.year == "2026");
        CHECK(read.comment == "a comment");
        CHECK(read.genre == 7);

        // The audio is still there.
        check_gold(as, "test_tag.wav", 1.0 / 32767.0);

        std::ostringstream out;
        out << read;

        CHECK(out.str().find("title   : a title") != std::string::npos);
    }

    SECTION("a file without a tag")
    {
        ID3v1Tag tag;

        CHECK_FALSE(tag.read("test_tag.wav", false));
        CHECK_FALSE(tag.read("test_tag.wav", true));
    }

    SECTION("a missing file throws")
    {
        ID3v1Tag tag;

        CHECK_THROWS_AS(tag.read("no_such_file.wav"), Exception);
    }

    SECTION("a file that isn't a wave file throws")
    {
        patch_file("test_tag.wav", 0, "RIFX");

        ID3v1Tag tag;

        CHECK_THROWS_AS(tag.read("test_tag.wav"), Exception);
    }

    reset_defaults();
}

TEST_CASE("Wavefile::write(Buffer) every sample size")
{
    Buffer b({0.5, -0.25, 1.5, -1.5});

    // Clamped to -1 and 1.
    Buffer expected({0.5, -0.25, 1.0, -1.0});

    for(uint32 bits : {8u, 16u, 24u, 32u, 48u, 64u})
    {
        INFO("bits " << bits);

        CHECK(Wavefile::write("test_buffer_sizes.wav", b, bits, 1000));

        check_gold(expected, "test_buffer_sizes.wav", 1.0 / 127.0);
    }

    SECTION("IEEE float")
    {
        Wavefile::setIEEEFloat(true);

        CHECK(Wavefile::write("test_buffer_sizes.wav", b, 32, 1000));
        check_gold(b, "test_buffer_sizes.wav", 0.0);

        CHECK(Wavefile::write("test_buffer_sizes.wav", b, 64, 1000));
        check_gold(b, "test_buffer_sizes.wav", 0.0);

        // IEEE float is only 32 or 64 bit.
        CHECK_THROWS_AS(Wavefile::write("test_buffer_sizes.wav", b, 16, 1000), Exception);

        AudioStream as(1000, 1);
        as << b;

        CHECK_THROWS_AS(Wavefile::write("test_buffer_sizes.wav", as, 16), Exception);
    }

    reset_defaults();
}

TEST_CASE("Wavefile::read(filename, AudioStream)")
{
    AudioStream as(1000, 2);
    as << Buffer({0.5, -0.5});

    CHECK(Wavefile::write("test_read.wav", as, 16));

    AudioStream read;

    CHECK(Wavefile::read("test_read.wav", read));
    check_gold(read, "test_read.wav", 0.0);
    check_close(read[1], Buffer({0.5, -0.5}), 1.0 / 32767.0);
}

TEST_CASE("Wavefile::readHeader() of 64 bit IEEE float")
{
    Wavefile::setDefaultSampleSize(64);
    Wavefile::setIEEEFloat(true);

    AudioStream as(1000, 1);
    as << Buffer({0.25});

    as >> "test_header64.wav";

    std::string info;

    CHECK(Wavefile::readHeader("test_header64.wav", info));
    CHECK(info.find("[bits_per_sample]      = [64") != std::string::npos);
    CHECK(info.find("channel[0] = 0.25") != std::string::npos);

    reset_defaults();
}

TEST_CASE("Wavefile::read() of a truncated file reads only what is there")
{
    AudioStream as(1000, 1);
    as << Buffer::ones(10);

    CHECK(Wavefile::write("test_truncated.wav", as, 16));

    AudioStream read;

    SECTION("the header says 20 samples, there are 10")
    {
        patch_file("test_truncated.wav", 40, std::string({40, 0, 0, 0}));

        REQUIRE_NOTHROW(read << "test_truncated.wav");

        check_close(read[0], Buffer::ones(10), 1.0 / 32767.0);
    }

    SECTION("a forged header says about 4 GB")
    {
        patch_file("test_truncated.wav", 40, std::string(4, static_cast<char>(0xff)));

        REQUIRE_NOTHROW(read << "test_truncated.wav");

        check_close(read[0], Buffer::ones(10), 1.0 / 32767.0);
    }
}

TEST_CASE("Wavefile::read() rejects fewer than 8 bits per sample")
{
    AudioStream as(1000, 1);
    as << Buffer::ones(10);

    CHECK(Wavefile::write("test_4bit.wav", as, 16));

    // The bytes per sample would be 4 / 8 = 0, then a division by 0.
    patch_file("test_4bit.wav", 34, uint16_bytes(4));

    AudioStream read;

    CHECK_THROWS_AS(read << "test_4bit.wav", Exception);
}

TEST_CASE("ID3v1Tag errors")
{
    AudioStream as(1000, 1);
    as << Buffer::ones(10);

    CHECK(Wavefile::write("test_tag_errors.wav", as, 16));

    ID3v1Tag tag;
    tag.title = "a title";

    SECTION("a short year")
    {
        tag.year = "26";

        CHECK(tag.write("test_tag_errors.wav"));

        ID3v1Tag read("test_tag_errors.wav");

        CHECK(read.year == "26");
    }

    SECTION("not WAVE")
    {
        patch_file("test_tag_errors.wav", 8, "AVI ");

        CHECK_THROWS_AS(tag.read("test_tag_errors.wav"), Exception);
        CHECK_THROWS_AS(tag.write("test_tag_errors.wav"), Exception);
    }

    SECTION("no data chunk")
    {
        patch_file("test_tag_errors.wav", 36, "junk");

        CHECK_THROWS_AS(tag.read("test_tag_errors.wav"), Exception);
    }

    SECTION("write() to a file that can't be opened")
    {
        CHECK_THROWS_AS(tag.write("no_such_dir/test.wav"), Exception);
    }
}

//! Returns how many files this process has open, -1 where it can't be
//! counted (Linux only).
static
int
count_open_files()
{
    const std::filesystem::path fd_dir("/proc/self/fd");

    if(!std::filesystem::exists(fd_dir)) return -1;

    int count = 0;

    for([[maybe_unused]] const auto & entry : std::filesystem::directory_iterator(fd_dir))
    {
        ++count;
    }

    return count;
}

TEST_CASE("Wavefile::read() checks every chunk against the file's size")
{
    using Catch::Matchers::ContainsSubstring;

    // The 44 byte header Wavefile writes:
    //   0 "RIFF", 8 "WAVE", 12 "fmt ", 16 its size, 36 "data", 40 its size.
    AudioStream as(1000, 1);
    as << Buffer::ones(10);

    CHECK(Wavefile::write("test_chunks.wav", as, 16));

    AudioStream read;

    SECTION("a chunk that says it's about 4 GB")
    {
        // 0xfffffff8 is -8 as a 32 bit long, it seeked back to the chunk
        // itself on Windows, forever.
        patch_file("test_chunks.wav", 12, "junk");
        patch_file("test_chunks.wav", 16, std::string({
            static_cast<char>(0xf8), static_cast<char>(0xff),
            static_cast<char>(0xff), static_cast<char>(0xff)}));

        CHECK_THROWS_WITH(read << "test_chunks.wav", ContainsSubstring("goes past the end of the file"));
    }

    SECTION("a 'fmt ' chunk past the end of the file")
    {
        patch_file("test_chunks.wav", 16, std::string({0, 0, 0, 0x40}));

        CHECK_THROWS_WITH(read << "test_chunks.wav", ContainsSubstring("goes past the end of the file"));
    }

    SECTION("a 'fmt ' chunk shorter than its 16 bytes of fields")
    {
        patch_file("test_chunks.wav", 16, std::string({14, 0, 0, 0}));

        CHECK_THROWS_WITH(read << "test_chunks.wav", ContainsSubstring("shorter than 16"));
    }

    SECTION("a file shorter than the RIFF header")
    {
        std::ofstream("test_chunks.wav", std::ios::binary) << "RIFF";

        CHECK_THROWS_WITH(read << "test_chunks.wav", ContainsSubstring("too short for a wave file"));
    }
}

TEST_CASE("Wavefile and ID3v1Tag close the file when they throw")
{
    int before = count_open_files();

    if(before < 0) SKIP("open files are counted on Linux only");

    AudioStream as(1000, 1);
    as << Buffer::ones(10);

    AudioStream read;
    Buffer buffer;
    ID3v1Tag tag;

    // No data.
    CHECK(Wavefile::write("test_leak.wav", as, 16));
    patch_file("test_leak.wav", 40, std::string(4, '\0'));
    CHECK_THROWS_AS(read << "test_leak.wav", Exception);

    // 0 channels.
    CHECK(Wavefile::write("test_leak.wav", as, 16));
    patch_file("test_leak.wav", 22, uint16_bytes(0));
    CHECK_THROWS_AS(read << "test_leak.wav", Exception);

    // An IEEE float sample size that isn't 32 or 64.
    CHECK(Wavefile::write("test_leak.wav", as, 16));
    patch_file("test_leak.wav", 20, uint16_bytes(0x0003));
    CHECK_THROWS_AS(read << "test_leak.wav", Exception);
    CHECK_THROWS_AS(buffer << "test_leak.wav", Exception);

    // Not a wave file.
    patch_file("test_leak.wav", 8, "AVI ");
    CHECK_THROWS_AS(tag.read("test_leak.wav"), Exception);
    CHECK_THROWS_AS(tag.write("test_leak.wav"), Exception);

    CHECK(count_open_files() == before);
}

TEST_CASE("ID3v1Tag::write() to a file without a tag keeps the audio")
{
    AudioStream as(1000, 1);
    as << Buffer({0.5, -0.25, 0.75, -0.5});

    CHECK(Wavefile::write("test_tag_append.wav", as, 16));

    // 2 bytes after the data: the scan for 'TAG' stopped short of the end
    // and wrote the tag over the last 3 bytes of the audio.
    std::ofstream("test_tag_append.wav", std::ios::binary | std::ios::app) << "xy";

    ID3v1Tag tag;
    tag.title = "a title";

    CHECK(tag.write("test_tag_append.wav"));

    AudioStream read;
    read << "test_tag_append.wav";

    check_close(read[0], Buffer({0.5, -0.25, 0.75, -0.5}), 1.0 / 32767.0);

    ID3v1Tag tag_read("test_tag_append.wav");

    CHECK(tag_read.title == "a title");
}
