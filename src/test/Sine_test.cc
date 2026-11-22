//-----------------------------------------------------------------------------
//
//  Sine tests, compared with the gold files.
//
//-----------------------------------------------------------------------------

#include <Nsound/Buffer.h>
#include <Nsound/Sine.h>
#include <Nsound/Wavefile.h>

#include "gold_check.h"

#include <catch2/catch_test_macros.hpp>

using namespace Nsound;

static const float64 GAMMA = 1.5e-14;

TEST_CASE("Sine")
{
    Wavefile::setDefaultSampleSize(64);
    Wavefile::setIEEEFloat(true);

    Sine sine(600);

    Buffer freqs = sine.drawLine(1.0, 0.0, 10.0);
    Buffer phase = sine.drawLine(1.0, 0.0, 1.0);

    SECTION("generate(1.0, 3.0)")
    {
        check_gold(sine.generate(1.0, 3.0), "gold/Sine_1.wav", GAMMA);
    }

    SECTION("generate(1.0, freqs)")
    {
        check_gold(sine.generate(1.0, freqs), "gold/Sine_2.wav", GAMMA);
    }

    SECTION("generate2(1.0, 3.0, 0.5)")
    {
        check_gold(sine.generate2(1.0, 3.0, 0.5), "gold/Sine_3.wav", GAMMA);
    }

    SECTION("generate2(1.0, 3.0, phase)")
    {
        check_gold(sine.generate2(1.0, 3.0, phase), "gold/Sine_4.wav", GAMMA);
    }

    SECTION("generate2(1.0, freqs, 0.5)")
    {
        check_gold(sine.generate2(1.0, freqs, 0.5), "gold/Sine_5.wav", GAMMA);
    }

    SECTION("generate2(1.0, freqs, phase)")
    {
        check_gold(sine.generate2(1.0, freqs, phase), "gold/Sine_6.wav", GAMMA);
    }

    SECTION("setChorus(5, 0.10)")
    {
        sine.setSeed(6846513); // Some random seed I typed in.
        sine.setChorus(5, 0.10);

        check_gold(sine.generate2(1.0, 2 * freqs, phase), "gold/Sine_7.wav", GAMMA);
    }
}
