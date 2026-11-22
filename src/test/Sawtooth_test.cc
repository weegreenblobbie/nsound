//-----------------------------------------------------------------------------
//
//  Sawtooth tests, the sum of the first harmonics of a sawtooth.
//
//-----------------------------------------------------------------------------

#include <Nsound/Buffer.h>
#include <Nsound/Sawtooth.h>
#include <Nsound/Sine.h>

#include "gold_check.h"

#include <catch2/catch_test_macros.hpp>

#include <cmath>

using namespace Nsound;

TEST_CASE("Sawtooth")
{
    Sine sine(1000.0);

    SECTION("one harmonic is a sine")
    {
        Buffer y = Sawtooth(1000.0, 1).generate(1.0, 1.0);

        check_close(y, (2.0 / M_PI) * sine.generate(1.0, 1.0), 1e-12);
    }

    SECTION("two harmonics add the octave at half the amplitude")
    {
        Buffer y = Sawtooth(1000.0, 2).generate(1.0, 1.0);

        Buffer expected = (2.0 / M_PI) * (sine.generate(1.0, 1.0) + 0.5 * sine.generate(1.0, 2.0));

        check_close(y, expected, 1e-12);
    }

    SECTION("many harmonics ramp down through each wavelength")
    {
        Buffer y = Sawtooth(1000.0, 100).generate(1.0, 1.0);

        // (pi - 2 pi t) / 2 * 2 / pi = 1 - 2 t, away from the jump at 0.
        for(uint32 i : {100u, 250u, 500u, 750u, 900u})
        {
            INFO("sample " << i);
            CHECK(std::fabs(y[i] - (1.0 - 2.0 * i / 1000.0)) < 0.01);
        }
    }

    SECTION("0 and negative harmonics are one")
    {
        Buffer one = Sawtooth(1000.0, 1).generate(1.0, 1.0);

        check_close(Sawtooth(1000.0, 0).generate(1.0, 1.0), one, 0.0);
        check_close(Sawtooth(1000.0, -1).generate(1.0, 1.0), one, 0.0);
    }
}
