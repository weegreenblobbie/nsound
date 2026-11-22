//-----------------------------------------------------------------------------
//
//  Kernel tests, the b and a coefficients of a filter.
//
//-----------------------------------------------------------------------------

#include <Nsound/Buffer.h>
#include <Nsound/Kernel.h>

#include "gold_check.h"

#include <catch2/catch_test_macros.hpp>

#include <sstream>

using namespace Nsound;

TEST_CASE("Kernel coefficients")
{
    Kernel k(3, 2);

    SECTION("setB() and setA() from Buffers, the extra ignored")
    {
        k.setB(Buffer({1.0, -2.0, 3.0, 99.0}));
        k.setA(Buffer({-4.0}));

        CHECK(k.getB(0) == 1.0);
        CHECK(k.getB(2) == 3.0);
        CHECK(k.getA(0) == -4.0);
        CHECK(k.getA(1) == 0.0);

        // Past the end reads 0.
        CHECK(k.getB(3) == 0.0);
    }

    SECTION("getSum() of the magnitudes")
    {
        k.setB(Buffer({1.0, -2.0, 3.0}));
        k.setA(Buffer({-4.0, 5.0}));

        CHECK(k.getSum() == 15.0);
    }

    SECTION("ga_interleave() swaps the odd b and a")
    {
        k.setB(Buffer({1.0, 2.0, 3.0}));
        k.setA(Buffer({4.0, 5.0}));

        k.ga_interleave();

        CHECK(k.getB(0) == 1.0);
        CHECK(k.getB(1) == 5.0);
        CHECK(k.getB(2) == 3.0);
        CHECK(k.getA(0) == 4.0);
        CHECK(k.getA(1) == 2.0);
    }

    SECTION("operator<< prints them")
    {
        k.setB(Buffer({1.0, 2.0, 3.0}));

        std::ostringstream out;
        out << k;

        CHECK_FALSE(out.str().empty());
    }
}
