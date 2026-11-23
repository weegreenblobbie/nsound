//-----------------------------------------------------------------------------
//
//  Buffer tests.
//
//-----------------------------------------------------------------------------

#include <Nsound/Nsound.h>

#ifdef NSOUND_C_PYLAB
    #include "plot_check.h"
#endif

#include <Nsound/AudioStream.h>
#include <Nsound/Buffer.h>
#include <Nsound/BufferSelection.h>
#include <Nsound/Sine.h>
#include <Nsound/Wavefile.h>

#include "gold_check.h"

#include <catch2/catch_test_macros.hpp>

#include <cmath>
#include <sstream>
#include <string>

using namespace Nsound;

static const float64 GAMMA = 1.5e-12;

//! 0.0, 1.1, 2.2, ... up to 100.
static
Buffer
make_ramp()
{
    Buffer b(29);

    for(float64 i = 0.0; i < 100.0; i += 1.1)
    {
        b << i;
    }

    return b;
}

static void check_buffer_add();

TEST_CASE("Buffer operator<<(float64)")
{
    Buffer b1 = make_ramp();

    uint32 index = 0;

    for(float64 i = 0.0; i < 100.0; i += 1.1)
    {
        INFO("index " << index);
        CHECK(std::fabs(b1[index++] - i) <= GAMMA);
    }
}

TEST_CASE("Buffer copy and assignment")
{
    Buffer b1 = make_ramp();

    SECTION("Buffer(const Buffer &)")
    {
        Buffer b2(b1);

        CHECK_FALSE(b1 != b2);
        CHECK_FALSE(b2 != b1);
    }

    SECTION("operator=(const Buffer &)")
    {
        Buffer b3;
        b3 = b1;

        CHECK_FALSE(b3 != b1);
        CHECK_FALSE(b1 != b3);
    }
}

TEST_CASE("Buffer operator<<(const Buffer &) and operator[]")
{
    Buffer b1 = make_ramp();
    Buffer b3 = b1;

    Buffer b4(b3);
    b4 << b1;
    b3 << b1;

    CHECK(b4 == b3);
    CHECK(b3 == b4);

    REQUIRE(b4.getLength() == b3.getLength());

    for(uint32 i = 0; i < b4.getLength(); ++i)
    {
        INFO("index " << i);
        CHECK(std::fabs(b4[i] - b3[i]) <= GAMMA);
    }
}

TEST_CASE("Buffer operator+,-,*,/")
{
    Buffer b3 = make_ramp();
    b3 << make_ramp();

    Buffer b4 = b3;

    CHECK_FALSE(1 + b4 * 2 != 2 * b3 + 1);
    CHECK_FALSE(1 - b4 / 2 != -1 * b3 / 2 + 1);
    CHECK_FALSE(2.0 / b4 != 4.0 * (0.5 / b4));

    b3 = -1.0 * b4;
    b4 += b3;

    for(uint32 i = 0; i < b4.getLength(); ++i)
    {
        INFO("index " << i);
        CHECK(b4[i] == 0.0);
    }
}

TEST_CASE("Buffer abs()")
{
    Buffer b4;

    CHECK(b4.getLength() == 0);

    float64 sum = 1.0;

    for(uint32 i = 0; i < 100; ++i)
    {
        sum *= -1.0;
        b4 << sum;
    }

    b4.abs();

    CHECK(b4.getMin() >= 0.0);
}

TEST_CASE("Buffer reverse()")
{
    Buffer b4;

    CHECK(b4.getLength() == 0);

    for(float64 f = 0.0; f < 100.0; f += 1.0)
    {
        b4 << f;
    }

    b4.reverse();

    uint32 index = 0;

    for(float64 f = 99.0; f >= 0.0; f -= 1.0)
    {
        INFO("index " << index);
        CHECK(std::fabs(b4[index++] - f) <= GAMMA);
    }
}

TEST_CASE("Buffer reverse() of an empty and a one sample Buffer")
{
    // The last index of an empty Buffer wrapped round to the largest uint32.
    Buffer empty;

    empty.reverse();

    CHECK(empty.getLength() == 0);
    CHECK(empty.getReverse().getLength() == 0);

    Buffer one({3.0});

    one.reverse();

    check_close(one, Buffer({3.0}), 0.0);
}

TEST_CASE("Buffer subbuffer()")
{
    // 0, 1, 2, ... 200
    Buffer b3;

    const uint32 chunk = 100;

    for(uint32 i = 0; i < chunk * 2 + 1; ++i)
    {
        b3 << i;
    }

    CHECK(b3.subbuffer(0, 99).getLength() == 99);
    CHECK(b3.subbuffer(0, chunk).getLength() == chunk);

    // Asking for more than there is gives what there is.
    CHECK(b3.subbuffer(0, b3.getLength() + 1).getLength() == b3.getLength());

    for(uint32 i = 0; i < 10; ++i)
    {
        INFO("offset " << i);
        CHECK(b3.subbuffer(i, chunk + 10).getLength() == chunk + 10);
    }

    Buffer subbuf = b3.subbuffer(0, 10);

    for(uint32 i = 0; i < 10; ++i)
    {
        CHECK(static_cast<uint32>(subbuf[i]) == i);
    }

    // With an offset.
    subbuf = b3.subbuffer(3, 3);

    for(uint32 i = 0; i < 3; ++i)
    {
        CHECK(static_cast<uint32>(subbuf[i]) == i + 3);
    }

    // The default n_samples is the rest of the buffer.
    CHECK(b3.subbuffer(3).getLength() == b3.getLength() - 3);

    subbuf = b3.subbuffer(0);

    CHECK(subbuf.getLength() == b3.getLength());

    for(uint32 i = 0; i < 10; ++i)
    {
        CHECK(static_cast<uint32>(subbuf[i]) == i);
    }

    subbuf = b3.subbuffer(5, 5);

    REQUIRE(subbuf.getLength() == 5);

    for(uint32 i = 0; i < 5; ++i)
    {
        CHECK(static_cast<uint32>(subbuf[i]) == i + 5);
    }
}

TEST_CASE("Buffer smooth()")
{
    // 99, 98, ... 0 followed by the noise file, as the old test had it.
    Buffer b4;

    for(float64 f = 0.0; f < 100.0; f += 1.0)
    {
        b4 << f;
    }

    b4.reverse();
    b4 << "gold/Filter_noise.wav";

    uint32 b4_length = b4.getLength();
    float64 b4_max = b4.getMax();

    b4.smooth(1, 16);

    CHECK(b4.getLength() == b4_length);
    CHECK(b4.getMax() <= b4_max);
}

TEST_CASE("Buffer pow()")
{
    Buffer b4 = 2 * Buffer::ones(5);

    b4 ^= 2.0;

    for(uint32 i = 0; i < 5; ++i)
    {
        CHECK(b4[i] == 4.0);
    }

    b4 ^= 0.50;

    for(uint32 i = 0; i < 5; ++i)
    {
        CHECK(b4[i] == 2.0);
    }

    b4 = (b4 ^ 2.0) + (b4 ^ 2.0);

    for(uint32 i = 0; i < 5; ++i)
    {
        CHECK(b4[i] == 8.0);
    }
}

TEST_CASE("Buffer operators with different lengths")
{
    Sine sine(100);

    Buffer b5 = sine.drawLine(1.0, 2.0, 2.0);
    Buffer b6 = sine.drawLine(2.0, 3.0, 3.0);

    SECTION("short * long is as long as the short one")
    {
        Buffer result = b5 * b6;

        REQUIRE(result.getLength() == b5.getLength());

        for(uint32 i = 0; i < result.getLength(); ++i)
        {
            INFO("index " << i);
            CHECK(result[i] == 6.0);
        }
    }

    SECTION("long * short keeps the long tail")
    {
        Buffer result = b6 * b5;

        REQUIRE(result.getLength() == b6.getLength());

        for(uint32 i = 0; i < b5.getLength(); ++i)
        {
            INFO("index " << i);
            CHECK(result[i] == 6.0);
        }

        for(uint32 i = b5.getLength(); i < result.getLength(); ++i)
        {
            INFO("index " << i);
            CHECK(result[i] == 3.0);
        }
    }
}

TEST_CASE("Buffer add()")
{
    check_buffer_add();
}

TEST_CASE("Buffer circular_iterator")
{
    Sine sine(100);

    Buffer b5 = sine.drawLine(1.0, 1.0, 100.0);

    Buffer::circular_iterator itor = b5.cbegin();

    for(uint32 i = 0; i < b5.getLength(); ++i)
    {
        ++itor;
    }

    CHECK(itor == b5.cbegin());

    itor.reset();

    for(uint32 i = 0; i < b5.getLength(); ++i)
    {
        --itor;
    }

    CHECK(itor == b5.cbegin());

    itor.reset();

    for(uint32 i = 0; i < 3; ++i)
    {
        ++itor;
    }

    CHECK(itor == (b5.cbegin() + 3));

    itor.reset();

    for(uint32 i = 0; i < 3; ++i)
    {
        --itor;
    }

    CHECK(itor == (b5.cbegin() - 3));
}

TEST_CASE("Buffer advanced operators")
{
    Sine sine(100);

    Buffer b7 = sine.generate(1.0, 2.0);

    SECTION("clip with operator()(BooleanVector) =")
    {
        Buffer data = b7;

        data(data > 0.5) = 0.5;
        data(data < -0.5) = -0.5;

        check_gold(data, "gold/Buffer_out1.wav", GAMMA);
    }

    SECTION("scale and shift with operator()(BooleanVector) *= +=")
    {
        Buffer data = b7;

        BooleanVector bv1 = data > 0.5;
        BooleanVector bv2 = data < -0.5;

        data(bv1) *= 0.1;
        data(bv2) *= 0.1;
        data(bv1) += 0.45;
        data(bv2) -= 0.45;

        check_gold(data, "gold/Buffer_out2.wav", GAMMA);
    }
}

TEST_CASE("Buffer(const FloatVector &)")
{
    FloatVector v = {1.5, -2.0, 3.25};

    check_close(Buffer(v), Buffer({1.5, -2.0, 3.25}), 0.0);
}

TEST_CASE("Buffer argmax(), argmin()")
{
    Buffer b({3.0, 7.0, -2.0, 7.0, 1.0});

    // The first index of the max or min.
    CHECK(b.argmax() == 1);
    CHECK(b.argmin() == 2);
}

TEST_CASE("Buffer exp(), log(), sqrt(), round()")
{
    SECTION("exp()")
    {
        Buffer b({0.0, 1.0});
        b.exp();

        check_close(b, Buffer({1.0, 2.718281828459045}), 1e-15);
    }

    SECTION("log() clamps values below 1e-9")
    {
        // e, M_E isn't defined by MSVC.
        Buffer b({1.0, 2.718281828459045, 0.0, -1.0});
        b.log();

        // ln(1e-9) = -20.72326583694641
        check_close(b, Buffer({0.0, 1.0, -20.72326583694641, -20.72326583694641}), 1e-14);
    }

    SECTION("sqrt() keeps the sign")
    {
        Buffer b({4.0, -9.0, 0.0});

        check_close(b.getSqrt(), Buffer({2.0, -3.0, 0.0}), 0.0);

        // getSqrt() returns a copy.
        check_close(b, Buffer({4.0, -9.0, 0.0}), 0.0);

        b.sqrt();

        check_close(b, Buffer({2.0, -3.0, 0.0}), 0.0);
    }

    SECTION("round() rounds halves up")
    {
        Buffer b({1.4, 1.5, -1.5, -0.4});
        b.round();

        check_close(b, Buffer({1.0, 2.0, -1.0, 0.0}), 0.0);
    }
}

TEST_CASE("Buffer convolve()")
{
    Buffer x({1.0, 2.0, 3.0});
    Buffer h({1.0, 1.0});

    // The result is one sample longer than a full convolution (N + M - 1),
    // the last sample is always 0.
    Buffer expected({1.0, 3.0, 5.0, 3.0, 0.0});

    check_close(x.getConvolve(h), expected, 0.0);

    x.convolve(h);

    check_close(x, expected, 0.0);
}

TEST_CASE("Buffer downSample(), upSample()")
{
    Sine sine(100);

    Buffer input = sine.generate(1.0, 3.0);

    Buffer down = input;
    down.downSample(2);

    CHECK(down.getLength() == input.getLength() / 2);
    check_close(down, input.getResample(uint32(1), uint32(2)), 0.0);
    check_close(input.getDownSample(2), input.getResample(uint32(1), uint32(2)), 0.0);

    Buffer up = input;
    up.upSample(2);

    CHECK(up.getLength() == input.getLength() * 2);
    check_close(up, input.getResample(uint32(2), uint32(1)), 0.0);
    check_close(input.getUpSample(2), input.getResample(uint32(2), uint32(1)), 0.0);
}

TEST_CASE("Buffer findPeaks()")
{
    //           0    1    2    3    4    5    6    7
    Buffer b({0.0, 3.0, 1.0, 5.0, 2.0, 0.0, 4.0, 0.0});

    SECTION("neighbors only")
    {
        CHECK(b.findPeaks() == Uint32Vector({1, 3, 6}));
    }

    SECTION("min_height")
    {
        CHECK(b.findPeaks(0, 3.5) == Uint32Vector({3, 6}));
    }

    SECTION("window of 5 samples")
    {
        // Index 1 isn't the highest within 2 samples, index 3 is bigger.
        CHECK(b.findPeaks(5) == Uint32Vector({3, 6}));
    }
}

TEST_CASE("Buffer limit()")
{
    SECTION("limit(min, max)")
    {
        Buffer b({-2.0, 0.5, 3.0});
        b.limit(-1.0, 1.0);

        check_close(b, Buffer({-1.0, 0.5, 1.0}), 0.0);
    }

    SECTION("limit(Buffer, Buffer) repeats the limits")
    {
        Buffer b({-2.0, -0.5, 3.0, 0.2});

        // min is -1, 0, -1, 0 and max is 1 throughout.
        b.limit(Buffer({-1.0, 0.0}), Buffer({1.0}));

        check_close(b, Buffer({-1.0, 0.0, 1.0, 0.2}), 0.0);
    }
}

TEST_CASE("Buffer getSum(), getMean(), getStd(), zNorm()")
{
    Buffer b({2.0, 4.0, 4.0, 4.0, 5.0, 5.0, 7.0, 9.0});

    CHECK(b.getSum() == 40.0);
    CHECK(b.getMean() == 5.0);

    // The population standard deviation.
    CHECK(b.getStd() == 2.0);

    b.zNorm();

    check_close(b, Buffer({-1.5, -0.5, -0.5, -0.5, 0.0, 0.0, 1.0, 2.0}), 1e-15);
}

TEST_CASE("Buffer mul(buffer, offset, n_samples)")
{
    Buffer b({2.0, 3.0, 4.0});

    Buffer a = Buffer::ones(6);

    SECTION("all of b at an offset")
    {
        a.mul(b, 1, 0);

        check_close(a, Buffer({1.0, 2.0, 3.0, 4.0, 1.0, 1.0}), 0.0);
    }

    SECTION("stops at the end of a")
    {
        a.mul(b, 4, 0);

        check_close(a, Buffer({1.0, 1.0, 1.0, 1.0, 2.0, 3.0}), 0.0);
    }

    SECTION("n_samples of b")
    {
        a.mul(b, 1, 2);

        check_close(a, Buffer({1.0, 2.0, 3.0, 1.0, 1.0, 1.0}), 0.0);
    }

    SECTION("n_samples longer than b")
    {
        a.mul(b, 0, 5);

        check_close(a, Buffer({2.0, 3.0, 4.0, 1.0, 1.0, 1.0}), 0.0);
    }
}

TEST_CASE("Buffer getSignalEnergy()")
{
    Buffer b({1.0, -2.0, 3.0, -4.0});

    // The mean magnitude over a window of 2 samples, past the end counts as 0.
    check_close(b.getSignalEnergy(2), Buffer({1.5, 2.5, 3.5, 2.0}), 0.0);
}

TEST_CASE("Buffer operator<<(AudioStream) adds the mono mix")
{
    AudioStream as(100, 2);

    as[0] = Buffer({1.0, 3.0});
    as[1] = Buffer({3.0, 5.0});

    Buffer b({9.0});
    b << as;

    check_close(b, Buffer({9.0, 2.0, 4.0}), 0.0);
}

TEST_CASE("Buffer element-wise and scalar operators")
{
    SECTION("operator/=(Buffer) stops at the shorter buffer")
    {
        Buffer b({8.0, 9.0, 2.0});
        b /= Buffer({2.0, 3.0});

        check_close(b, Buffer({4.0, 3.0, 2.0}), 0.0);
    }

    SECTION("operator^=(Buffer) stops at the shorter buffer")
    {
        Buffer b({3.0, 16.0, 5.0});
        b ^= Buffer({2.0, 0.5});

        check_close(b, Buffer({9.0, 4.0, 5.0}), 0.0);
    }

    SECTION("operator-=(double), operator-(Buffer, double)")
    {
        Buffer b({1.0, 2.5});

        check_close(b - 1.5, Buffer({-0.5, 1.0}), 0.0);

        b -= 1.0;

        check_close(b, Buffer({0.0, 1.5}), 0.0);
    }

    SECTION("comparisons with a double")
    {
        Buffer b({1.0, 2.0, 3.0});

        CHECK((b >= 2.0) == BooleanVector({false, true, true}));
        CHECK((b <= 2.0) == BooleanVector({true, true, false}));
        CHECK((b == 2.0) == BooleanVector({false, true, false}));
        CHECK((b != 2.0) == BooleanVector({true, false, true}));
    }
}

TEST_CASE("Buffer operator<<(std::ostream)")
{
    std::ostringstream out;

    out << Buffer({1.5, -2.0});

    CHECK(out.str() == "1.5\n-2\n");
}

TEST_CASE("Buffer writeWavefile(), readWavefile()")
{
    Wavefile::setDefaultSampleSize(64);
    Wavefile::setIEEEFloat(true);

    Buffer b({0.25, -0.5, 0.75});

    b.writeWavefile("test_buffer.wav");

    Buffer read;
    read.readWavefile("test_buffer.wav");

    check_close(read, b, 0.0);
}

TEST_CASE("Buffer getResample(factor)")
{
    Sine sine(100);

    Buffer input = sine.generate(1.0, 3.0);

    SECTION("a factor of 1.5 resamples by 3 / 2")
    {
        check_close(input.getResample(1.5), input.getResample(uint32(3), uint32(2)), 0.0);

        Buffer b = input;
        b.resample(1.5);

        check_close(b, input.getResample(uint32(3), uint32(2)), 0.0);
    }

    SECTION("a constant factor Buffer resamples like the constant")
    {
        Buffer factor = 0.5 * Buffer::ones(input.getLength());

        check_close(input.getResample(factor), input.getResample(0.5), 0.0);

        Buffer b = input;
        b.resample(factor);

        check_close(b, input.getResample(0.5), 0.0);
    }
}

TEST_CASE("Buffer select() and BufferSelection")
{
    Buffer b({0.0, 1.0, 2.0, 3.0, 4.0, 5.0});

    SECTION("select(start, stop) includes both ends")
    {
        b.select(1, 3) = 9.0;

        check_close(b, Buffer({0.0, 9.0, 9.0, 9.0, 4.0, 5.0}), 0.0);
    }

    SECTION("operator/=, operator^=")
    {
        b.select(4, 5) /= 2.0;
        b.select(1, 2) ^= 2.0;

        check_close(b, Buffer({0.0, 1.0, 4.0, 3.0, 2.0, 2.5}), 0.0);
    }

    SECTION("copies select the same samples")
    {
        BufferSelection selection = b.select(0, 1);

        BufferSelection copy(selection);
        copy += 10.0;

        BufferSelection assigned = b.select(5, 5);
        assigned = selection;
        assigned *= 2.0;

        check_close(b, Buffer({20.0, 22.0, 2.0, 3.0, 4.0, 5.0}), 0.0);
    }
}

TEST_CASE("Buffer write(), read()")
{
    Buffer b({1.5, -2.0, 3.25});

    SECTION("read(data, size) gets back what write() wrote")
    {
        std::string data = b.write();

        Buffer read;
        read.read(data.data(), data.size());

        check_close(read, b, 0.0);
    }

    SECTION("read(istream) gets back what write(ostream) wrote")
    {
        std::stringstream stream;
        b.write(stream);

        Buffer read;
        read.read(stream);

        check_close(read, b, 0.0);
    }

    SECTION("read() throws on data that isn't a Buffer")
    {
        std::string data = "not a buffer";

        Buffer read;

        CHECK_THROWS_AS(read.read(data.data(), data.size()), Exception);
    }

    // write()'s bytes: "buff", the uint32 sample count, the samples.
    SECTION("read() throws when the bytes end before the samples")
    {
        std::string data = b.write();
        data.resize(data.size() - 8);

        Buffer read({7.0});

        CHECK_THROWS_AS(read.read(data.data(), data.size()), Exception);

        // Unchanged.
        check_close(read, Buffer({7.0}), 0.0);
    }

    SECTION("read() of a forged sample count throws, it doesn't reserve 32 GB")
    {
        std::string data = b.write();
        data.replace(4, 4, std::string(4, static_cast<char>(0xff)));

        Buffer read;

        CHECK_THROWS_AS(read.read(data.data(), data.size()), Exception);
    }
}

TEST_CASE("Buffer speedUp()")
{
    Buffer b({0.0, 1.0, 2.0, 3.0, 4.0, 5.0});

    SECTION("speedUp(2.0) keeps every other sample")
    {
        check_close(b.getSpeedUp(2.0), Buffer({0.0, 2.0, 4.0}), 0.0);

        b.speedUp(2.0);

        check_close(b, Buffer({0.0, 2.0, 4.0}), 0.0);
    }

    SECTION("speedUp(Buffer) steps by each value in turn")
    {
        // Steps of 1, 2, 1, 2, ... from index 0: 0, 1, 3, 4.
        Buffer steps({1.0, 2.0});

        check_close(b.getSpeedUp(steps), Buffer({0.0, 1.0, 3.0, 4.0}), 0.0);

        b.speedUp(steps);

        check_close(b, Buffer({0.0, 1.0, 3.0, 4.0}), 0.0);
    }
}

TEST_CASE("Buffer zeros(), rand()")
{
    check_close(Buffer::zeros(4), Buffer({0.0, 0.0, 0.0, 0.0}), 0.0);

    Buffer noise = Buffer::rand(100);

    CHECK(noise.getLength() == 100);
    CHECK(noise.getMax() <= 1.0);
    CHECK(noise.getMin() >= -1.0);
    CHECK(noise.getMax() > noise.getMin());
}

TEST_CASE("Buffer edge cases")
{
    Buffer b({1.0, 2.0, 3.0});

    SECTION("assigning a Buffer to itself keeps it")
    {
        Buffer & same = b;
        b = same;

        check_close(b, Buffer({1.0, 2.0, 3.0}), 0.0);
    }

    SECTION("b << b appends a copy of itself")
    {
        b << b;

        check_close(b, Buffer({1.0, 2.0, 3.0, 1.0, 2.0, 3.0}), 0.0);
    }

    SECTION("operator== and operator!= with different lengths or values")
    {
        CHECK_FALSE(b == Buffer({1.0, 2.0}));
        CHECK_FALSE(b == Buffer({1.0, 2.0, 4.0}));
        CHECK(b != Buffer({1.0, 2.0}));
        CHECK(b != Buffer({1.0, 2.0, 4.0}));
    }

    SECTION("subbuffer() past the end is empty")
    {
        CHECK(b.subbuffer(3, 2).getLength() == 0);
    }

    SECTION("smooth() with fewer than 2 samples to average does nothing")
    {
        b.smooth(1, 1);

        check_close(b, Buffer({1.0, 2.0, 3.0}), 0.0);
    }

    SECTION("getResample(1, 1) is a copy")
    {
        check_close(b.getResample(uint32(1), uint32(1)), b, 0.0);
    }

    SECTION("getResample() of a long Buffer works in windows")
    {
        // Over 40000 samples are resampled 2048 at a time.
        Buffer noise = Buffer::rand(40100);

        Buffer up = noise.getResample(uint32(2), uint32(1));

        CHECK(up.getLength() == 2 * noise.getLength());
        CHECK(up.getMaxMagnitude() > 0.1);
    }

    SECTION("assigning a BufferSelection to itself keeps it")
    {
        BufferSelection selection = b.select(0, 0);
        BufferSelection & same = selection;

        selection = same;
        selection = 5.0;

        check_close(b, Buffer({5.0, 2.0, 3.0}), 0.0);
    }
}

#ifdef NSOUND_C_PYLAB

TEST_CASE("Buffer plot()")
{
    using namespace plot_check;

    Buffer b({1.0, 2.0, 4.0});

    b.plot("the title");

    // A new figure with the samples and the title.
    REQUIRE(figure_axes().size() == 1);

    auto axes = gca();

    CHECK(text(axes.attr("title")) == "the title");
    check_data(ydata(lines(axes)[0]), b);

    pyplot().attr("close")("all");
}

#else

TEST_CASE("Buffer plot() without matplotlib")
{
    CHECK_NOTHROW(Buffer({1.0, 2.0}).plot("title"));
}

#endif

// The brute force add() test below is the old one, unchanged, except its
// reportError() is now a Catch2 failure that names the arguments.
#define reportError(file, line, n_samples, offset, a_length, b_length, message) \
    FAIL((message) << ": n_samples = " << (n_samples)                         \
         << ", offset = " << (offset)                                          \
         << ", a_length = " << (a_length)                                      \
         << ", b_length = " << (b_length))

//-----------------------------------------------------------------------------
// Exhaustively test the Buffer::add() method
//
//  There are 11 cases to consider.
//
//  c = a + b
//
//  Case 1: n_samples == 0
//          && a_length < offset
//
//           [aaaaaaa]
//         + |--offset--|[bbbbbbbbbbbbbbbbbbbbbb]
//  result = |aaaaaaaa---bbbbbbbbbbbbbbbbbbbbbbb|
//
//  Case 2: n_samples != 0
//          && a_length < offset
//          && n_samples <= b_length
//
//           [aaaaaaa]
//         + |--offset--|[bbbbbbbbbbbbbbbbbbbbbb]
//                       |---n_samples---|
//  result = |aaaaaaaa---bbbbbbbbbbbbbbbb|
//
//  Case 3: n_samples != 0
//          && a_length < offset
//          && n_samples > b_length
//
//           [aaaaaaa]
//         + |--offset--|[bbbbbbbbbbbbbbbbbbbbbb]
//                       |---n_samples----------------|
//  result = |aaaaaaaa---bbbbbbbbbbbbbbbbbbbbbbbb-----|
//
//
//  Case 4: n_samples == 0
//          && a_length >= offset
//          && a_length < offset + b_length
//
//           [aaaaaaaaaaaaaaaaaaaaaaaaaaaa]
//         + |--offset--|[bbbbbbbbbbbbbbbbbbbbbb]
//  result = |aaaaaaaaaaaccccccccccccccccccbbbbb|
//
//
//  Case 5: n_samples != 0
//          && a_length >= offset
//          && n_samples <= b_length
//          && a_length <= offset + n_samples
//
//           [aaaaaaaaaaaaaaaaaaaa]
//         + |--offset--|[bbbbbbbbbbbbbbbbbbbbbb]
//                      |----n_samples----|
//  result = |aaaaaaaaaacccccccccccbbbbbbb|
//
//
//  Case 6: n_samples != 0
//          && a_length >= offset
//          && n_samples < b_length
//          && a_length > offset + n_samples
//
//           [aaaaaaaaaaaaaaaaaaaaaaaaaaaa]
//         + |--offset--|[bbbbbbbbbbbbbbbbbbbbbb]
//                      |--n_samples--|
//  result = |aaaaaaaaaacccccccccccccccaaa|
//
//  Case 7: n_samples != 0
//          && a_length >= offset
//          && n_samples > b_length
//          && a_length <= offset + b_length
//
//           [aaaaaaaaaaaaaaaaaaaa]
//         + |--offset--|[bbbbbbbbbbbbbbbbbbbbbb]
//                      |----n_samples---------------|
//  result = |aaaaaaaaaaaccccccccccbbbbbbbbbbbbbb----|
//
//
//  Case 8: n_samples == 0
//          && a_length >= offset + b_length
//
//           [aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa]
//         + |--offset--|[bbbbbbbbbbbbbbbbbbbbbb]
//  result = |aaaaaaaaaaaccccccccccccccccccccccccaaaaaa|
//
//  Case 9:  n_samples != 0
//           && a_length >= offset + n_samples
//           && n_samples < b_length
//
//           [aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa]
//         + |--offset--|[bbbbbbbbbbbbbbbbbbbb]
//                      |----n_samples----|
//  result = |aaaaaaaaaaaccccccccccccccccccaaaaaaaaaaaa|
//
//  Case 10: n_samples != 0
//           && a_length >= offset + n_samples
//           && n_samples >= b_length
//
//           [aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa]
//         + |--offset--|[bbbbbbbbbbbbbbbbbbbbb]
//                      |----n_samples---------------|
//  result = |aaaaaaaaaaacccccccccccccccccccccccaaaaaaa|
//
//  Case 11: n_samples != 0
//           && a_length > offset + b_length
//           && a_length < offset + n_samples
//
//           [aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa]
//         + |--offset--|[bbbbbbbbbbbbbbbbbbbbb]
//                      |----n_samples----------------------|
//  result = |aaaaaaaaaaacccccccccccccccccccccccaaaaaaaa----|
//
//-----------------------------------------------------------------------------
static
void
check_buffer_add()
{
    bool final_case[12];

    for(uint32 i = 0; i < 12; ++i)
    {
        final_case[i] = false;
    }

    for(uint32 run = 0; run < 2; ++run)
    {
        Buffer b;
        Buffer a;

        uint32 test_size = 15;

        if(run == 0)
        {
            for(uint32 i = 0; i < test_size; ++i)
            {
                b << 1.0;
            }

            a = b;

            // Make buffer a longer.
            for(uint32 i = 0; i < test_size * 5 + 7; ++i)
            {
                a << 1.0;
            }
        }
        else
        {
            for(uint32 i = 0; i < test_size; ++i)
            {
                a << 1.0;
            }

            b = a;

            // Make buffer a longer.
            for(uint32 i = 0; i < test_size * 5 + 7; ++i)
            {
                b << 1.0;
            }
        }

        uint32 a_length = a.getLength();
        uint32 b_length = b.getLength();

        // Brute force test the add() method.

        uint32 offset_end = a_length + test_size * 6 + 7;

        for(uint32 n_samples = 0; n_samples < offset_end; ++n_samples)
        {
            for(uint32 offset = 0; offset < offset_end; ++offset)
            {
                bool current_run[12];

                for(uint32 i = 0; i < 12; ++i)
                {
                    current_run[i] = false;
                }

                Buffer result(a);
                result.add(b,offset,n_samples);

                uint32 result_length = result.getLength();

                ///////////////////////////////////////////////////////////////
                // Case 1:
                //      result = [aaaaaaa]
                //             + |--offset--|[bbbbbbbbbbbbbbbbbbbbbb]
                //             = |----------------------------------|
                ///////////////////////////////////////////////////////////////
                if(n_samples == 0 && a_length < offset)
                {
                    current_run[0] = true;

                    // Verify result length
                    if(result_length != offset + b_length)
                    {
                        reportError(__FILE__,__LINE__,
                                        n_samples, offset, a_length, b_length,
                                        "result length error");
                    }

                    // Check prefix values.
                    for(uint32 i = 0; i < a_length; ++i)
                    {
                        if(result[i] != 1.0)
                        {
                            reportError(__FILE__,__LINE__,
                                        n_samples, offset, a_length, b_length,
                                        "result prefix error");
                        }
                    }

                    // Check gap values
                    for(uint32 i = a_length + 1; i < offset; ++i)
                    {
                        if(result[i] != 0.0)
                        {
                            reportError(__FILE__,__LINE__,
                                        n_samples, offset, a_length, b_length,
                                        "result gap error");
                        }
                    }

                    // Check b values
                    for(uint32 i = offset + 1; i < result_length; ++i)
                    {
                        if(result[i] != 1.0)
                        {
                            reportError(__FILE__,__LINE__,
                                        n_samples, offset, a_length, b_length,
                                        "result b value error");
                        }
                    }
                }

                ///////////////////////////////////////////////////////////////
                //  Case 2: n_samples != 0
                //          && a_length < offset
                //          && n_samples <= b_length
                //
                //  result = [aaaaaaa]
                //         + |--offset--|[bbbbbbbbbbbbbbbbbbbbbb]
                //                       |---n_samples---|
                //         = |---------------------------|
                ///////////////////////////////////////////////////////////////
                else if(n_samples != 0
                            && a_length < offset
                            && n_samples <= b_length)
                {
                    current_run[1] = true;
                    // Verify result length
                    if(result_length != offset + n_samples)
                    {
                        reportError(__FILE__,__LINE__,
                                        n_samples, offset, a_length, b_length,
                                        "result length error");
                    }

                    // Check prefix values.
                    for(uint32 i = 0; i < a_length; ++i)
                    {
                        if(result[i] != 1.0)
                        {
                            reportError(__FILE__,__LINE__,
                                        n_samples, offset, a_length, b_length,
                                        "result prefix error");
                        }
                    }

                    // Check gap values
                    for(uint32 i = a_length + 1; i < offset; ++i)
                    {
                        if(result[i] != 0.0)
                        {
                            reportError(__FILE__,__LINE__,
                                        n_samples, offset, a_length, b_length,
                                        "result gap error");
                        }
                    }

                    // Check b values
                    for(uint32 i = offset + 1; i < result_length; ++i)
                    {
                        if(result[i] != 1.0)
                        {
                            reportError(__FILE__,__LINE__,
                                        n_samples, offset, a_length, b_length,
                                        "result b value error");
                        }
                    }
                }

                ///////////////////////////////////////////////////////////////
                //  Case 3: n_samples != 0
                //          && a_length < offset
                //          && n_samples > b_length
                //
                //  result = [aaaaaaa]
                //         + |--offset--|[bbbbbbbbbbbbbbbbbbbbbb]
                //                       |---n_samples----------------|
                //         = |----------------------------------------|
                ///////////////////////////////////////////////////////////////
                else if(n_samples != 0
                            && a_length < offset
                            && n_samples > b_length)
                {
                    current_run[2] = true;

                    // Verify result length
                    if(result_length != offset + n_samples)
                    {
                        reportError(__FILE__,__LINE__,
                                        n_samples, offset, a_length, b_length,
                                        "result length error");
                    }

                    // Check prefix values.
                    for(uint32 i = 0; i < a_length; ++i)
                    {
                        if(result[i] != 1.0)
                        {
                            reportError(__FILE__,__LINE__,
                                        n_samples, offset, a_length, b_length,
                                        "result prefix error");
                        }
                    }

                    // Check gap values
                    for(uint32 i = a_length + 1; i < offset; ++i)
                    {
                        if(result[i] != 0.0)
                        {
                            reportError(__FILE__,__LINE__,
                                        n_samples, offset, a_length, b_length,
                                        "result gap error");
                        }
                    }

                    // Check b values
                    for(uint32 i = offset + 1; i < offset + b_length; ++i)
                    {
                        if(result[i] != 1.0)
                        {
                            reportError(__FILE__,__LINE__,
                                        n_samples, offset, a_length, b_length,
                                        "result b value error");
                        }
                    }

                    // Check tail values
                    for(uint32 i = offset + b_length + 1; i < result_length; ++i)
                    {
                        if(result[i] != 0.0)
                        {
                            reportError(__FILE__,__LINE__,
                                        n_samples, offset, a_length, b_length,
                                        "result tail error");
                        }
                    }
                }

                ///////////////////////////////////////////////////////////////
                //  Case 4: n_samples == 0
                //          && a_length >= offset
                //          && a_length < offset + b_length
                //
                //  result = [aaaaaaaaaaaaaaaaaaaaaaaaaaaa]
                //         + |--offset--|[bbbbbbbbbbbbbbbbbbbbbb]
                //         = |----------------------------------|
                ///////////////////////////////////////////////////////////////
                else if(n_samples == 0
                            && a_length >= offset
                            && a_length < offset + b_length)
                {
                    current_run[3] = true;

                    // Verify result length
                    if(result_length != offset + b_length)
                    {
                        reportError(__FILE__,__LINE__,
                                        n_samples, offset, a_length, b_length,
                                        "result length error");
                    }

                    // Check prefix values.
                    for(uint32 i = 0; i < offset; ++i)
                    {
                        if(result[i] != 1.0)
                        {
                            reportError(__FILE__,__LINE__,
                                        n_samples, offset, a_length, b_length,
                                        "result prefix error");
                        }
                    }

                    // Check overlap values
                    for(uint32 i = offset + 1; i < a_length; ++i)
                    {
                        if(result[i] != 2.0)
                        {
                            reportError(__FILE__,__LINE__,
                                        n_samples, offset, a_length, b_length,
                                        "result overlap error");
                        }
                    }

                    // Check tail values
                    for(uint32 i = a_length + 1; i < result_length; ++i)
                    {
                        if(result[i] != 1.0)
                        {
                            reportError(__FILE__,__LINE__,
                                        n_samples, offset, a_length, b_length,
                                        "result tail error");
                        }
                    }
                }

                ///////////////////////////////////////////////////////////////
                //  Case 5: n_samples != 0
                //          && a_length >= offset
                //          && n_samples <= b_length
                //          && a_length <= offset + n_samples
                //
                //  result = [aaaaaaaaaaaaaaaaaaaa]
                //         + |--offset--|[bbbbbbbbbbbbbbbbbbbbbb]
                //                      |----n_samples----|
                //         = |----------------------------|
                ///////////////////////////////////////////////////////////////
                else if(n_samples != 0
                            && a_length >= offset
                            && a_length <= offset + n_samples
                            && n_samples <= b_length)
                {
                    current_run[4] = true;

                    // Verify result length
                    if(result_length != offset + n_samples)
                    {
                        reportError(__FILE__,__LINE__,
                                        n_samples, offset, a_length, b_length,
                                        "result length error");
                    }

                    // Check prefix values.
                    for(uint32 i = 0; i < offset; ++i)
                    {
                        if(result[i] != 1.0)
                        {
                            reportError(__FILE__,__LINE__,
                                        n_samples, offset, a_length, b_length,
                                        "result prefix error");
                        }
                    }

                    // Check overlap values
                    for(uint32 i = offset + 1; i < a_length; ++i)
                    {
                        if(result[i] != 2.0)
                        {
                            reportError(__FILE__,__LINE__,
                                        n_samples, offset, a_length, b_length,
                                        "result overlap error");
                        }
                    }

                    // Check tail values
                    for(uint32 i = a_length + 1; i < result_length; ++i)
                    {
                        if(result[i] != 1.0)
                        {
                            reportError(__FILE__,__LINE__,
                                        n_samples, offset, a_length, b_length,
                                        "result tail error");
                        }
                    }
                }
                ///////////////////////////////////////////////////////////////
                //  Case 6: n_samples != 0
                //          && a_length >= offset
                //          && n_samples < b_length
                //          && a_length > offset + n_samples
                //
                //  result = [aaaaaaaaaaaaaaaaaaaaaaaaaaaa]
                //         + |--offset--|[bbbbbbbbbbbbbbbbbbbbbb]
                //                      |--n_samples--|
                //         = |----------------------------|
                ///////////////////////////////////////////////////////////////
                else if(n_samples != 0
                            && a_length >= offset
                            && a_length > offset + n_samples
                            && n_samples < b_length)
                {
                    current_run[5] = true;

                    // Verify result length
                    if(result_length != a_length)
                    {
                        reportError(__FILE__,__LINE__,
                                        n_samples, offset, a_length, b_length,
                                        "result length error");
                    }

                    // Check prefix values.
                    for(uint32 i = 0; i < offset; ++i)
                    {
                        if(result[i] != 1.0)
                        {
                            reportError(__FILE__,__LINE__,
                                        n_samples, offset, a_length, b_length,
                                        "result prefix error");
                        }
                    }

                    // Check overlap values
                    for(uint32 i = offset + 1; i < offset + n_samples; ++i)
                    {
                        if(result[i] != 2.0)
                        {
                            reportError(__FILE__,__LINE__,
                                        n_samples, offset, a_length, b_length,
                                        "result overlap error");
                        }
                    }

                    // Check tail values
                    for(uint32 i = a_length + 1; i < result_length; ++i)
                    {
                        if(result[i] != 1.0)
                        {
                            reportError(__FILE__,__LINE__,
                                        n_samples, offset, a_length, b_length,
                                        "result tail error");
                        }
                    }
                }
                ///////////////////////////////////////////////////////////////
                //  Case 7: n_samples != 0
                //          && a_length >= offset
                //          && n_samples > b_length
                //          && a_length <= offset + b_length
                //
                //  result = [aaaaaaaaaaaaaaaaaaaa]
                //         + |--offset--|[bbbbbbbbbbbbbbbbbbbbbb]
                //                      |----n_samples---------------|
                //         = |---------------------------------------|
                ///////////////////////////////////////////////////////////////
                else if(n_samples != 0
                            && a_length >= offset
                            && a_length <= offset + b_length
                            && n_samples > b_length)
                {
                    current_run[6] = true;

                    // Verify result length
                    if(result_length != offset + n_samples)
                    {
                        reportError(__FILE__,__LINE__,
                                        n_samples, offset, a_length, b_length,
                                        "result length error");
                    }

                    // Check prefix values.
                    for(uint32 i = 0; i < offset; ++i)
                    {
                        if(result[i] != 1.0)
                        {
                            reportError(__FILE__,__LINE__,
                                        n_samples, offset, a_length, b_length,
                                        "result prefix error");
                        }
                    }

                    // Check overlap values
                    for(uint32 i = offset + 1; i < a_length; ++i)
                    {
                        if(result[i] != 2.0)
                        {
                            reportError(__FILE__,__LINE__,
                                        n_samples, offset, a_length, b_length,
                                        "result overlap error");
                        }
                    }

                    // Check b values
                    for(uint32 i = a_length + 1; i < offset + b_length; ++i)
                    {
                        if(result[i] != 1.0)
                        {
                            reportError(__FILE__,__LINE__,
                                        n_samples, offset, a_length, b_length,
                                        "result b value error");
                        }
                    }

                    // Check tail values
                    for(uint32 i = offset + b_length + 1; i < result_length; ++i)
                    {
                        if(result[i] != 0.0)
                        {
                            reportError(__FILE__,__LINE__,
                                        n_samples, offset, a_length, b_length,
                                        "result tail error");
                        }
                    }
                }
                ///////////////////////////////////////////////////////////////
                //  Case 8: n_samples == 0
                //          && a_length >= offset + b_length
                //
                //  result = [aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa]
                //         + |--offset--|[bbbbbbbbbbbbbbbbbbbbbb]
                //         = |-----------------------------------------|
                ///////////////////////////////////////////////////////////////
                else if(n_samples == 0
                            && a_length >= offset + b_length)
                {
                    current_run[7] = true;

                    // Verify result length
                    if(result_length != a_length)
                    {
                        reportError(__FILE__,__LINE__,
                                        n_samples, offset, a_length, b_length,
                                        "result length error");
                    }

                    // Check prefix values.
                    for(uint32 i = 0; i < offset; ++i)
                    {
                        if(result[i] != 1.0)
                        {
                            reportError(__FILE__,__LINE__,
                                        n_samples, offset, a_length, b_length,
                                        "result prefix error");
                        }
                    }

                    // Check overlap values
                    for(uint32 i = offset + 1; i < offset + b_length; ++i)
                    {
                        if(result[i] != 2.0)
                        {
                            reportError(__FILE__,__LINE__,
                                        n_samples, offset, a_length, b_length,
                                        "result overlap error");
                        }
                    }

                    // Check a values
                    for(uint32 i = offset + b_length + 1; i < result_length; ++i)
                    {
                        if(result[i] != 1.0)
                        {
                            reportError(__FILE__,__LINE__,
                                        n_samples, offset, a_length, b_length,
                                        "result a value error");
                        }
                    }
                }
                ///////////////////////////////////////////////////////////////
                //  Case 9:  n_samples != 0
                //           && a_length >= offset + n_samples
                //           && n_samples < b_length
                //
                //  result = [aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa]
                //         + |--offset--|[bbbbbbbbbbbbbbbbbbbb]
                //                      |----n_samples----|
                //         = |-----------------------------------------|
                ///////////////////////////////////////////////////////////////
                else if(n_samples != 0
                            && a_length >= offset + n_samples
                            && n_samples < b_length)
                {
                    current_run[8] = true;

                    // Verify result length
                    if(result_length != a_length)
                    {
                        reportError(__FILE__,__LINE__,
                                        n_samples, offset, a_length, b_length,
                                        "result length error");
                    }

                    // Check prefix values.
                    for(uint32 i = 0; i < offset; ++i)
                    {
                        if(result[i] != 1.0)
                        {
                            reportError(__FILE__,__LINE__,
                                        n_samples, offset, a_length, b_length,
                                        "result prefix error");
                        }
                    }

                    // Check overlap values
                    for(uint32 i = offset + 1; i < offset + n_samples; ++i)
                    {
                        if(result[i] != 2.0)
                        {
                            reportError(__FILE__,__LINE__,
                                        n_samples, offset, a_length, b_length,
                                        "result overlap error");
                        }
                    }

                    // Check a values
                    for(uint32 i = offset + n_samples + 1; i < result_length; ++i)
                    {
                        if(result[i] != 1.0)
                        {
                            reportError(__FILE__,__LINE__,
                                        n_samples, offset, a_length, b_length,
                                        "result a value error");
                        }
                    }
                }
                ///////////////////////////////////////////////////////////////
                //  Case 10: n_samples != 0
                //           && a_length >= offset + n_samples
                //           && n_samples >= b_length
                //
                //  result = [aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa]
                //         + |--offset--|[bbbbbbbbbbbbbbbbbbbbb]
                //                      |----n_samples------------|
                //         = |-----------------------------------------|
                ///////////////////////////////////////////////////////////////
                else if(n_samples != 0
                            && a_length >= offset + n_samples
                            && n_samples >= b_length)
                {
                    current_run[9] = true;

                    // Verify result length
                    if(result_length != a_length)
                    {
                        reportError(__FILE__,__LINE__,
                                        n_samples, offset, a_length, b_length,
                                        "result length error");
                    }

                    // Check prefix values.
                    for(uint32 i = 0; i < offset; ++i)
                    {
                        if(result[i] != 1.0)
                        {
                            reportError(__FILE__,__LINE__,
                                        n_samples, offset, a_length, b_length,
                                        "result prefix error");
                        }
                    }

                    // Check overlap values
                    for(uint32 i = offset + 1; i < offset + b_length; ++i)
                    {
                        if(result[i] != 2.0)
                        {
                            reportError(__FILE__,__LINE__,
                                        n_samples, offset, a_length, b_length,
                                        "result overlap error");
                        }
                    }

                    // Check a values
                    for(uint32 i = offset + b_length + 1; i < result_length; ++i)
                    {
                        if(result[i] != 1.0)
                        {
                            reportError(__FILE__,__LINE__,
                                        n_samples, offset, a_length, b_length,
                                        "result a value error");
                        }
                    }
                }
                ///////////////////////////////////////////////////////////////
                //  Case 11: n_samples != 0
                //           && a_length > offset + b_length
                //           && a_length < offset + n_samples
                //
                //  result = [aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa]
                //         + |--offset--|[bbbbbbbbbbbbbbbbbbbbb]
                //                      |----n_samples----------------------|
                //         = |----------------------------------------------|
                ///////////////////////////////////////////////////////////////
                else if(n_samples != 0
                            && a_length > offset + b_length
                            && a_length < offset + n_samples)
                {
                    current_run[10] = true;

                    // Verify result length
                    if(result_length != offset + n_samples)
                    {
                        reportError(__FILE__,__LINE__,
                                        n_samples, offset, a_length, b_length,
                                        "result length error");
                    }

                    // Check prefix values.
                    for(uint32 i = 0; i < offset; ++i)
                    {
                        if(result[i] != 1.0)
                        {
                            reportError(__FILE__,__LINE__,
                                        n_samples, offset, a_length, b_length,
                                        "result prefix error");
                        }
                    }

                    // Check overlap values
                    for(uint32 i = offset + 1; i < offset + b_length; ++i)
                    {
                        if(result[i] != 2.0)
                        {
                            reportError(__FILE__,__LINE__,
                                        n_samples, offset, a_length, b_length,
                                        "result overlap error");
                        }
                    }

                    // Check a values
                    for(uint32 i = offset + b_length + 1; i < a_length; ++i)
                    {
                        if(result[i] != 1.0)
                        {
                            reportError(__FILE__,__LINE__,
                                        n_samples, offset, a_length, b_length,
                                        "result a value error");
                        }
                    }

                    // Check tail values
                    for(uint32 i = a_length + 1; i < result_length; ++i)
                    {
                        if(result[i] != 0.0)
                        {
                            reportError(__FILE__,__LINE__,
                                        n_samples, offset, a_length, b_length,
                                        "result tail error");
                        }
                    }
                }
                else
                {
                    reportError(
                            __FILE__,
                            __LINE__,
                            n_samples,
                            offset,
                            a_length,
                            b_length,
                            "unhandled case");
                }

                for(uint32 i = 0; i < 11; ++i)
                {
                    if(current_run[i])
                        final_case[i] = true;
                }
            }
        }
    }

    // The old test printed this too: case 9 has never been reached.
    std::string not_hit;

    for(uint32 i = 0; i < 11; ++i)
    {
        if(!final_case[i])
        {
            not_hit += std::to_string(i + 1) + " ";
        }
    }

    if(!not_hit.empty())
    {
        WARN("Buffer::add() cases not hit: " << not_hit);
    }
}
