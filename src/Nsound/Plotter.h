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
#ifndef _NSOUND_PLOTTER_H_
#define _NSOUND_PLOTTER_H_

#include <Nsound/Nsound.h>

#include <map>
#include <string>


namespace Nsound
{

class AudioStream;
class Buffer;

#if ! ( defined(NSOUND_C_PYLAB) || defined(NSOUND_IN_PYTHON_MODULE) )
    typedef void PyObject;
#endif

//-----------------------------------------------------------------------------
//! A wrapper around a Matplotlib Axes object.
class Axes
{
    public:

    Axes(PyObject * axes_ptr);
    Axes(const Axes & copy);
    ~Axes();
    Axes & operator=(const Axes & rhs);
    PyObject * get_axes(){return axes_ptr_;};

    private:

    PyObject * axes_ptr_;
};

//-----------------------------------------------------------------------------
// DOXME
class Plotter
{
    public:

    //! Construct the Plotter.
    Plotter();

    //! Deconstruct the Plotter.
    virtual ~Plotter();

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
    void grid(boolean flag);

    //! Hides all plots, prevents any plot window being rendered to the screen.  Useful if only saving plots to disk.
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

    //! executes the python string
    void run_string(const std::string & command) const;

    //! Saves the last figure drawn to filename.
    void
    savefig(const std::string & filename, const std::string & kwargs = "");

    //! Sets the x axis scaling: log or linear
    void
    set_xscale(const std::string & s);

    //! Sets the y axis scaling: log or linear
    void
    set_yscale(const std::string & s);

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

    //! SWIG helper function function to shadow.
    void _swig_shadow(){};

    protected:

    typedef std::pair < std::string, PyObject * > StringPyObjectPair;
    typedef std::map  < std::string, PyObject * > PyPlotTable;

    PyObject * _make_kwargs(const std::string & kwargs) const;

    static int32 count_;

    static PyPlotTable table_;

    enum PlotterState
    {
        BOOTING,
        INITALIZED,
        UNLOADED,
    };

    static PlotterState state_;

    static boolean grid_is_on_;

    static float64 xmin_;
    static float64 xmax_;
    static float64 ymin_;
    static float64 ymax_;

    // disable these
    private:

    //! Copy constructor disabled.
    Plotter(const Plotter & rhs);

    //! Assignment disabled.
    Plotter & operator=(const Plotter & rhs);

    //! Create a PyList object from a buffer!
    PyObject * makePyListFromBuffer(const Buffer & buffer) const;

    //! Create a PyInt
    PyObject * makePyIntFromUint32(const uint32 & i) const;

};

} // namespace

// :mode=c++: jEdit modeline

#endif
