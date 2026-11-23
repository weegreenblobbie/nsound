//-----------------------------------------------------------------------------
//
//  Plotter tests, they plot with matplotlib's Agg backend in Bazel's Python
//  and check what matplotlib drew.
//
//-----------------------------------------------------------------------------

#include <Nsound/Nsound.h>

#ifdef NSOUND_C_PYLAB
    #include "plot_check.h"
#endif

#include <Nsound/AudioStream.h>
#include <Nsound/Buffer.h>
#include <Nsound/FFTChunk.h>
#include <Nsound/FFTransform.h>
#include <Nsound/Plotter.h>
#include <Nsound/Sine.h>

#include <catch2/catch_test_macros.hpp>

#include <fstream>
#include <utility>

using namespace Nsound;

#ifdef NSOUND_C_PYLAB

using namespace plot_check;

namespace
{

//! Returns the width and height in a PNG file's header.
std::pair<uint32, uint32>
png_size(const std::string & filename)
{
    std::ifstream file(filename, std::ios::binary);

    unsigned char header[24] = {};
    file.read(reinterpret_cast<char *>(header), sizeof(header));

    REQUIRE(file.good());
    REQUIRE(header[1] == 'P');
    REQUIRE(header[2] == 'N');
    REQUIRE(header[3] == 'G');

    // The IHDR chunk, big endian.
    auto read32 = [&](int i)
    {
        return (uint32(header[i]) << 24) | (uint32(header[i + 1]) << 16)
            | (uint32(header[i + 2]) << 8) | uint32(header[i + 3]);
    };

    return {read32(16), read32(20)};
}

//! Returns whether the axes draw their x grid lines.
bool
grid_is_on(const py::object & axes)
{
    auto gridlines = axes.attr("xaxis").attr("get_gridlines")()
        .cast<std::vector<py::object>>();

    REQUIRE_FALSE(gridlines.empty());

    return gridlines[0].attr("get_visible")().cast<bool>();
}

} // namespace

TEST_CASE("Plotter plots with matplotlib")
{
    // Fails when Python or matplotlib can't be loaded.
    pyplot();
}

TEST_CASE("Plotter figure() and hide()")
{
    Plotter pylab;

    pylab.hide();
    CHECK(n_figures() == 0);

    pylab.figure();
    pylab.figure("figsize=(3, 2)");

    CHECK(n_figures() == 2);

    auto size = pyplot().attr("gcf")().attr("get_size_inches")()
        .cast<std::vector<double>>();

    CHECK(size == std::vector<double>({3.0, 2.0}));

    pylab.hide();
    CHECK(n_figures() == 0);
}

TEST_CASE("Plotter plot()")
{
    Plotter pylab;
    pylab.figure();

    Buffer x({0.5, 1.5, 2.5});
    Buffer y({1.0, 4.0, 9.0});

    SECTION("y against its indices, with the grid on")
    {
        pylab.plot(y);

        auto line = lines(gca()).back();

        check_data(xdata(line), Buffer({0.0, 1.0, 2.0}));
        check_data(ydata(line), y);

        CHECK(grid_is_on(gca()));
    }

    SECTION("x and y with a format and kwargs")
    {
        pylab.plot(x, y, "r--", "linewidth=2.5");

        auto line = lines(gca()).back();

        check_data(xdata(line), x);
        check_data(ydata(line), y);

        CHECK(line.attr("get_color")().cast<std::string>() == "r");
        CHECK(line.attr("get_linestyle")().cast<std::string>() == "--");
        CHECK(line.attr("get_linewidth")().cast<double>() == 2.5);
    }

    SECTION("a point")
    {
        pylab.plot(1.5, 2.5, "o");

        auto line = lines(gca()).back();

        check_data(xdata(line), Buffer({1.5}));
        check_data(ydata(line), Buffer({2.5}));
        CHECK(line.attr("get_marker")().cast<std::string>() == "o");
    }

    SECTION("bad arguments")
    {
        CHECK_THROWS_AS(pylab.plot(Buffer()), Exception);
        CHECK_THROWS_AS(pylab.plot(Buffer({1.0}), y), Exception);

        // The kwargs must be Python.
        CHECK_THROWS_AS(pylab.plot(y, "", "linewidth="), py::error_already_set);
    }

    pylab.hide();
}

TEST_CASE("Plotter kwargs are parsed, never run")
{
    Plotter pylab;
    pylab.figure();

    Buffer y({1.0, 2.0, 3.0});

    auto builtins = py::module_::import("builtins");

    // Each would set builtins.nsound_ran if it were run.
    const std::string ran = "__import__('builtins').setattr(__import__('builtins'), 'nsound_ran', 1)";

    SECTION("a value that isn't a literal")
    {
        CHECK_THROWS_AS(pylab.plot(y, "", "linewidth=" + ran), py::error_already_set);
    }

    SECTION("code after the keyword arguments")
    {
        CHECK_THROWS_AS(pylab.plot(y, "", "linewidth=2) or " + ran + " or f("), py::error_already_set);
    }

    SECTION("a positional argument")
    {
        CHECK_THROWS_AS(pylab.plot(y, "", ran), py::error_already_set);
    }

    SECTION("**")
    {
        CHECK_THROWS_AS(pylab.plot(y, "", "**{'color': 'red'}"), py::error_already_set);
    }

    CHECK_FALSE(py::hasattr(builtins, "nsound_ran"));

    SECTION("literals are passed on")
    {
        pylab.plot(y, "", "color='red', linewidth=2.5, label=\"it's\", dashes=(2, 1)");

        py::object line = pyplot().attr("gca")().attr("get_lines")()[py::int_(0)];

        CHECK(line.attr("get_color")().cast<std::string>() == "red");
        CHECK(line.attr("get_linewidth")().cast<double>() == 2.5);
        CHECK(line.attr("get_label")().cast<std::string>() == "it's");
    }

    pylab.hide();
}

TEST_CASE("Plotter axhline(), axvline(), text() and the labels")
{
    Plotter pylab;
    pylab.figure();

    pylab.axhline(0.25, "color='red'");
    pylab.axvline(0.75);
    pylab.text(1.5, 2.5, "hello", "fontsize=7");
    pylab.title("the title", "fontsize=9");
    pylab.xlabel("the x label");
    pylab.ylabel("the y label");

    auto axes = gca();
    auto drawn = lines(axes);

    REQUIRE(drawn.size() == 2);

    check_data(ydata(drawn[0]), Buffer({0.25, 0.25}));
    CHECK(drawn[0].attr("get_color")().cast<std::string>() == "red");
    check_data(xdata(drawn[1]), Buffer({0.75, 0.75}));

    auto texts = axes.attr("texts").cast<std::vector<py::object>>();

    REQUIRE(texts.size() == 1);
    CHECK(text(texts[0]) == "hello");
    CHECK(texts[0].attr("get_position")().cast<std::vector<double>>()
        == std::vector<double>({1.5, 2.5}));
    CHECK(texts[0].attr("get_fontsize")().cast<double>() == 7.0);

    CHECK(text(axes.attr("title")) == "the title");
    CHECK(axes.attr("title").attr("get_fontsize")().cast<double>() == 9.0);
    CHECK(text(axes.attr("xaxis").attr("label")) == "the x label");
    CHECK(text(axes.attr("yaxis").attr("label")) == "the y label");

    pylab.hide();
}

TEST_CASE("Plotter xlim(), ylim(), the scales and grid()")
{
    Plotter pylab;
    pylab.figure();
    pylab.plot(Buffer({1.0, 2.0, 3.0}));

    pylab.xlim(0.5, 1.5);
    pylab.ylim(-2.0, 4.0);

    auto axes = gca();

    CHECK(axes.attr("get_xlim")().cast<std::vector<double>>()
        == std::vector<double>({0.5, 1.5}));
    CHECK(axes.attr("get_ylim")().cast<std::vector<double>>()
        == std::vector<double>({-2.0, 4.0}));

    CHECK_THROWS_AS(pylab.xlim(1.0, 1.0), Exception);
    CHECK_THROWS_AS(pylab.ylim(2.0, 1.0), Exception);

    pylab.set_xscale("log");
    pylab.set_yscale("log");

    CHECK(axes.attr("get_xscale")().cast<std::string>() == "log");
    CHECK(axes.attr("get_yscale")().cast<std::string>() == "log");

    // Only log and linear are taken.
    pylab.set_xscale("symlog");
    CHECK(axes.attr("get_xscale")().cast<std::string>() == "log");

    pylab.set_xscale("linear");
    CHECK(axes.attr("get_xscale")().cast<std::string>() == "linear");

    pylab.grid(false);
    CHECK_FALSE(grid_is_on(axes));

    pylab.grid(true);
    CHECK(grid_is_on(axes));

    pylab.hide();
}

TEST_CASE("Plotter legend()")
{
    Plotter pylab;
    pylab.figure();

    pylab.plot(Buffer({1.0, 2.0}), "", "label='first'");
    pylab.plot(Buffer({2.0, 1.0}), "", "label='second'");
    pylab.legend("loc='upper left'");

    auto legend = gca().attr("get_legend")();

    REQUIRE_FALSE(legend.is_none());

    auto texts = legend.attr("get_texts")().cast<std::vector<py::object>>();

    REQUIRE(texts.size() == 2);
    CHECK(text(texts[0]) == "first");
    CHECK(text(texts[1]) == "second");

    pylab.hide();
}

TEST_CASE("Plotter imagesc()")
{
    Plotter pylab;
    pylab.figure();

    // 2 channels of 3 samples: rows of the image.
    AudioStream z(1.0, 2);
    z[0] << 1.0 << 2.0 << 3.0;
    z[1] << 4.0 << 5.0 << 6.0;

    auto image = [&]()
    {
        return gca().attr("get_images")().cast<std::vector<py::object>>().back();
    };

    auto extent = [&]()
    {
        return image().attr("get_extent")().cast<std::vector<double>>();
    };

    SECTION("the data, centered on the indices")
    {
        pylab.imagesc(z);

        auto rows = image().attr("get_array")().attr("tolist")()
            .cast<std::vector<std::vector<double>>>();

        REQUIRE(rows.size() == 2);
        check_data(rows[0], z[0]);
        check_data(rows[1], z[1]);

        CHECK(extent() == std::vector<double>({-0.5, 2.5, -0.5, 1.5}));
        CHECK(image().attr("origin").cast<std::string>() == "lower");
        CHECK(grid_is_on(gca()));
    }

    SECTION("centered on the axes")
    {
        pylab.imagesc(Buffer({10.0, 20.0, 30.0}), Buffer({1.0, 3.0}), z);

        CHECK(extent() == std::vector<double>({5.0, 35.0, 0.0, 4.0}));
    }

    SECTION("kwargs change the defaults")
    {
        pylab.imagesc(z, "origin='upper'");

        CHECK(image().attr("origin").cast<std::string>() == "upper");
    }

    SECTION("the shapes must match")
    {
        CHECK_THROWS_AS(pylab.imagesc(AudioStream(1.0, 2)), Exception);
        CHECK_THROWS_AS(pylab.imagesc(Buffer({0.0, 1.0}), Buffer({0.0, 1.0}), z), Exception);
        CHECK_THROWS_AS(pylab.imagesc(Buffer({0.0, 1.0, 2.0}), Buffer({0.0, 1.0, 2.0}), z), Exception);
    }

    pylab.hide();
}

TEST_CASE("Plotter subplot()")
{
    Plotter pylab;
    pylab.figure();

    Axes a = pylab.subplot(2, 1, 1);

    CHECK(a.get_axes() == gca().ptr());
    CHECK(grid_is_on(gca()));

    Axes b = pylab.subplot(2, 1, 2, "", &a, &a);

    REQUIRE(figure_axes().size() == 2);

    py::object a_axes = py::reinterpret_borrow<py::object>(a.get_axes());
    py::object b_axes = py::reinterpret_borrow<py::object>(b.get_axes());

    CHECK(a_axes.attr("get_shared_x_axes")().attr("joined")(a_axes, b_axes).cast<bool>());
    CHECK(a_axes.attr("get_shared_y_axes")().attr("joined")(a_axes, b_axes).cast<bool>());

    SECTION("copies hold the same axes")
    {
        Axes copy(a);
        b = copy;

        Axes & same = b;
        b = same;

        CHECK(b.get_axes() == a.get_axes());
    }

    pylab.hide();
}

TEST_CASE("Plotter savefig() and show()")
{
    Plotter pylab;
    pylab.figure("figsize=(4, 3)");
    pylab.plot(Buffer({1.0, 2.0}));

    pylab.savefig("plot.png", "dpi=50");

    CHECK(png_size("plot.png") == std::make_pair(uint32(200), uint32(150)));

    // The Agg backend shows nothing.
    CHECK_NOTHROW(Plotter::show());

    pylab.hide();
}

TEST_CASE("FFTChunk plot()")
{
    Sine sine(1000.0);
    FFTransform t(1000.0);

    FFTChunkVector chunks = t.fft(sine.generate(0.1, 125.0), 128, 0);

    REQUIRE_FALSE(chunks.empty());

    const FFTChunk & chunk = chunks[0];

    Buffer x = chunk.getFrequencyAxis();
    uint32 n = x.getLength();

    SECTION("the magnitude, not in dB")
    {
        chunk.plot("spectrum", false);

        REQUIRE(figure_axes().size() == 1);

        auto axes = gca();

        CHECK(text(axes.attr("title")) == "spectrum");
        CHECK(text(axes.attr("yaxis").attr("label")) == "Magnitude");

        check_data(xdata(lines(axes)[0]), x);
        check_data(ydata(lines(axes)[0]), chunk.getMagnitude().subbuffer(0, n));
    }

    SECTION("in dB, with the phase")
    {
        chunk.plot("", true, true);

        auto axes = figure_axes();

        REQUIRE(axes.size() == 2);

        CHECK(text(axes[0].attr("title")) == "Magnitude");
        CHECK(text(axes[0].attr("yaxis").attr("label")) == "Magnitude dB");
        check_data(ydata(lines(axes[0])[0]), chunk.getMagnitude().getdB().subbuffer(0, n));

        CHECK(text(axes[1].attr("title")) == "Phase");
        check_data(ydata(lines(axes[1])[0]), chunk.getPhase().subbuffer(0, n));
    }

    pyplot().attr("close")("all");
}

#else

TEST_CASE("Plotter does nothing without matplotlib")
{
    Plotter pylab;

    CHECK_NOTHROW(pylab.plot(Buffer({1.0, 2.0})));
    CHECK(pylab.subplot(1, 1, 1).get_axes() == nullptr);
}

#endif
