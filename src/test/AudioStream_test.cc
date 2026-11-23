//-----------------------------------------------------------------------------
//
//  AudioStream and AudioStreamSelection tests.
//
//-----------------------------------------------------------------------------

#include <Nsound/Nsound.h>

#ifdef NSOUND_C_PYLAB
    #include "plot_check.h"
#endif

#include <Nsound/AudioStream.h>
#include <Nsound/AudioStreamSelection.h>
#include <Nsound/Buffer.h>
#include <Nsound/Sine.h>
#include <Nsound/Wavefile.h>

#include "gold_check.h"

#include <catch2/catch_test_macros.hpp>

#include <sstream>
#include <string>
#include <vector>

using namespace Nsound;

//! Checks that as has one channel per expected Buffer, each matching it.
static
void
check_channels(const AudioStream & as, const std::vector<Buffer> & expected)
{
    REQUIRE(as.getNChannels() == expected.size());

    for(uint32 i = 0; i < expected.size(); ++i)
    {
        INFO("channel " << i);
        check_close(as[i], expected[i], 0.0);
    }
}

//! A 2 channel stream at 10 Hz.
static
AudioStream
make_stereo(const Buffer & left, const Buffer & right)
{
    AudioStream as(10.0, 2);

    as[0] = left;
    as[1] = right;

    return as;
}

static const Buffer LEFT({1.0, 4.0, 9.0, 16.0});
static const Buffer RIGHT({-2.0, 0.5, 3.0, -8.0});

TEST_CASE("AudioStream constructors")
{
    SECTION("default is 1 empty channel at 44100 Hz")
    {
        AudioStream as;

        CHECK(as.getSampleRate() == 44100.0);
        CHECK(as.getNChannels() == 1);
        CHECK(as.getLength() == 0);
    }

    SECTION("copy")
    {
        AudioStream as = make_stereo(LEFT, RIGHT);
        AudioStream copy(as);

        CHECK(copy.getSampleRate() == 10.0);
        check_channels(copy, {LEFT, RIGHT});
    }

    SECTION("from a wave file, and readWavefile(), writeWavefile()")
    {
        Wavefile::setDefaultSampleSize(64);
        Wavefile::setIEEEFloat(true);

        AudioStream as = make_stereo(0.1 * LEFT, 0.1 * RIGHT);

        as.writeWavefile("test_stream.wav");

        check_channels(AudioStream("test_stream.wav"), {0.1 * LEFT, 0.1 * RIGHT});

        AudioStream read;
        read.readWavefile("test_stream.wav");

        check_channels(read, {0.1 * LEFT, 0.1 * RIGHT});
    }
}

TEST_CASE("AudioStream add()")
{
    AudioStream a(2.0, 2);
    a << Buffer::ones(4);

    AudioStream b(2.0, 2);
    b << Buffer::ones(2);

    SECTION("add(as, offset, n_samples)")
    {
        a.add(b, uint32(1), uint32(0));

        check_channels(a, {Buffer({1, 2, 2, 1}), Buffer({1, 2, 2, 1})});
    }

    SECTION("add(as, offset_seconds, duration_seconds)")
    {
        // 1 second at 2 Hz is 2 samples, 0.5 seconds is 1.
        a.add(b, 1.0, 0.5);

        check_channels(a, {Buffer({1, 1, 2, 1}), Buffer({1, 1, 2, 1})});
    }
}

TEST_CASE("AudioStream applies Buffer methods to every channel")
{
    AudioStream as = make_stereo(LEFT, RIGHT);

    //! Checks each channel of as against op applied to LEFT and RIGHT.
    auto check_like_buffer = [&as](auto op)
    {
        Buffer left = LEFT;
        Buffer right = RIGHT;
        op(left);
        op(right);
        check_channels(as, {left, right});
    };

    SECTION("abs()")        { as.abs();          check_like_buffer([](Buffer & b){ b.abs(); }); }
    SECTION("convolve()")   { as.convolve(Buffer({1, 1})); check_like_buffer([](Buffer & b){ b.convolve(Buffer({1, 1})); }); }
    SECTION("dB()")         { as.abs(); as.dB(); check_like_buffer([](Buffer & b){ b.abs(); b.dB(); }); }
    SECTION("derivative()") { as.derivative(1);  check_like_buffer([](Buffer & b){ b.derivative(1); }); }
    SECTION("downSample()") { as.downSample(2);  check_like_buffer([](Buffer & b){ b.downSample(2); }); }
    SECTION("upSample()")   { as.upSample(2);    check_like_buffer([](Buffer & b){ b.upSample(2); }); }
    SECTION("limit()")      { as.limit(0.0, 5.0); check_like_buffer([](Buffer & b){ b.limit(0.0, 5.0); }); }
    SECTION("limit(Buffer)")
    {
        as.limit(Buffer({0.0}), Buffer({5.0, 2.0}));
        check_like_buffer([](Buffer & b){ b.limit(Buffer({0.0}), Buffer({5.0, 2.0})); });
    }
    SECTION("reverse()")    { as.reverse();      check_like_buffer([](Buffer & b){ b.reverse(); }); }
    SECTION("smooth()")     { as.smooth(1, 2);   check_like_buffer([](Buffer & b){ b.smooth(1, 2); }); }
    SECTION("speedUp()")    { as.speedUp(2.0f);  check_like_buffer([](Buffer & b){ b.speedUp(2.0); }); }
    SECTION("speedUp(Buffer)")
    {
        as.speedUp(Buffer({1.0, 2.0}));
        check_like_buffer([](Buffer & b){ b.speedUp(Buffer({1.0, 2.0})); });
    }
    SECTION("getSpeedUp()")
    {
        as = as.getSpeedUp(Buffer({2.0}));
        check_like_buffer([](Buffer & b){ b.speedUp(Buffer({2.0})); });
    }
    SECTION("sqrt()")       { as.sqrt();         check_like_buffer([](Buffer & b){ b.sqrt(); }); }
    SECTION("resample(factor)")
    {
        as.resample(1.5);
        check_like_buffer([](Buffer & b){ b.resample(1.5); });
    }
    SECTION("resample(Buffer)")
    {
        as.resample(Buffer({0.5, 0.5, 0.5, 0.5}));
        check_like_buffer([](Buffer & b){ b.resample(Buffer({0.5, 0.5, 0.5, 0.5})); });
    }
    SECTION("getResample()")
    {
        as = as.getResample(0.5);
        check_like_buffer([](Buffer & b){ b.resample(0.5); });
    }
    SECTION("resample2() changes the sample rate")
    {
        as.resample2(20.0);

        CHECK(as.getSampleRate() == 20.0);
        check_like_buffer([](Buffer & b){ b.resample(2.0); });
    }
}

TEST_CASE("AudioStream getLength(), getDuration()")
{
    // The shortest channel.
    AudioStream as = make_stereo(Buffer({1, 2, 3}), Buffer({1, 2, 3, 4, 5}));

    CHECK(as.getLength() == 3);
    CHECK(as.getDuration() == 0.3);
}

TEST_CASE("AudioStream getMax(), getMin(), getMaxMagnitude()")
{
    SECTION("mixed values")
    {
        AudioStream as = make_stereo(LEFT, RIGHT);

        CHECK(as.getMax() == 16.0);
        CHECK(as.getMin() == -8.0);
        CHECK(as.getMaxMagnitude() == 16.0);
    }

    SECTION("all negative")
    {
        AudioStream as = make_stereo(Buffer({-3.0, -1.0}), Buffer({-2.0, -5.0}));

        CHECK(as.getMax() == -1.0);
        CHECK(as.getMaxMagnitude() == 5.0);
    }

    SECTION("silence")
    {
        AudioStream as = make_stereo(Buffer({0.0, 0.0}), Buffer({0.0, 0.0}));

        CHECK(as.getMaxMagnitude() == 0.0);
    }
}

TEST_CASE("AudioStream mono(), getMono()")
{
    AudioStream as = make_stereo(Buffer({1.0, 3.0}), Buffer({3.0, 5.0}));

    check_channels(as.getMono(), {Buffer({2.0, 4.0})});

    as.mono();

    check_channels(as, {Buffer({2.0, 4.0})});
}

TEST_CASE("AudioStream normalize()")
{
    SECTION("scales the peak magnitude to 1")
    {
        AudioStream as = make_stereo(Buffer({0.5, -2.0}), Buffer({1.0, 0.0}));

        as.normalize();

        check_channels(as, {Buffer({0.25, -1.0}), Buffer({0.5, 0.0})});
    }

    SECTION("leaves silence alone")
    {
        AudioStream as = make_stereo(Buffer({0.0, 0.0}), Buffer({0.0, 0.0}));

        as.normalize();

        check_channels(as, {Buffer({0.0, 0.0}), Buffer({0.0, 0.0})});
    }
}

TEST_CASE("AudioStream assignment, comparison, element access")
{
    AudioStream as = make_stereo(LEFT, RIGHT);

    SECTION("operator=(AudioStream)")
    {
        AudioStream other(44100.0, 3);
        other = as;

        CHECK(other.getSampleRate() == 10.0);
        check_channels(other, {LEFT, RIGHT});

        AudioStream & same = other;
        other = same;

        check_channels(other, {LEFT, RIGHT});
    }

    SECTION("operator=(Buffer) sets every channel")
    {
        as = Buffer({7.0});

        check_channels(as, {Buffer({7.0}), Buffer({7.0})});
    }

    SECTION("operator==")
    {
        CHECK(as == make_stereo(LEFT, RIGHT));

        AudioStream mono(10.0, 1);
        mono << LEFT;
        CHECK_FALSE(as == mono);

        CHECK_FALSE(as == make_stereo(Buffer({1.0}), Buffer({1.0})));
        CHECK_FALSE(as == make_stereo(LEFT, LEFT));
    }

    SECTION("operator()(channel, index)")
    {
        CHECK(as(1, 2) == 3.0);
    }

    SECTION("begin(), end() iterate over the channels")
    {
        const AudioStream & stream = as;

        uint32 n = 0;

        for(auto itor = stream.begin(); itor != stream.end(); ++itor)
        {
            CHECK((*itor)->getLength() == 4);
            ++n;
        }

        CHECK(n == 2);
    }

    SECTION("setNChannels() starts with empty channels")
    {
        as.setNChannels(3);

        check_channels(as, {Buffer(), Buffer(), Buffer()});
    }
}

TEST_CASE("AudioStream operator<<")
{
    AudioStream as = make_stereo(Buffer({1.0}), Buffer({2.0}));

    SECTION("a mono stream is added to every channel")
    {
        AudioStream mono(10.0, 1);
        mono << Buffer({5.0});

        as << mono;

        check_channels(as, {Buffer({1.0, 5.0}), Buffer({2.0, 5.0})});
    }

    SECTION("channel by channel")
    {
        as << make_stereo(Buffer({3.0}), Buffer({4.0}));

        check_channels(as, {Buffer({1.0, 3.0}), Buffer({2.0, 4.0})});
    }

    SECTION("a Buffer or a float64 is added to every channel")
    {
        as << Buffer({3.0}) << 6.0;

        check_channels(as, {Buffer({1.0, 3.0, 6.0}), Buffer({2.0, 3.0, 6.0})});
    }
}

TEST_CASE("AudioStream arithmetic operators")
{
    AudioStream as = make_stereo(Buffer({2.0, 4.0}), Buffer({3.0, 9.0}));

    AudioStream mono(10.0, 1);
    mono << Buffer({2.0, 0.5});

    AudioStream other = make_stereo(Buffer({1.0, 2.0}), Buffer({3.0, 0.5}));

    SECTION("with a mono AudioStream, applied to every channel")
    {
        AudioStream a = as; a += mono;
        check_channels(a, {Buffer({4.0, 4.5}), Buffer({5.0, 9.5})});

        a = as; a -= mono;
        check_channels(a, {Buffer({0.0, 3.5}), Buffer({1.0, 8.5})});

        a = as; a *= mono;
        check_channels(a, {Buffer({4.0, 2.0}), Buffer({6.0, 4.5})});

        a = as; a /= mono;
        check_channels(a, {Buffer({1.0, 8.0}), Buffer({1.5, 18.0})});

        a = as; a ^= mono;
        check_channels(a, {Buffer({4.0, 2.0}), Buffer({9.0, 3.0})});
    }

    SECTION("with an AudioStream, channel by channel")
    {
        AudioStream a = as; a += other;
        check_channels(a, {Buffer({3.0, 6.0}), Buffer({6.0, 9.5})});

        a = as; a -= other;
        check_channels(a, {Buffer({1.0, 2.0}), Buffer({0.0, 8.5})});

        a = as; a *= other;
        check_channels(a, {Buffer({2.0, 8.0}), Buffer({9.0, 4.5})});

        a = as; a /= other;
        check_channels(a, {Buffer({2.0, 2.0}), Buffer({1.0, 18.0})});

        a = as; a ^= other;
        check_channels(a, {Buffer({2.0, 16.0}), Buffer({27.0, 3.0})});

        check_channels(as + other, {Buffer({3.0, 6.0}), Buffer({6.0, 9.5})});
    }

    SECTION("with a Buffer, applied to every channel")
    {
        Buffer b({2.0, 0.5});

        AudioStream a = as; a += b;
        check_channels(a, {Buffer({4.0, 4.5}), Buffer({5.0, 9.5})});

        a = as; a *= b;
        check_channels(a, {Buffer({4.0, 2.0}), Buffer({6.0, 4.5})});

        a = as; a /= b;
        check_channels(a, {Buffer({1.0, 8.0}), Buffer({1.5, 18.0})});

        a = as; a ^= b;
        check_channels(a, {Buffer({4.0, 2.0}), Buffer({9.0, 3.0})});

        check_channels(as * b, {Buffer({4.0, 2.0}), Buffer({6.0, 4.5})});
    }

    SECTION("with a float64")
    {
        AudioStream a = as; a += 1.0;
        check_channels(a, {Buffer({3.0, 5.0}), Buffer({4.0, 10.0})});

        a = as; a -= 1.0;
        check_channels(a, {Buffer({1.0, 3.0}), Buffer({2.0, 8.0})});

        a = as; a *= 2.0;
        check_channels(a, {Buffer({4.0, 8.0}), Buffer({6.0, 18.0})});

        a = as; a /= 2.0;
        check_channels(a, {Buffer({1.0, 2.0}), Buffer({1.5, 4.5})});

        a = as; a ^= 0.5;
        check_channels(a, {Buffer({std::sqrt(2.0), 2.0}), Buffer({std::sqrt(3.0), 3.0})});

        check_channels(as * 2.0, {Buffer({4.0, 8.0}), Buffer({6.0, 18.0})});
        check_channels(2.0 * as, {Buffer({4.0, 8.0}), Buffer({6.0, 18.0})});
        check_channels(as ^ 2.0, {Buffer({4.0, 16.0}), Buffer({9.0, 81.0})});
    }
}

TEST_CASE("AudioStream comparisons with a float64")
{
    AudioStream as = make_stereo(Buffer({1.0, 2.0}), Buffer({2.0, 3.0}));

    CHECK((as > 2.0)  == BooleanVectorVector({{false, false}, {false, true}}));
    CHECK((as >= 2.0) == BooleanVectorVector({{false, true}, {true, true}}));
    CHECK((as < 2.0)  == BooleanVectorVector({{true, false}, {false, false}}));
    CHECK((as <= 2.0) == BooleanVectorVector({{true, true}, {true, false}}));
    CHECK((as == 2.0) == BooleanVectorVector({{false, true}, {true, false}}));
    CHECK((as != 2.0) == BooleanVectorVector({{true, false}, {false, true}}));
}

TEST_CASE("AudioStream selections")
{
    AudioStream as = make_stereo(Buffer({1.0, 2.0, 3.0, 4.0}), Buffer({4.0, 3.0, 2.0, 1.0}));

    SECTION("operator()(BooleanVectorVector) with each operator")
    {
        as(as > 2.0) += 10.0;
        check_channels(as, {Buffer({1, 2, 13, 14}), Buffer({14, 13, 2, 1})});

        as(as > 10.0) -= 10.0;
        as(as > 2.0) *= 2.0;
        check_channels(as, {Buffer({1, 2, 6, 8}), Buffer({8, 6, 2, 1})});

        as(as > 5.0) /= 2.0;
        as(as < 2.0) ^= 3.0;
        check_channels(as, {Buffer({1, 2, 3, 4}), Buffer({4, 3, 2, 1})});

        as(as == 2.0) = 0.0;
        check_channels(as, {Buffer({1, 0, 3, 4}), Buffer({4, 3, 0, 1})});
    }

    SECTION("copies select the same samples")
    {
        AudioStreamSelection selection = as(as > 3.0);

        AudioStreamSelection copy(selection);
        copy = 0.0;

        AudioStreamSelection assigned = as(as < 0.0);
        assigned = selection;
        assigned += 9.0;

        AudioStreamSelection & same = assigned;
        assigned = same;

        check_channels(as, {Buffer({1, 2, 3, 9}), Buffer({9, 3, 2, 1})});
    }

    SECTION("select(start, stop) selects channels start to stop - 1")
    {
        AudioStream three(10.0, 3);
        three << Buffer({1.0, 2.0});

        // Channels 1 and 2, not 0 and 1.
        three.select(1, 3) = 0.0;

        check_channels(three, {Buffer({1, 2}), Buffer({0, 0}), Buffer({0, 0})});

        three.select(0, 1) += 5.0;

        check_channels(three, {Buffer({6, 7}), Buffer({0, 0}), Buffer({0, 0})});
    }
}

TEST_CASE("AudioStream operator<<(std::ostream)")
{
    std::ostringstream out;

    out << make_stereo(Buffer({1.0, 2.0}), Buffer({3.0}));

    // Shows up to 10 samples of each channel, as many as the shortest channel
    // has.  There is no newline after the samples.
    CHECK(out.str() ==
        "channel[0].length = 2\n"
        "channel[0] = 1 "
        "channel[1].length = 1\n"
        "channel[1] = 3 ");
}

TEST_CASE("AudioStream pad()")
{
    AudioStream as = make_stereo(Buffer({1.0, 2.0, 3.0}), Buffer({4.0}));

    check_channels(as.getPad(0.5), {Buffer({1.0, 2.0, 3.0}), Buffer({4.0, 0.5, 0.5})});

    as.pad();

    check_channels(as, {Buffer({1.0, 2.0, 3.0}), Buffer({4.0, 0.0, 0.0})});

    // Equal lengths are left alone.
    as.pad(9.0);

    check_channels(as, {Buffer({1.0, 2.0, 3.0}), Buffer({4.0, 0.0, 0.0})});
}

TEST_CASE("AudioStream pan()")
{
    AudioStream as = make_stereo(Buffer({1.0, 1.0}), Buffer({1.0, 1.0}));

    SECTION("pan(0.5) is 3/4 left, 1/4 right")
    {
        as.pan(0.5);

        check_channels(as, {Buffer({0.75, 0.75}), Buffer({0.25, 0.25})});
    }

    SECTION("pan(Buffer) pans each sample")
    {
        as.pan(Buffer({1.0, -1.0}));

        check_channels(as, {Buffer({1.0, 0.0}), Buffer({0.0, 1.0})});
    }
}

TEST_CASE("AudioStream substream()")
{
    // 10 samples at 10 Hz.
    Buffer ramp({0, 1, 2, 3, 4, 5, 6, 7, 8, 9});

    AudioStream as = make_stereo(ramp, 2.0 * ramp);

    SECTION("by index")
    {
        check_channels(as.substream(uint32(2), uint32(3)), {Buffer({2, 3, 4}), Buffer({4, 6, 8})});
        check_channels(as.substream(int32(7), int32(0)), {Buffer({7, 8, 9}), Buffer({14, 16, 18})});
    }

    SECTION("by time")
    {
        check_channels(as.substream(0.2, 0.3), {Buffer({2, 3, 4}), Buffer({4, 6, 8})});

        // A duration of 0, or past the end, is the rest of the stream.
        check_channels(as.substream(0.7, 0.0), {Buffer({7, 8, 9}), Buffer({14, 16, 18})});
        check_channels(as.substream(0.8, 5.0), {Buffer({8, 9}), Buffer({16, 18})});
    }
}

TEST_CASE("AudioStream transpose()")
{
    AudioStream as = make_stereo(Buffer({1.0, 2.0, 3.0}), Buffer({4.0, 5.0}));

    // Pads with 0 first, then 2 channels of 3 become 3 channels of 2.
    as.transpose();

    check_channels(as, {Buffer({1.0, 4.0}), Buffer({2.0, 5.0}), Buffer({3.0, 0.0})});
}

TEST_CASE("AudioStream write(), read()")
{
    AudioStream as = make_stereo(LEFT, RIGHT);

    SECTION("read(data, size) gets back what write() wrote")
    {
        std::string data = as.write();

        AudioStream read;
        read.read(data.data(), data.size());

        CHECK(read.getSampleRate() == 10.0);
        check_channels(read, {LEFT, RIGHT});
    }

    SECTION("read() throws on data that isn't an AudioStream")
    {
        std::string data = "not an audio stream";

        AudioStream read;

        CHECK_THROWS_AS(read.read(data.data(), data.size()), Exception);
    }

    // write()'s bytes: "audiostr", the float64 sample rate, the uint32
    // channel count, the channels.
    SECTION("read() throws when the bytes end before the channels")
    {
        std::string data = as.write();
        data.resize(data.size() - 8);

        AudioStream read(7.0, 1);

        CHECK_THROWS_AS(read.read(data.data(), data.size()), Exception);

        // Unchanged.
        CHECK(read.getSampleRate() == 7.0);
        CHECK(read.getNChannels() == 1);
    }

    SECTION("read() of a forged channel count throws, it doesn't make 4 billion")
    {
        std::string data = as.write();
        data.replace(16, 4, std::string(4, static_cast<char>(0xff)));

        AudioStream read;

        CHECK_THROWS_AS(read.read(data.data(), data.size()), Exception);
    }
}

TEST_CASE("AudioStream ones(), zeros(), rand()")
{
    // 0.4 seconds at 10 Hz is 4 samples.
    check_channels(AudioStream::ones(10.0, 2, 0.4), {Buffer::ones(4), Buffer::ones(4)});
    check_channels(AudioStream::zeros(10.0, 2, 0.4), {Buffer::zeros(4), Buffer::zeros(4)});

    AudioStream noise = AudioStream::rand(100.0, 2, 1.0);

    CHECK(noise.getNChannels() == 2);
    CHECK(noise.getLength() == 100);
    CHECK(noise.getMax() <= 1.0);
    CHECK(noise.getMin() >= -1.0);
    CHECK(noise.getMax() > noise.getMin());
}

#ifdef NSOUND_C_PYLAB

TEST_CASE("AudioStream plot()")
{
    using namespace plot_check;

    AudioStream as = make_stereo(LEFT, RIGHT);

    as.plot("the title");

    // A subplot per channel, against time in seconds, the title on top.
    auto axes = figure_axes();

    REQUIRE(axes.size() == 2);

    CHECK(text(axes[0].attr("title")) == "the title");
    CHECK(text(axes[1].attr("title")) == "");

    Buffer time = Sine(as.getSampleRate()).drawLine(
        as.getDuration(), 0.0, as.getDuration());

    for(uint32 c = 0; c < 2; ++c)
    {
        INFO("channel " << c);

        check_data(xdata(lines(axes[c])[0]), time);
        check_data(ydata(lines(axes[c])[0]), as[c]);
        CHECK(text(axes[c].attr("xaxis").attr("label")) == "Time (sec)");
    }

    pyplot().attr("close")("all");
}

#else

TEST_CASE("AudioStream plot() without matplotlib")
{
    CHECK_NOTHROW(make_stereo(LEFT, RIGHT).plot("title"));
}

#endif
