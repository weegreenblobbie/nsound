//-----------------------------------------------------------------------------
//
//  Stretcher tests, time stretching and pitch shifting by overlapping
//  windows of the input.
//
//-----------------------------------------------------------------------------

#include <Nsound/AudioStream.h>
#include <Nsound/Buffer.h>
#include <Nsound/Sine.h>
#include <Nsound/Stretcher.h>
#include <Nsound/Wavefile.h>

#include "gold_check.h"

#include <catch2/catch_test_macros.hpp>

#include <cmath>

using namespace Nsound;

//! The frequency of a tone from its zero crossings in the middle half.
static
float64
frequency(const Buffer & b, float64 sample_rate)
{
    uint32 start = b.getLength() / 4;
    uint32 stop = 3 * b.getLength() / 4;

    uint32 n = 0;
    for(uint32 i = start + 1; i < stop; ++i)
    {
        if((b[i - 1] < 0.0) != (b[i] < 0.0)) ++n;
    }

    return 0.5 * n * sample_rate / (stop - start);
}

// A 640 sample Hanning window, searching up to 160 samples for the best
// match.
static Stretcher make_stretcher() { return Stretcher(8000.0, 0.08, 0.25); }

TEST_CASE("Stretcher timeShift() changes the length, not the pitch")
{
    Sine sine(8000.0);
    Buffer x = sine.generate(1.0, 440.0);

    Stretcher st = make_stretcher();

    for(float64 factor : {0.5, 1.5, 2.0})
    {
        INFO("factor " << factor);

        Buffer y = st.timeShift(x, factor);

        // The windows step factor times slower than the input, so it is short
        // by up to a stretched window.
        CHECK(std::fabs(y.getLength() - factor * 8000.0) <= factor * 640.0);
        CHECK(std::fabs(frequency(y, 8000.0) - 440.0) < 5.0);
    }
}

TEST_CASE("Stretcher pitchShift() changes the pitch, not the length")
{
    Sine sine(8000.0);
    Buffer x = sine.generate(1.0, 440.0);

    Stretcher st = make_stretcher();

    Buffer y = st.pitchShift(x, 2.0);

    CHECK(std::fabs(y.getLength() - 8000.0) <= 640.0);
    CHECK(std::fabs(frequency(y, 8000.0) - 880.0) < 10.0);
}

TEST_CASE("Stretcher gold, the same with and without OpenMP")
{
    Wavefile::setDefaultSampleSize(64);
    Wavefile::setIEEEFloat(true);

    Sine sine(8000.0);

    Buffer x = sine.generate(0.25, 440.0) + 0.5 * sine.generate(0.25, 1250.0);

    Stretcher st = make_stretcher();

    check_gold(st.timeShift(x, 1.5), "gold/Stretcher_timeShift_1.5.wav", 1e-14);
}

TEST_CASE("Stretcher overloads agree")
{
    Sine sine(8000.0);
    Buffer x = sine.generate(0.5, 440.0);

    Stretcher st = make_stretcher();

    Buffer time = st.timeShift(x, 1.5);
    Buffer pitch = st.pitchShift(x, 1.5);

    Buffer constant = 1.5 * Buffer::ones(x.getLength());

    check_close(st.timeShift(x, constant), time, 0.0);

    AudioStream as(8000.0, 2);
    as << x;

    AudioStream as_time = st.timeShift(as, 1.5);
    AudioStream as_time_b = st.timeShift(as, constant);
    AudioStream as_pitch = st.pitchShift(as, 1.5);

    for(uint32 channel = 0; channel < 2; ++channel)
    {
        INFO("channel " << channel);
        check_close(as_time[channel], time, 0.0);
        check_close(as_time_b[channel], time, 0.0);
        check_close(as_pitch[channel], pitch, 0.0);
    }

    // A changing factor, and pitch shifting by one: getSpeedUp() rather than
    // getResample().
    CHECK(st.timeShift(x, Buffer({1.0, 2.0})).getLength() > 0);
    CHECK(st.pitchShift(x, Buffer({1.0, 1.0})).getLength() > 0);
    CHECK(st.pitchShift(as, Buffer({1.0, 1.0})).getLength() > 0);
}

TEST_CASE("Stretcher copies")
{
    Sine sine(8000.0);
    Buffer x = sine.generate(0.5, 440.0);

    Stretcher st = make_stretcher();

    Buffer expected = st.timeShift(x, 1.5);

    Stretcher copy(st);
    check_close(copy.timeShift(x, 1.5), expected, 0.0);

    Stretcher other(8000.0, 0.02, 0.5);
    other = st;
    check_close(other.timeShift(x, 1.5), expected, 0.0);

    Stretcher & same = other;
    other = same;
    check_close(other.timeShift(x, 1.5), expected, 0.0);
}

TEST_CASE("Stretcher showProgress()")
{
    Sine sine(8000.0);
    Buffer x = sine.generate(0.5, 440.0);

    Stretcher st = make_stretcher();

    Buffer expected = st.timeShift(x, 1.5);

    st.showProgress(true);

    check_close(st.timeShift(x, 1.5), expected, 0.0);
}

TEST_CASE("Stretcher checks its input")
{
    Sine sine(8000.0);
    Buffer x = sine.generate(0.5, 440.0);

    Stretcher st = make_stretcher();

    // Both used to loop forever.
    SECTION("an input no longer than the window")
    {
        CHECK_THROWS_AS(st.timeShift(x.subbuffer(0, 640), 2.0), Exception);
    }

    SECTION("a factor of 0")
    {
        CHECK_THROWS_AS(st.timeShift(x, 0.0), Exception);
        CHECK_THROWS_AS(st.timeShift(x, Buffer({1.0, 0.0})), Exception);
    }
}
