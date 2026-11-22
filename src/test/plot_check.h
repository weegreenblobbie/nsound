//-----------------------------------------------------------------------------
//
//  Checks what Plotter drew, asking matplotlib through pybind11, for the
//  tests built with NSOUND_C_PYLAB.  Comparing data instead of images works
//  the same with every matplotlib version and platform.
//
//-----------------------------------------------------------------------------

#pragma once

// pybind11 includes Python.h, which must come first.
#include <pybind11/embed.h>
#include <pybind11/stl.h>

#include <Nsound/Buffer.h>
#include <Nsound/Plotter.h>

#include <catch2/catch_test_macros.hpp>

#include <string>
#include <vector>

namespace plot_check
{

namespace py = pybind11;

//! Returns matplotlib.pyplot, after a Plotter started Python and imported
//! it; the test fails when plotting is off.
inline
py::module_
pyplot()
{
    Nsound::Plotter plotter;

    REQUIRE(Py_IsInitialized());

    py::dict modules = py::module_::import("sys").attr("modules");

    REQUIRE(modules.contains("matplotlib.pyplot"));

    return py::module_::import("matplotlib.pyplot");
}

//! Returns the current axes.
inline
py::object
gca()
{
    return pyplot().attr("gca")();
}

//! Returns the axes of the current figure.
inline
std::vector<py::object>
figure_axes()
{
    return pyplot().attr("gcf")().attr("get_axes")().cast<std::vector<py::object>>();
}

//! Returns the lines on the axes, in the order they were drawn.
inline
std::vector<py::object>
lines(const py::object & axes)
{
    return axes.attr("get_lines")().cast<std::vector<py::object>>();
}

//! Returns a matplotlib Text's string, e.g. axes.title.
inline
std::string
text(const py::object & text)
{
    return text.attr("get_text")().cast<std::string>();
}

//! Returns a line's x data.
inline
std::vector<double>
xdata(const py::object & line)
{
    return line.attr("get_xdata")().cast<std::vector<double>>();
}

//! Returns a line's y data.
inline
std::vector<double>
ydata(const py::object & line)
{
    return line.attr("get_ydata")().cast<std::vector<double>>();
}

//! Checks the data equals the samples, they pass through Python unchanged.
inline
void
check_data(const std::vector<double> & data, const Nsound::Buffer & samples)
{
    REQUIRE(data.size() == samples.getLength());

    for(std::size_t i = 0; i < data.size(); ++i)
    {
        INFO("i = " << i);
        CHECK(data[i] == samples[static_cast<Nsound::uint32>(i)]);
    }
}

//! Returns the number of open figures.
inline
std::size_t
n_figures()
{
    return pyplot().attr("get_fignums")().cast<std::vector<int>>().size();
}

} // namespace
