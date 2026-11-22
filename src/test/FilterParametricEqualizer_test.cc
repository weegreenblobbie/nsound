//-----------------------------------------------------------------------------
//
//  FilterParametricEqualizer tests, compared with the gold files.
//
//-----------------------------------------------------------------------------

#include <Nsound/AudioStream.h>
#include <Nsound/Buffer.h>
#include <Nsound/FilterParametricEqualizer.h>
#include <Nsound/Sine.h>
#include <Nsound/Wavefile.h>

#include "filter_signals.h"
#include "gold_check.h"

#include <catch2/catch_test_macros.hpp>

#include <cmath>
#include <string>
#include <type_traits>

using namespace Nsound;

static const float64 GAMMA = 1.5e-14;

//! Filters two copies of the noise file with a sweep from 24 to 6 Hz.
static
void
check_equalizer(FilterParametricEqualizer::Type type, const std::string & gold_file)
{
    Wavefile::setDefaultSampleSize(64);
    Wavefile::setIEEEFloat(true);

    Sine sine(100);

    FilterParametricEqualizer f(type, 100, 12, 0.2, 4.0);

    Buffer freqs = sine.drawLine(5.0, 24, 6);

    Buffer noise;
    noise << "gold/Filter_noise.wav";
    noise << "gold/Filter_noise.wav";

    check_gold(f.filter(noise, freqs), gold_file, GAMMA);
}

TEST_CASE("FilterParametricEqualizer")
{
    SECTION("peaking")
    {
        check_equalizer(
            FilterParametricEqualizer::PEAKING,
            "gold/FilterParametricEqualizer_out_peaking.wav");
    }

    SECTION("low shelf")
    {
        check_equalizer(
            FilterParametricEqualizer::LOW_SHELF,
            "gold/FilterParametricEqualizer_out_low.wav");
    }

    SECTION("high shelf")
    {
        check_equalizer(
            FilterParametricEqualizer::HIGH_SHELF,
            "gold/FilterParametricEqualizer_out_high.wav");
    }
}

TEST_CASE("FilterParametricEqualizer boosts in dB")
{
    // 8000 / 8192 Hz bins, so 1000 Hz is bin 1024.
    auto response = [](FilterParametricEqualizer::Type type, float64 boost_dB)
    {
        FilterParametricEqualizer f(type, 8000.0, 1000.0, 0.707106781187, boost_dB);
        return f.getFrequencyResponse(8192);
    };

    float64 gain_6dB = std::pow(10.0, 6.0 / 20.0);

    SECTION("0 dB, the default, passes everything")
    {
        for(auto type : {
            FilterParametricEqualizer::PEAKING,
            FilterParametricEqualizer::LOW_SHELF,
            FilterParametricEqualizer::HIGH_SHELF})
        {
            FilterParametricEqualizer f(type, 8000.0, 1000.0);

            Buffer h = f.getFrequencyResponse(8000);

            CHECK(std::fabs(h.getMin() - 1.0) < 1e-9);
            CHECK(std::fabs(h.getMax() - 1.0) < 1e-9);
        }
    }

    SECTION("peaking boosts its center")
    {
        Buffer h = response(FilterParametricEqualizer::PEAKING, 6.0);

        CHECK(std::fabs(h[1024] - gain_6dB) < 1e-3);
        CHECK(std::fabs(h[1] - 1.0) < 1e-3);
        CHECK(std::fabs(h[h.getLength() - 2] - 1.0) < 1e-3);
    }

    SECTION("low shelf boosts DC, high shelf Nyquist")
    {
        Buffer low = response(FilterParametricEqualizer::LOW_SHELF, 6.0);
        Buffer high = response(FilterParametricEqualizer::HIGH_SHELF, 6.0);

        CHECK(std::fabs(low[1] - gain_6dB) < 1e-3);
        CHECK(std::fabs(low[low.getLength() - 2] - 1.0) < 1e-3);

        CHECK(std::fabs(high[1] - 1.0) < 1e-3);
        CHECK(std::fabs(high[high.getLength() - 2] - gain_6dB) < 1e-3);
    }
}

static_assert(!std::is_copy_constructible_v<FilterParametricEqualizer>);
static_assert(!std::is_copy_assignable_v<FilterParametricEqualizer>);

TEST_CASE("FilterParametricEqualizer overloads agree")
{
    FilterSignals s = make_filter_signals();

    uint32 n = 2000;
    Buffer x = s.input.subbuffer(0, n);
    Buffer freqs = 50.0 * Buffer::ones(n);
    Buffer resonances = 0.5 * Buffer::ones(n);
    Buffer boosts = 6.0 * Buffer::ones(n);

    FilterParametricEqualizer f(
        FilterParametricEqualizer::PEAKING, s.sample_rate, 50.0, 0.5, 6.0);

    Buffer y = f.filter(x);

    // It changes the start of the sweep, which is near 50 Hz.
    CHECK((y - x).getAbs().getMax() > 0.1);

    Buffer per_sample;
    Buffer per_sample_f;
    Buffer per_sample_fr;
    Buffer per_sample_frb;

    f.reset();
    for(uint32 i = 0; i < n; ++i) per_sample << f.filter(x[i]);

    f.reset();
    for(uint32 i = 0; i < n; ++i) per_sample_f << f.filter(x[i], 50.0);

    f.reset();
    for(uint32 i = 0; i < n; ++i) per_sample_fr << f.filter(x[i], 50.0, 0.5);

    f.reset();
    for(uint32 i = 0; i < n; ++i) per_sample_frb << f.filter(x[i], 50.0, 0.5, 6.0);

    check_close(per_sample, y, 0.0);
    check_close(per_sample_f, y, 0.0);
    check_close(per_sample_fr, y, 0.0);
    check_close(per_sample_frb, y, 0.0);

    check_close(f.filter(x, freqs), y, 0.0);
    check_close(f.filter(x, freqs, resonances), y, 0.0);
    check_close(f.filter(x, freqs, resonances, boosts), y, 0.0);

    AudioStream as(s.sample_rate, 2);
    as << x;

    check_every_channel(f.filter(as), y);
    check_every_channel(f.filter(as, freqs), y);
    check_every_channel(f.filter(as, freqs, resonances), y);
    check_every_channel(f.filter(as, freqs, resonances, boosts), y);

    // Another resonance and boost filter differently.
    CHECK((f.filter(x, freqs, 2.0 * resonances) - y).getAbs().getMax() > 0.01);
    CHECK((f.filter(x, freqs, resonances, 2.0 * boosts) - y).getAbs().getMax() > 0.01);
}

TEST_CASE("FilterParametricEqualizer in real time keeps its state between calls")
{
    FilterSignals s = make_filter_signals();

    Buffer x = s.input.subbuffer(0, 2000);
    Buffer first = x.subbuffer(0, 1000);
    Buffer second = x.subbuffer(1000);

    Buffer freqs = 50.0 * Buffer::ones(1000);
    Buffer resonances = 0.5 * Buffer::ones(1000);
    Buffer boosts = 6.0 * Buffer::ones(1000);

    FilterParametricEqualizer whole(
        FilterParametricEqualizer::LOW_SHELF, s.sample_rate, 50.0, 0.5, 6.0);

    Buffer expected = whole.filter(x);

    FilterParametricEqualizer f(
        FilterParametricEqualizer::LOW_SHELF, s.sample_rate, 50.0, 0.5, 6.0);

    f.setRealtime(true);

    SECTION("filter(x)")
    {
        Buffer y;
        y << f.filter(first) << f.filter(second);
        check_close(y, expected, 0.0);
    }

    SECTION("filter(x, frequencies)")
    {
        Buffer y;
        y << f.filter(first, freqs) << f.filter(second, freqs);
        check_close(y, expected, 0.0);
    }

    SECTION("filter(x, frequencies, resonances)")
    {
        Buffer y;
        y << f.filter(first, freqs, resonances) << f.filter(second, freqs, resonances);
        check_close(y, expected, 0.0);
    }

    SECTION("filter(x, frequencies, resonances, boosts)")
    {
        Buffer y;
        y << f.filter(first, freqs, resonances, boosts)
          << f.filter(second, freqs, resonances, boosts);
        check_close(y, expected, 0.0);
    }

    SECTION("a filter per audio channel")
    {
        AudioStream stereo(s.sample_rate, 2);
        stereo << x;

        CHECK_THROWS_AS(f.filter(stereo), Exception);
        CHECK_THROWS_AS(f.filter(stereo, freqs), Exception);
        CHECK_THROWS_AS(f.filter(stereo, freqs, resonances), Exception);
        CHECK_THROWS_AS(f.filter(stereo, freqs, resonances, boosts), Exception);
    }
}

//! A peaking equalizer constructed with the sample rate first, the way
//! check_filter_base() builds filters.
struct PeakingEqualizer : public FilterParametricEqualizer
{
    PeakingEqualizer(
        const float64 & sample_rate,
        const float64 & frequency,
        const float64 & resonance,
        const float64 & boost_dB)
        :
        FilterParametricEqualizer(PEAKING, sample_rate, frequency, resonance, boost_dB)
    {
    }
};

TEST_CASE("FilterParametricEqualizer Filter base")
{
    check_filter_base<PeakingEqualizer>(8000.0, 1000.0, 0.707106781187, 6.0);
}
