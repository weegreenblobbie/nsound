//-----------------------------------------------------------------------------
//
//  Buffer::getResample() tests, compared with the gold files.
//
//-----------------------------------------------------------------------------

#include <Nsound/Buffer.h>
#include <Nsound/Sine.h>
#include <Nsound/Wavefile.h>

#include "gold_check.h"

#include <catch2/catch_test_macros.hpp>

#include <string>

using namespace Nsound;

static const float64 GAMMA = 1.5e-14;

TEST_CASE("Buffer getResample(L, M)")
{
    Wavefile::setDefaultSampleSize(64);
    Wavefile::setIEEEFloat(true);

    static const uint32 LM[4] = { 2, 3, 5, 7 };

    Sine sin(150);

    Buffer input = sin.generate(1.0, 3.0);

    for(uint32 i = 1; i <= 4; ++i)
    {
        const uint32 L = i;
        const uint32 M = LM[i - 1];

        check_gold(
            input.getResample(L, M),
            "gold/BufferResample_out_" + std::to_string(L) + "_" + std::to_string(M) + ".wav",
            GAMMA);

        // Reverse L & M
        check_gold(
            input.getResample(M, L),
            "gold/BufferResample_out_" + std::to_string(M) + "_" + std::to_string(L) + ".wav",
            GAMMA);
    }
}
