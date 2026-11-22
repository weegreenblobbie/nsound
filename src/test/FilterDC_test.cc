//-----------------------------------------------------------------------------
//
//  FilterDC tests, the DC blocker y[n] = x[n] - x[n-1] + g y[n-1].
//
//-----------------------------------------------------------------------------

#include <Nsound/AudioStream.h>
#include <Nsound/Buffer.h>
#include <Nsound/FilterDC.h>

#include "gold_check.h"

#include <catch2/catch_test_macros.hpp>

using namespace Nsound;

TEST_CASE("FilterDC removes a constant")
{
    FilterDC f(0.5);

    // 1, then each sample half the one before.
    check_close(f.filter(Buffer::ones(5)), Buffer({1.0, 0.5, 0.25, 0.125, 0.0625}), 0.0);
}

TEST_CASE("FilterDC keeps its state between calls")
{
    FilterDC f(0.5);

    Buffer y;
    y << f.filter(Buffer({1.0, 2.0}));
    y << f.filter(Buffer({2.0}));

    // 1, 2 - 1 + 0.5, 2 - 2 + 0.75
    check_close(y, Buffer({1.0, 1.5, 0.75}), 0.0);
}

TEST_CASE("FilterDC filters each channel from the same state")
{
    FilterDC f(0.5);

    AudioStream as(10.0, 2);
    as << Buffer::ones(5);

    AudioStream y = f.filter(as);

    check_close(y[0], Buffer({1.0, 0.5, 0.25, 0.125, 0.0625}), 0.0);
    check_close(y[1], y[0], 0.0);
}
