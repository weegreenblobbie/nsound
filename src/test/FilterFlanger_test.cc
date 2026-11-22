//-----------------------------------------------------------------------------
//
//  FilterFlanger tests, y[n] = (x[n] + x[n - d[n]]) / 2 where the delay d
//  sweeps between 0 and the delay at the flanger's frequency.
//
//-----------------------------------------------------------------------------

#include <Nsound/AudioStream.h>
#include <Nsound/Buffer.h>
#include <Nsound/FilterFlanger.h>

#include "filter_signals.h"
#include "gold_check.h"

#include <catch2/catch_test_macros.hpp>

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

TEST_CASE("FilterFlanger averages the input with its delayed self")
{
    FilterFlanger f(1000.0, 2.0, 0.01);

    // The delay line starts empty, so a constant starts at half.
    Buffer y = f.filter(Buffer::ones(100));

    CHECK(y[0] == 0.5);
    check_close(y.subbuffer(11), Buffer::ones(89), 0.0);

    SECTION("no delay passes the input")
    {
        Buffer x = ramp(100, -1.0, 1.0);

        check_close(f.filter(x, 2.0, 0.0), x, 1e-15);
    }
}

TEST_CASE("FilterFlanger needs a positive frequency and delay")
{
    CHECK_THROWS_AS(FilterFlanger(1000.0, 0.0, 0.01), Exception);
    CHECK_THROWS_AS(FilterFlanger(1000.0, 2.0, 0.0), Exception);
}

TEST_CASE("FilterFlanger overloads agree")
{
    FilterSignals s = make_filter_signals();

    Buffer x = s.input.subbuffer(0, 2000);
    Buffer freqs = ramp(2000, 1.0, 5.0);
    Buffer delays = ramp(2000, 0.002, 0.01);

    FilterFlanger f(s.sample_rate, 2.0, 0.01);

    Buffer y = f.filter(x);

    Buffer per_sample;
    Buffer per_sample_f;
    Buffer per_sample_fd;
    Buffer per_sample_dynamic;

    f.reset();
    for(uint32 i = 0; i < 2000; ++i) per_sample << f.filter(x[i]);

    f.reset();
    for(uint32 i = 0; i < 2000; ++i) per_sample_f << f.filter(x[i], 2.0);

    f.reset();
    for(uint32 i = 0; i < 2000; ++i) per_sample_fd << f.filter(x[i], 2.0, 0.01);

    f.reset();
    for(uint32 i = 0; i < 2000; ++i)
    {
        per_sample_dynamic << f.filter(x[i], freqs[i], delays[i]);
    }

    check_close(per_sample, y, 0.0);
    check_close(per_sample_f, y, 0.0);
    check_close(per_sample_fd, y, 0.0);
    check_close(f.filter(x, 2.0), y, 0.0);
    check_close(f.filter(x, 2.0, 0.01), y, 0.0);

    // A frequency and delay that change per sample.
    Buffer dynamic = f.filter(x, freqs, delays);

    check_close(dynamic, per_sample_dynamic, 0.0);
    CHECK((dynamic - y).getAbs().getMax() > 0.01);

    AudioStream as(s.sample_rate, 2);
    as << x;

    check_every_channel(f.filter(as), y);
    check_every_channel(f.filter(as, 2.0), y);
    check_every_channel(f.filter(as, 2.0, 0.01), y);
    check_every_channel(f.filter(as, freqs, delays), dynamic);
}

TEST_CASE("FilterFlanger copies carry the state")
{
    FilterFlanger f(1000.0, 2.0, 0.01);

    Buffer x = ramp(200, -1.0, 1.0);

    // Part way through, so the copies have a delay line and sweep position
    // to carry over.
    for(uint32 i = 0; i < 150; ++i) f.filter(x[i]);

    Buffer expected;
    {
        FilterFlanger reference(1000.0, 2.0, 0.01);
        for(uint32 i = 0; i < 150; ++i) reference.filter(x[i]);
        for(uint32 i = 0; i < 200; ++i) expected << reference.filter(x[i]);
    }

    SECTION("the copy constructor")
    {
        FilterFlanger copy(f);

        Buffer y;
        for(uint32 i = 0; i < 200; ++i) y << copy.filter(x[i]);

        check_close(y, expected, 0.0);
    }

    SECTION("operator=")
    {
        FilterFlanger other(1000.0, 5.0, 0.02);
        other = f;

        FilterFlanger & same = other;
        other = same;

        Buffer y;
        for(uint32 i = 0; i < 200; ++i) y << other.filter(x[i]);

        check_close(y, expected, 0.0);
    }
}

TEST_CASE("FilterFlanger in real time keeps its state between calls")
{
    Buffer x = ramp(200, -1.0, 1.0);
    Buffer freqs = ramp(200, 1.0, 5.0);
    Buffer delays = ramp(200, 0.002, 0.01);

    FilterFlanger whole(1000.0, 2.0, 0.01);

    FilterFlanger f(1000.0, 2.0, 0.01);
    f.setRealtime(true);

    Buffer first = x.subbuffer(0, 100);
    Buffer second = x.subbuffer(100);

    SECTION("filter(x)")
    {
        Buffer y;
        y << f.filter(first) << f.filter(second);

        check_close(y, whole.filter(x), 0.0);
    }

    SECTION("filter(x, frequency)")
    {
        Buffer y;
        y << f.filter(first, 3.0) << f.filter(second, 3.0);

        check_close(y, whole.filter(x, 3.0), 0.0);
    }

    SECTION("filter(x, frequency, delay)")
    {
        Buffer y;
        y << f.filter(first, 3.0, 0.005) << f.filter(second, 3.0, 0.005);

        check_close(y, whole.filter(x, 3.0, 0.005), 0.0);
    }

    SECTION("filter(x, frequencies, delays)")
    {
        Buffer y;
        y << f.filter(first, freqs.subbuffer(0, 100), delays.subbuffer(0, 100))
          << f.filter(second, freqs.subbuffer(100), delays.subbuffer(100));

        check_close(y, whole.filter(x, freqs, delays), 0.0);
    }

    SECTION("a filter per audio channel")
    {
        AudioStream stereo(1000.0, 2);
        stereo << x;

        CHECK_THROWS_AS(f.filter(stereo, 3.0, 0.005), Exception);
        CHECK_THROWS_AS(f.filter(stereo, freqs, delays), Exception);
    }
}

TEST_CASE("FilterFlanger Filter base")
{
    check_filter_base<FilterFlanger>(1000.0, 2.0, 0.01);
}
