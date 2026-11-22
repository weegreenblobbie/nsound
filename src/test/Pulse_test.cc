//-----------------------------------------------------------------------------
//
//  Pulse tests.
//
//-----------------------------------------------------------------------------

#include <Nsound/Buffer.h>
#include <Nsound/Pulse.h>

#include "gold_check.h"

#include <catch2/catch_test_macros.hpp>

using namespace Nsound;

//! A Buffer of 1s from start for width samples, every period samples.
static
Buffer
pulses(uint32 n, uint32 period, uint32 width)
{
    Buffer b;
    for(uint32 i = 0; i < n; ++i) b << ((i % period) < width ? 1.0 : 0.0);
    return b;
}

// Pulse adds the sample time to its time each sample, so an edge exactly on
// a sample can land a sample either way. These widths end half way between
// samples.
TEST_CASE("Pulse")
{
    SECTION("a width in percent of the wavelength")
    {
        Pulse p(20.0, 0.275);

        check_close(p.generate(2.0, 1.0), pulses(40, 20, 6), 0.0);

        // generate() starts again.
        check_close(p.generate(2.0, 1.0), pulses(40, 20, 6), 0.0);
    }

    SECTION("a width in seconds")
    {
        Pulse p(20.0, 0.125, Pulse::PULSE_SECONDS);

        check_close(p.generate(2.0, 1.0), pulses(40, 20, 3), 0.0);

        p.setUnits(Pulse::PULSE_PERCENT);

        check_close(p.generate(2.0, 1.0), pulses(40, 20, 3), 0.0);
    }

    SECTION("generate2() with another width")
    {
        Pulse p(20.0, 0.275);

        check_close(p.generate2(2.0, 1.0, 0.525), pulses(40, 20, 11), 0.0);
    }

    SECTION("one sample at a time, and the edges")
    {
        Pulse p(20.0, 0.275);

        Buffer y;
        for(uint32 i = 0; i < 40; ++i) y << p.generate(1.0);

        check_close(y, pulses(40, 20, 6), 0.0);

        // The next pulse rises at 2 seconds.
        CHECK(p.riseTime() == 2.0);

        p.reset();

        CHECK(p.riseTime() == 0.0);
        CHECK(p.generate(1.0) == 1.0);
        CHECK(p.fallTime() == 0.275);
    }
}
