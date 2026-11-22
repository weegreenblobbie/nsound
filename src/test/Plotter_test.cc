//-----------------------------------------------------------------------------
//
//  Plotter tests.  Built without matplotlib (NSOUND_C_PYLAB), every method
//  does nothing, so these check they can all be called safely.
//
//-----------------------------------------------------------------------------

#include <Nsound/AudioStream.h>
#include <Nsound/Buffer.h>
#include <Nsound/FFTChunk.h>
#include <Nsound/FFTransform.h>
#include <Nsound/Plotter.h>
#include <Nsound/Sine.h>

#include <catch2/catch_test_macros.hpp>

using namespace Nsound;

TEST_CASE("Plotter without matplotlib")
{
    Plotter pylab;

    Buffer x({0.0, 1.0, 2.0});
    Buffer y({1.0, 4.0, 9.0});

    AudioStream z(1.0, 2);
    z << x;

    CHECK_NOTHROW(pylab.figure());
    CHECK_NOTHROW(pylab.plot(y));
    CHECK_NOTHROW(pylab.plot(x, y, "r-", "lw=2"));
    CHECK_NOTHROW(pylab.plot(1.0, 2.0, "o"));
    CHECK_NOTHROW(pylab.axhline(0.5, "color='red'"));
    CHECK_NOTHROW(pylab.axvline(0.5));
    CHECK_NOTHROW(pylab.xlim(0.0, 2.0));
    CHECK_NOTHROW(pylab.ylim(0.0, 9.0));
    CHECK_NOTHROW(pylab.grid(true));
    CHECK_NOTHROW(pylab.imagesc(z));
    CHECK_NOTHROW(pylab.imagesc(x, Buffer({0.0, 1.0}), z));
    CHECK_NOTHROW(pylab.legend());
    CHECK_NOTHROW(pylab.set_xscale("log"));
    CHECK_NOTHROW(pylab.set_yscale("log"));
    CHECK_NOTHROW(pylab.text(1.0, 1.0, "hi"));
    CHECK_NOTHROW(pylab.title("title"));
    CHECK_NOTHROW(pylab.xlabel("x"));
    CHECK_NOTHROW(pylab.ylabel("y"));
    CHECK_NOTHROW(pylab.run_string("print('hi')"));
    CHECK_NOTHROW(pylab.savefig("plot.png"));
    CHECK_NOTHROW(pylab.hide());
    CHECK_NOTHROW(Plotter::show());

    SECTION("subplots and their axes")
    {
        Axes a = pylab.subplot(2, 1, 1);
        Axes b = pylab.subplot(2, 1, 2, "", &a, &a);

        Axes copy(a);
        b = copy;

        Axes & same = b;
        b = same;

        CHECK(b.get_axes() == a.get_axes());
    }
}

TEST_CASE("FFTChunk plot() without matplotlib")
{
    Sine sine(1000.0);
    FFTransform t(1000.0);

    FFTChunkVector chunks = t.fft(sine.generate(0.1, 125.0), 128, 0);

    REQUIRE_FALSE(chunks.empty());

    CHECK_NOTHROW(chunks[0].plot("spectrum"));
    CHECK_NOTHROW(chunks[0].plot("spectrum", true, true));
}
