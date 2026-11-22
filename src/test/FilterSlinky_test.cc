//-----------------------------------------------------------------------------
//
//  FilterSlinky tests, band pass octaves each delayed by one more step
//  than the octave above.
//
//-----------------------------------------------------------------------------

#include <Nsound/AudioStream.h>
#include <Nsound/Buffer.h>
#include <Nsound/FilterSlinky.h>
#include <Nsound/Sine.h>

#include "filter_signals.h"
#include "gold_check.h"

#include <catch2/catch_test_macros.hpp>

#include <type_traits>

using namespace Nsound;

// It owns its filters and delay lines.
static_assert(!std::is_copy_constructible_v<FilterSlinky>);
static_assert(!std::is_copy_assignable_v<FilterSlinky>);

TEST_CASE("FilterSlinky delays the low octaves")
{
    // Octaves of 1000 Hz up to Nyquist, 4000 Hz: 3000 to 4000 Hz isn't
    // delayed, 2000 to 3000 Hz by 0.01 seconds, 1000 to 2000 Hz by 0.02 and
    // 0 to 1000 Hz by 0.03, 240 samples.
    FilterSlinky f(8000.0, 0.01, 1000.0);

    Sine sine(8000.0);

    Buffer x;
    x << sine.generate(0.05, 500.0) << sine.silence(0.1);

    Buffer y = f.filter(x);

    Buffer early = y.subbuffer(0, 200);
    early *= early;

    Buffer all = y;
    all *= all;

    // A 500 Hz tone comes out after the 240 sample delay.
    CHECK(early.getSum() < 0.01 * all.getSum());
    CHECK(all.getSum() > 0.1 * (x * x).getSum());
}

TEST_CASE("FilterSlinky checks its arguments")
{
    // A 0 Hz octave never reached Nyquist, so the constructor looped
    // forever.
    CHECK_THROWS_AS(FilterSlinky(8000.0, 0.01, 0.0), Exception);
    CHECK_THROWS_AS(FilterSlinky(8000.0, 0.01, -100.0), Exception);
    CHECK_THROWS_AS(FilterSlinky(8000.0, 0.0, 1000.0), Exception);
}

TEST_CASE("FilterSlinky overloads agree, the frequency is ignored")
{
    FilterSignals s = make_filter_signals();

    Buffer x = s.input.subbuffer(0, 2000);
    Buffer constant = 440.0 * Buffer::ones(2000);

    FilterSlinky f(s.sample_rate, 0.01, 1000.0);

    Buffer y = f.filter(x);

    // Each call starts from the beginning.
    check_close(f.filter(x), y, 0.0);

    Buffer per_sample;
    Buffer per_sample_f;

    f.reset();
    for(uint32 i = 0; i < 2000; ++i) per_sample << f.filter(x[i]);

    f.reset();
    for(uint32 i = 0; i < 2000; ++i) per_sample_f << f.filter(x[i], 440.0);

    check_close(per_sample, y, 0.0);
    check_close(per_sample_f, y, 0.0);
    check_close(f.filter(x, constant), y, 0.0);

    AudioStream as(s.sample_rate, 2);
    as << x;

    check_every_channel(f.filter(as), y);
    check_every_channel(f.filter(as, constant), y);
}

TEST_CASE("FilterSlinky in real time keeps its state between calls")
{
    FilterSignals s = make_filter_signals();

    Buffer x = s.input.subbuffer(0, 2000);

    FilterSlinky whole(s.sample_rate, 0.01, 1000.0);

    FilterSlinky f(s.sample_rate, 0.01, 1000.0);
    f.setRealtime(true);

    Buffer y;
    y << f.filter(x.subbuffer(0, 1000)) << f.filter(x.subbuffer(1000));

    check_close(y, whole.filter(x), 0.0);
}

TEST_CASE("FilterSlinky Filter base")
{
    check_filter_base<FilterSlinky>(8000.0, 0.01, 1000.0);
}
