//-----------------------------------------------------------------------------
//
//  $Id: Plotter.h 912 2015-07-26 00:50:29Z weegreenblobbie $
//
//  Nsound is a C++ library and Python module for audio synthesis featuring
//  dynamic digital filters. Nsound lets you easily shape waveforms and write
//  to disk or plot them. Nsound aims to be as powerful as Csound but easy to
//  use.
//
//  Copyright (c) 2005-Present Nick Hilton
//
//  weegreenblobbie2_gmail_com (replace '_' with '@' and '.')
//
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Library General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
//-----------------------------------------------------------------------------
#pragma once

#include <Nsound/Nsound.h>

#include <string>

// Python's object type, declared the way Python.h declares it, so this header
// doesn't need Python.h.
struct _object;
typedef struct _object PyObject;

namespace Nsound
{

class AudioStream;
class Buffer;

//-----------------------------------------------------------------------------
//! A matplotlib Axes, as Plotter::subplot() returns it.
//
//! It holds a reference to the Python object, none when plotting is off.
class Axes
{
    public:

    Axes(PyObject * axes_ptr);
    Axes(const Axes & copy);
    Axes & operator=(const Axes & rhs);
    ~Axes();

    //! Returns the matplotlib Axes, nullptr when plotting is off.
    PyObject * get_axes(){return axes_ptr_;};

    private:

    PyObject * axes_ptr_;
};

//-----------------------------------------------------------------------------
//! Plots with matplotlib's pyplot.
//
//! In a C++ program the first Plotter starts Bazel's Python, which stays
//! running until the program ends; in the Python module it uses the running
//! Python.  When Python or matplotlib can't be loaded it prints why, once, and
//! the plotting functions do nothing.  The kwargs strings are Python keyword
//! arguments, e.g. "color='red', linewidth=2".  Python errors are thrown as
//! pybind11::error_already_set.
class Plotter
{
    public:

    //! Construct the Plotter.
    Plotter();

    //! Draws a horizontal line at y and spans xmin to xmax (ralitive).
    void
    axhline(
        const float64 & y_pos = 0.0,
        const std::string & kwargs = "");

    //! Draws a vertical line at x and spans ymin to ymax (ralitive).
    void
    axvline(
        const float64 & x_pos = 0.0,
        const std::string & kwargs = "");

    //! Sets the limit for the x & y axis.
    void xlim(const float64 & xmin, const float64 & xmax);
    void ylim(const float64 & ymin, const float64 & ymax);

    //! Creates a new figure window to plot in.
    void figure(const std::string & kwargs="") const;

    //! Sets the grid state.
    void grid(boolean flag) const;

    //! Closes all the figures, so none is shown.  Useful if only saving plots
    //! to disk.
    void hide();

    //! Plots the AudioStream like a 2D matrix.
    void imagesc(
        const AudioStream & Z,
        const std::string & kwargs="");

    //! Plots the AudioStream like a 2D matrix with axis labels.
    void imagesc(
        const Buffer & x_axis,
        const Buffer & y_axis,
        const AudioStream & Z,
        const std::string & kwargs="");

    //! Shows the legend on the current axes.
    void legend(const std::string & kwargs="");

    //! Plots the Buffer on the current figure.
    void plot(
        const Buffer & y,
        const std::string & fmt = "",
        const std::string & kwargs = "");

    //! Plots x vs y on the current figure, length(x) must equal length(y)
    void plot(
        const Buffer & x,
        const Buffer & y,
        const std::string & fmt = "",
        const std::string & kwargs = "");

    //! Plots x vs y on the current figure
    void plot(
        const float64 & x,
        const float64 & y,
        const std::string & fmt = "",
        const std::string & kwargs = "");

    //! Runs the Python code in __main__, where pyplot is _pyplot_nsound_.
    void run_string(const std::string & command) const;

    //! Saves the current figure to filename.
    void
    savefig(const std::string & filename, const std::string & kwargs = "");

    //! Sets the x axis scaling: log or linear
    void
    set_xscale(const std::string & s) const;

    //! Sets the y axis scaling: log or linear
    void
    set_yscale(const std::string & s) const;

    //! Creates a figure in a subplot, subplot(A, B, C, **kwargs)
    //
    //! A = number of rows
    //! B = number of columns
    //! C = axes number
    //! kwargs - keyword arguments
    //! sharex - an axes to share xlimits with
    //! sharey - an axes to share ylimits with
    Axes
    subplot(
        const uint32 n_rows,
        const uint32 n_cols,
        const uint32 n,
        const std::string & kwargs = "",
        Axes * sharex = nullptr,
        Axes * sharey = nullptr);

    //! Adds text at the x, y data point
    void text(
        const float64 & x,
        const float64 & y,
        const std::string & text,
        const std::string & kwargs = "");

    //! Add a title to the plot at the top and centered.
    void title(const std::string & title, const std::string & kwargs="");

    //! Add a label x axis
    void xlabel(const std::string & label, const std::string & kwargs="");

    //! Add a label y axis
    void ylabel(const std::string & label, const std::string & kwargs="");

    //! Actually draw the plots to the screen
    static
    void
    show();
};

} // namespace

// :mode=c++: jEdit modeline
