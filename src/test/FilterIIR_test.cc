//-----------------------------------------------------------------------------
//
//  FilterIIR tests, an IIR filter designed by a genetic algorithm.
//
//-----------------------------------------------------------------------------

#include <Nsound/AudioStream.h>
#include <Nsound/Buffer.h>
#include <Nsound/FilterIIR.h>
#include <Nsound/FilterLowPassIIR.h>

#include "filter_signals.h"
#include "gold_check.h"

#include <catch2/catch_test_macros.hpp>

#include <cmath>
#include <sstream>

using namespace Nsound;

//! The root mean square of a - b over a's length, how the design measures
//! its error.
static
float64
rms_error(const Buffer & a, const Buffer & b)
{
    Buffer delta = a - b.subbuffer(0, a.getLength());
    delta *= delta;
    return std::sqrt(delta.getMean());
}

//! Checks the error per generation never goes up and ends below the start.
static
void
check_evolution(const Buffer & evolution, int32 max_iterations)
{
    REQUIRE(evolution.getLength() > 1);
    CHECK(evolution.getLength() <= static_cast<uint32>(max_iterations) + 1);

    for(uint32 i = 1; i < evolution.getLength(); ++i)
    {
        INFO("generation " << i);
        CHECK(evolution[i] <= evolution[i - 1]);
    }

    CHECK(evolution[evolution.getLength() - 1] < evolution[0]);
}

TEST_CASE("FilterIIR designImpulseResponse()")
{
    // Match the impulse response of a 2 pole low pass IIR.
    FilterLowPassIIR reference(1000.0, 2, 100.0, 0.01);

    Buffer target = reference.getImpulseResponse(128);

    FilterIIR f(1000.0, 2);

    CHECK(f.getKernelSize() == 2);

    Buffer evolution = f.designImpulseResponse(target, 0.001, 200);

    check_evolution(evolution, 200);

    // The error it reports is the designed filter's.
    float64 error = evolution[evolution.getLength() - 1];

    CHECK(std::fabs(rms_error(target, f.getImpulseResponse(256)) - error) < 1e-12);
}

TEST_CASE("FilterIIR designFrequencyResponse()")
{
    FilterLowPassIIR reference(1000.0, 2, 100.0, 0.01);

    // The design compares the first 256 of the 257 bins of a 512 point FFT.
    Buffer target = reference.getFrequencyResponse(512).subbuffer(0, 256);

    FilterIIR f(1000.0, 2);

    Buffer evolution = f.designFrequencyResponse(target, 0.001, 200);

    check_evolution(evolution, 200);

    float64 error = evolution[evolution.getLength() - 1];

    CHECK(std::fabs(rms_error(target, f.getFrequencyResponse(512)) - error) < 1e-12);
}

TEST_CASE("FilterIIR needs a power of 2 long response")
{
    FilterIIR f(1000.0, 2);

    CHECK_THROWS_AS(f.designImpulseResponse(Buffer::ones(100)), Exception);
}

TEST_CASE("FilterIIR copies, overloads and output")
{
    FilterLowPassIIR reference(1000.0, 2, 100.0, 0.01);

    FilterIIR f(1000.0, 2);
    f.designImpulseResponse(reference.getImpulseResponse(128), 0.001, 50);

    Buffer h = f.getImpulseResponse(128);

    SECTION("the copy constructor")
    {
        FilterIIR copy(f);

        check_close(copy.getImpulseResponse(128), h, 0.0);
    }

    SECTION("operator= onto a filter with a different number of poles")
    {
        FilterIIR other(1000.0, 4);

        other = f;

        CHECK(other.getKernelSize() == 2);
        check_close(other.getImpulseResponse(128), h, 0.0);

        FilterIIR & same = other;
        other = same;

        check_close(other.getImpulseResponse(128), h, 0.0);
    }

    SECTION("the overloads agree, the frequency is ignored")
    {
        FilterSignals s = make_filter_signals();

        FilterIIR g(s.sample_rate, 2);
        g.designImpulseResponse(reference.getImpulseResponse(128), 0.001, 50);

        // FilterIIR has no overloads with a single frequency for a Buffer or
        // AudioStream, so not check_one_cutoff_overloads().
        Buffer x = s.input.subbuffer(0, 1000);
        Buffer y = g.filter(x);
        Buffer freqs = 440.0 * Buffer::ones(1000);

        Buffer per_sample;
        Buffer per_sample_f;

        g.reset();
        for(uint32 i = 0; i < 1000; ++i) per_sample << g.filter(x[i]);

        g.reset();
        for(uint32 i = 0; i < 1000; ++i) per_sample_f << g.filter(x[i], 440.0);

        check_close(per_sample, y, 0.0);
        check_close(per_sample_f, y, 0.0);
        check_close(g.filter(x, freqs), y, 0.0);

        AudioStream as(s.sample_rate, 2);
        as << x;

        check_every_channel(g.filter(as), y);
        check_every_channel(g.filter(as, freqs), y);
    }

    SECTION("operator<< shows the kernel")
    {
        std::ostringstream out;
        out << f;

        CHECK_FALSE(out.str().empty());
    }

    SECTION("plot() without matplotlib")
    {
        CHECK_NOTHROW(f.plot());
    }
}
