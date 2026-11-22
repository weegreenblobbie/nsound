//-----------------------------------------------------------------------------
//
//  Tests of the binary stream operators and Tic() / Toc().
//
//-----------------------------------------------------------------------------

#include <Nsound/Nsound.h>
#include <Nsound/StreamOperators.h>
#include <Nsound/TicToc.h>

#include <catch2/catch_test_macros.hpp>

#include <sstream>

using namespace Nsound;

TEST_CASE("operator& writes and reads the bytes back")
{
    std::ostringstream out;

    char c = 'n';
    uint32 u32 = 4000000000u;
    uint64 u64 = 12345678901234ull;
    float32 f32 = 1.5f;
    float64 f64 = -2.25;

    out & c & u32 & u64 & f32 & f64;

    CHECK(out.str().size() == 1 + 4 + 8 + 4 + 8);

    std::istringstream in(out.str());

    char c2 = 0;
    uint32 u32b = 0;
    uint64 u64b = 0;
    float32 f32b = 0.0f;
    float64 f64b = 0.0;

    in & c2 & u32b & u64b & f32b & f64b;

    CHECK(c2 == c);
    CHECK(u32b == u32);
    CHECK(u64b == u64);
    CHECK(f32b == f32);
    CHECK(f64b == f64);
}

TEST_CASE("Tic() and Toc()")
{
    Tic();

    CHECK(Toc() >= 0.0);
}
