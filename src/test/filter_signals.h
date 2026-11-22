//-----------------------------------------------------------------------------
//
//  The input signals the low, high, band pass and band reject filter tests
//  share, the same as src/test/test_filters.py.
//
//-----------------------------------------------------------------------------
#pragma once

#include <Nsound/AudioStream.h>
#include <Nsound/Buffer.h>
#include <Nsound/Filter.h>
#include <Nsound/Sine.h>
#include <Nsound/Wavefile.h>

#include "gold_check.h"

#include <catch2/catch_test_macros.hpp>

#include <cmath>
#include <memory>

//! The gold files are 32 bit floats, test_filters.py compares to 5 places.
static const Nsound::float64 FILTER_GAMMA = 1e-5;

//! A 5 second sweep from 0 to 1720 Hz at 8000 Hz, and cutoffs that follow
//! it: freqs_lo wobbles 200 Hz around the sweep, freqs_hi is 200 Hz above.
struct FilterSignals
{
    Nsound::float64 sample_rate = 8000.0;
    Nsound::Buffer input;
    Nsound::Buffer freqs_lo;
    Nsound::Buffer freqs_hi;
};

inline
FilterSignals
make_filter_signals()
{
    Nsound::Wavefile::setDefaultSampleSize(32);
    Nsound::Wavefile::setIEEEFloat(true);

    FilterSignals s;

    Nsound::Sine sine(s.sample_rate);

    Nsound::Buffer sweep = sine.drawLine(5.0, 0.0, 1720.0);

    s.input = sine.generate(5.0, sweep);
    s.freqs_lo = 200.0 * sine.generate(5.0, 1.0) + sweep;
    s.freqs_hi = 200.0 + s.freqs_lo;

    return s;
}

//! Checks that every channel of as matches y exactly.
inline
void
check_every_channel(const Nsound::AudioStream & as, const Nsound::Buffer & y)
{
    for(Nsound::uint32 i = 0; i < as.getNChannels(); ++i)
    {
        INFO("channel " << i);
        check_close(as[i], y, 0.0);
    }
}

//! Checks that filtering x through each overload of a filter with one cutoff
//! gives filter(x) when the cutoff is the filter's own, fc.
template <class F>
void
check_one_cutoff_overloads(F & f, const Nsound::Buffer & x, Nsound::float64 fc)
{
    using namespace Nsound;

    Buffer y = f.filter(x);
    Buffer constant = fc * Buffer::ones(x.getLength());

    Buffer per_sample;
    Buffer per_sample_fc;

    f.reset();
    for(uint32 i = 0; i < x.getLength(); ++i) per_sample << f.filter(x[i]);

    f.reset();
    for(uint32 i = 0; i < x.getLength(); ++i) per_sample_fc << f.filter(x[i], fc);

    check_close(per_sample, y, 0.0);
    check_close(per_sample_fc, y, 0.0);
    check_close(f.filter(x, fc), y, 0.0);
    check_close(f.filter(x, constant), y, 0.0);

    AudioStream as(f.getSampleRate(), 2);
    as << x;

    check_every_channel(f.filter(as), y);
    check_every_channel(f.filter(as, fc), y);
    check_every_channel(f.filter(as, constant), y);
}

//! Checks that filtering x through each overload of a band filter gives
//! filter(x) when the cutoffs are the filter's own, f_low and f_high. The
//! overloads with one frequency ignore it.
template <class F>
void
check_two_cutoff_overloads(
    F & f,
    const Nsound::Buffer & x,
    Nsound::float64 f_low,
    Nsound::float64 f_high)
{
    using namespace Nsound;

    Buffer y = f.filter(x);
    Buffer low = f_low * Buffer::ones(x.getLength());
    Buffer high = f_high * Buffer::ones(x.getLength());

    Buffer per_sample;
    Buffer per_sample_f;
    Buffer per_sample_band;

    f.reset();
    for(uint32 i = 0; i < x.getLength(); ++i) per_sample << f.filter(x[i]);

    f.reset();
    for(uint32 i = 0; i < x.getLength(); ++i) per_sample_f << f.filter(x[i], f_low);

    f.reset();
    for(uint32 i = 0; i < x.getLength(); ++i) per_sample_band << f.filter(x[i], f_low, f_high);

    check_close(per_sample, y, 0.0);
    check_close(per_sample_f, y, 0.0);
    check_close(per_sample_band, y, 0.0);
    check_close(f.filter(x, f_low, f_high), y, 0.0);
    check_close(f.filter(x, low, high), y, 0.0);

    AudioStream as(f.getSampleRate(), 2);
    as << x;

    check_every_channel(f.filter(as), y);
    check_every_channel(f.filter(as, f_low, f_high), y);
    check_every_channel(f.filter(as, low, high), y);

    // Only some band filters have these.
    if constexpr (requires { f.filter(x, f_low); })
    {
        check_close(f.filter(x, f_low), y, 0.0);
        check_close(f.filter(x, low), y, 0.0);
        check_every_channel(f.filter(as, f_low), y);
        check_every_channel(f.filter(as, low), y);
    }
}

//! Checks the Filter base methods: the frequency axis, phase response,
//! plot() without matplotlib and deleting through a Filter pointer.
template <class F, class... Args>
void
check_filter_base(Nsound::float64 sample_rate, Args... args)
{
    using namespace Nsound;

    F f(sample_rate, args...);

    // 1000 rounds up to 1024 bins of sample_rate / 1024 Hz, up to Nyquist.
    Buffer axis = f.getFrequencyAxis(1000);

    REQUIRE(axis.getLength() == 513);
    CHECK(axis[1] == sample_rate / 1024.0);
    CHECK(axis[512] == sample_rate / 2.0);

    Buffer phase = f.getPhaseResponse();

    REQUIRE(phase.getLength() > 0);

    for(uint32 i = 0; i < phase.getLength(); ++i)
    {
        if(std::isnan(phase[i])) FAIL("phase response is NaN at " << i);
    }

    CHECK_NOTHROW(f.plot());
    // Filters with a cutoff also have plot(show_fc, show_phase).
    if constexpr(requires { f.plot(true, true); })
    {
        CHECK_NOTHROW(f.plot(true, true));
    }

    // In real time mode a filter keeps its state between calls, so it can't
    // filter several channels.
    AudioStream stereo(sample_rate, 2);
    stereo << Buffer::ones(10);

    f.setRealtime(true);

    CHECK_THROWS_AS(f.filter(stereo), Exception);

    std::unique_ptr<Filter> base = std::make_unique<F>(sample_rate, args...);

    CHECK_NOTHROW(base.reset());
}

