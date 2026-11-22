//-----------------------------------------------------------------------------
//
//  Mesh2D tests, a 2D waveguide mesh struck and listened to at its center.
//
//-----------------------------------------------------------------------------

#include <Nsound/Buffer.h>
#include <Nsound/Mesh2D.h>

#include "gold_check.h"

#include <catch2/catch_test_macros.hpp>

#include <fstream>
#include <string>

using namespace Nsound;

static Mesh2D make_mesh() { return Mesh2D(44100.0, 8, 8, 0.99999, 0.2); }

TEST_CASE("Mesh2D strike()")
{
    Mesh2D m = make_mesh();

    Buffer a = m.strike(0.2, 0.2, 1.0);

    SECTION("a second at 44100 Hz that rings")
    {
        REQUIRE(a.getLength() == 44101);
        CHECK(a.getAbs().getMax() > 0.1);
        CHECK(m.getEnergy() > 0.0);
    }

    SECTION("each strike starts from rest")
    {
        // It carried on from the last strike.
        check_close(m.strike(0.2, 0.2, 1.0), a, 0.0);
    }

    SECTION("the position matters")
    {
        // Every position used to sound the same.
        CHECK((m.strike(0.8, 0.6, 1.0) - a).getAbs().getMax() > 0.1);
    }

    SECTION("clear() stops it")
    {
        m.clear();

        CHECK(m.getEnergy() == 0.0);
    }
}

TEST_CASE("Mesh2D strike() of Buffers carries on")
{
    Mesh2D m = make_mesh();

    // A strike then silence, a sample at a time.
    Buffer v = Buffer::zeros(100);
    v[0] = 1.0;

    Buffer y = m.strike(Buffer::ones(1) * 0.2, Buffer::ones(1) * 0.2, v);

    REQUIRE(y.getLength() == 100);
    CHECK(y.getAbs().getMax() > 0.0);

    // The next call carries on ringing without a new strike.
    Buffer more = m.strike(Buffer::ones(1) * 0.2, Buffer::ones(1) * 0.2, Buffer::zeros(10));

    CHECK(more.getAbs().getMax() > 0.0);
}

TEST_CASE("Mesh2D copies")
{
    Mesh2D m = make_mesh();

    Buffer expected = m.strike(0.3, 0.4, 1.0);

    SECTION("the copy constructor")
    {
        // A copy had a sample rate of 0, so its attack wasn't shaped.
        Mesh2D copy(m);

        check_close(copy.strike(0.3, 0.4, 1.0), expected, 0.0);
    }

    SECTION("operator= onto another size, with the state")
    {
        Mesh2D ringing = make_mesh();

        Buffer v = Buffer::zeros(50);
        v[0] = 1.0;
        ringing.strike(Buffer::ones(1) * 0.3, Buffer::ones(1) * 0.3, v);

        Mesh2D other(8000.0, 3, 5, 0.9, 0.1);
        other = ringing;

        CHECK(other.getEnergy() == ringing.getEnergy());

        Mesh2D & same = other;
        other = same;

        Buffer quiet = Buffer::zeros(20);

        check_close(
            other.strike(Buffer::ones(1), Buffer::ones(1), quiet),
            ringing.strike(Buffer::ones(1), Buffer::ones(1), quiet),
            0.0);
    }
}

TEST_CASE("Mesh2D checks its size")
{
    // An empty mesh read past its end.
    CHECK_THROWS_AS(Mesh2D(44100.0, 0, 8, 0.99, 0.2), Exception);
    CHECK_THROWS_AS(Mesh2D(44100.0, 8, 0, 0.99, 0.2), Exception);
}

TEST_CASE("Mesh2D dumps the mesh")
{
    Mesh2D m(44100.0, 3, 3, 0.99, 0.2);

    m.setMeshDumping(true, ".", "dump");
    m.strike(0.5, 0.5, 1.0);

    SECTION("the map numbers the junctions, top row first")
    {
        std::ifstream map("dump_map.txt");
        REQUIRE(map.good());

        int32 first = -1;
        map >> first;
        CHECK(first == 6);
    }

    SECTION("a line of velocities per tick")
    {
        std::ifstream mesh("dump.txt");
        REQUIRE(mesh.good());

        uint32 lines = 0;
        std::string line;
        while(std::getline(mesh, line)) ++lines;

        CHECK(lines == 44101);
    }

    SECTION("a directory that isn't there throws")
    {
        m.setMeshDumping(true, "no/such/dir");

        CHECK_THROWS_AS(m.strike(0.5, 0.5, 1.0), Exception);
    }
}

TEST_CASE("Mesh2D limits tau to (0, 0.5]")
{
    Mesh2D high(44100.0, 4, 4, 0.99, 0.9);
    Mesh2D half(44100.0, 4, 4, 0.99, 0.5);

    check_close(high.strike(0.3, 0.3, 1.0), half.strike(0.3, 0.3, 1.0), 0.0);

    Mesh2D low(44100.0, 4, 4, 0.99, -1.0);
    Mesh2D tiny(44100.0, 4, 4, 0.99, 1e-6);

    check_close(low.strike(0.3, 0.3, 1.0), tiny.strike(0.3, 0.3, 1.0), 0.0);
}
