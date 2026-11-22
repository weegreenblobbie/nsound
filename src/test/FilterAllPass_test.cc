//-----------------------------------------------------------------------------
//
//  FilterAllPass tests, H(z) = (g + z^-M) / (1 + g z^-M).
//
//-----------------------------------------------------------------------------

#include <Nsound/AudioStream.h>
#include <Nsound/Buffer.h>
#include <Nsound/FilterAllPass.h>

#include "filter_signals.h"
#include "gold_check.h"

#include <catch2/catch_test_macros.hpp>

#include <cmath>

using namespace Nsound;

//! n samples from start towards finish, a step of (finish - start) / n.
static
Buffer
ramp(uint32 n, float64 start, float64 finish)
{
    Buffer b;

    for(uint32 i = 0; i < n; ++i)
    {
        b << start + (finish - start) * i / n;
    }

    return b;
}

TEST_CASE("FilterAllPass impulse response")
{
    // A 0.3 second delay at 10 Hz is 3 samples, the filter delays M = 4.
    FilterAllPass f(10.0, 0.3, 0.5);

    // g, then (1 - g^2) (-g)^(k - 1) every M samples.
    Buffer expected = Buffer::zeros(16);
    expected[0] = 0.5;
    expected[4] = 0.75;
    expected[8] = -0.375;
    expected[12] = 0.1875;

    check_close(f.getImpulseResponse(16), expected, 1e-15);
}

TEST_CASE("FilterAllPass passes every frequency with a gain of 1")
{
    for(float64 gain : {0.3, 0.5, 0.7})
    {
        INFO("gain " << gain);

        FilterAllPass f(1000.0, 0.0123, gain);

        Buffer h = f.getFrequencyResponse(1024);

        CHECK(std::fabs(h.getMin() - 1.0) < 1e-9);
        CHECK(std::fabs(h.getMax() - 1.0) < 1e-9);

        // Parseval, the impulse response's energy is 1.
        Buffer impulse = f.getImpulseResponse(8192);
        impulse *= impulse;

        CHECK(std::fabs(impulse.getSum() - 1.0) < 1e-9);
    }
}

TEST_CASE("FilterAllPass a shorter delay")
{
    FilterAllPass f(10.0, 0.3, 0.5);

    Buffer impulse = Buffer::zeros(8);
    impulse[0] = 1.0;

    // 0.1 seconds is 1 sample, M = 2.
    Buffer expected = Buffer::zeros(8);
    expected[0] = 0.5;
    expected[2] = 0.75;
    expected[4] = -0.375;
    expected[6] = 0.1875;

    check_close(f.filter(impulse, 0.1), expected, 1e-15);
}

TEST_CASE("FilterAllPass overloads agree")
{
    FilterSignals s = make_filter_signals();

    FilterAllPass f(s.sample_rate, 0.01, 0.5);

    check_one_cutoff_overloads(f, s.input.subbuffer(0, 1000), 0.01);

    // A delay that changes per sample.
    Buffer x = s.input.subbuffer(0, 1000);
    Buffer delays = ramp(1000, 0.001, 0.01);

    Buffer y = f.filter(x, delays);

    f.reset();

    Buffer per_sample;
    for(uint32 i = 0; i < 1000; ++i) per_sample << f.filter(x[i], delays[i]);

    check_close(per_sample, y, 0.0);
}

TEST_CASE("FilterAllPass copies")
{
    FilterAllPass f(1000.0, 0.01, 0.5);

    Buffer x = ramp(100, 1.0, -1.0);

    // Part way through, so the copies have history to carry over.
    f.filter(x);

    Buffer expected = FilterAllPass(f).filter(x);

    SECTION("the copy constructor carries the state")
    {
        FilterAllPass copy(f);

        check_close(copy.filter(x), expected, 0.0);
    }

    SECTION("operator=")
    {
        FilterAllPass other(1000.0, 0.01, 0.9);
        other = f;

        check_close(other.filter(x), expected, 0.0);

        FilterAllPass & same = other;
        other = same;

        check_close(other.filter(x), f.filter(x), 0.0);
    }
}

TEST_CASE("FilterAllPass Filter base")
{
    check_filter_base<FilterAllPass>(1000.0, 0.01, 0.5);

    FilterAllPass f(1000.0, 0.01, 0.5);

    CHECK_NOTHROW(f.plot(true, true));
}
