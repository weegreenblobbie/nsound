//-----------------------------------------------------------------------------
//
//  FilterStageIIR tests, the stage the low and high pass IIR filters use.
//
//-----------------------------------------------------------------------------

#include <Nsound/AudioStream.h>
#include <Nsound/Buffer.h>
#include <Nsound/FilterHighPassIIR.h>
#include <Nsound/FilterLowPassIIR.h>
#include <Nsound/FilterStageIIR.h>

#include "filter_signals.h"
#include "gold_check.h"

#include <catch2/catch_test_macros.hpp>

using namespace Nsound;

TEST_CASE("FilterStageIIR is the low and high pass IIR filter")
{
    FilterSignals s = make_filter_signals();

    Buffer x = s.input.subbuffer(0, 4000);

    SECTION("low pass")
    {
        FilterStageIIR stage(FilterStageIIR::LOW_PASS, s.sample_rate, 4, 880.0, 0.01);
        FilterLowPassIIR low_pass(s.sample_rate, 4, 880.0, 0.01);

        check_close(stage.filter(x), low_pass.filter(x), 0.0);
        check_close(stage.filter(x, s.freqs_lo.subbuffer(0, 4000)),
                    low_pass.filter(x, s.freqs_lo.subbuffer(0, 4000)), 0.0);
    }

    SECTION("high pass")
    {
        FilterStageIIR stage(FilterStageIIR::HIGH_PASS, s.sample_rate, 4, 880.0, 0.01);
        FilterHighPassIIR high_pass(s.sample_rate, 4, 880.0, 0.01);

        check_close(stage.filter(x), high_pass.filter(x), 0.0);
    }
}

TEST_CASE("FilterStageIIR overloads agree")
{
    FilterSignals s = make_filter_signals();

    FilterStageIIR f(FilterStageIIR::LOW_PASS, s.sample_rate, 4, 880.0, 0.01);

    check_one_cutoff_overloads(f, s.input.subbuffer(0, 4000), 880.0);
}

TEST_CASE("FilterStageIIR copies")
{
    FilterSignals s = make_filter_signals();

    Buffer x = s.input.subbuffer(0, 4000);

    FilterStageIIR f(FilterStageIIR::LOW_PASS, s.sample_rate, 4, 880.0, 0.01);

    // Cache a second kernel, so the copies copy more than one.
    f.filter(x, 1200.0);

    Buffer expected = f.filter(x);

    SECTION("the copy constructor")
    {
        FilterStageIIR copy(f);

        check_close(copy.filter(x), expected, 0.0);
    }

    SECTION("operator= onto a filter with a different number of poles")
    {
        FilterStageIIR other(FilterStageIIR::HIGH_PASS, s.sample_rate, 2, 440.0, 0.01);

        other = f;

        check_close(other.filter(x), expected, 0.0);

        FilterStageIIR & same = other;
        other = same;

        check_close(other.filter(x), expected, 0.0);
    }

    SECTION("deleting through a Filter pointer")
    {
        std::unique_ptr<Filter> base = std::make_unique<FilterStageIIR>(f);

        CHECK_NOTHROW(base.reset());
    }
}
