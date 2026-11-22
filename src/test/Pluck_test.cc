//-----------------------------------------------------------------------------
//
//  Pluck tests, a Karplus-Strong plucked string from white noise.
//
//-----------------------------------------------------------------------------

#include <Nsound/Buffer.h>
#include <Nsound/Pluck.h>

#include "gold_check.h"

#include <catch2/catch_test_macros.hpp>

#include <cmath>

using namespace Nsound;

//! The energy of b from start for n samples.
static
float64
energy(const Buffer & b, uint32 start, uint32 n)
{
    Buffer s = b.subbuffer(start, n);
    s *= s;
    return s.getSum();
}

TEST_CASE("Pluck")
{
    Pluck p(8000.0, 10);

    Buffer y = p.generate(1.0, 400.0);

    SECTION("a second, normalized")
    {
        CHECK(y.getLength() == 8000);
        CHECK(std::fabs(y.getAbs().getMax() - 1.0) < 1e-12);
    }

    SECTION("it decays")
    {
        CHECK(energy(y, 6000, 2000) < 0.5 * energy(y, 0, 2000));
    }

    SECTION("it repeats every period, 20 samples at 400 Hz")
    {
        Buffer middle = y.subbuffer(2000, 2000);

        uint32 best_lag = 0;
        float64 best = -1e300;

        for(uint32 lag = 10; lag <= 30; ++lag)
        {
            float64 c = 0.0;

            for(uint32 i = 0; i + lag < middle.getLength(); ++i)
            {
                c += middle[i] * middle[i + lag];
            }

            if(c > best)
            {
                best = c;
                best_lag = lag;
            }
        }

        // The delay line is a period, and the loop's filters add a sample.
        CHECK(best_lag >= 20);
        CHECK(best_lag <= 21);
    }

    SECTION("shorter than a period throws")
    {
        CHECK_THROWS_AS(p.generate(0.001, 400.0), Exception);
    }
}
