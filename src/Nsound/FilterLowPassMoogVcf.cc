//-----------------------------------------------------------------------------
//
//  $Id: FilterLowPassMoogVcf.cc 911 2015-07-10 03:04:24Z weegreenblobbie $
//
//  Copyright (c) 2006 Nick Hilton
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

#include <Nsound/FilterLowPassMoogVcf.h>
#include <Nsound/Plotter.h>

#include <cmath>
#include <cstdio>
#include <iostream>

using namespace Nsound;

//-----------------------------------------------------------------------------
// Reference:
//    http://www.musicdsp.org/archive.php?classid=3#24
//


FilterLowPassMoogVcf::
FilterLowPassMoogVcf(
    const float64 & sample_rate,
    const float64 & cutoff_hz,
    const float64 & resonance)
    :
    Filter(sample_rate),
    resonance_(resonance),
    cutoff_(cutoff_hz),
    k_(0),
    p_(0),
    r_(0),
    oldx_(0),
    y0_(0),
    y1_(0),
    y2_(0),
    y3_(0),
    oldy0_(0),
    oldy1_(0),
    oldy2_(0)
{
    _make_filter(cutoff_hz, resonance);
}


void
FilterLowPassMoogVcf::
_make_filter(float64 cutoff_hz, float64 resonance)
{
    // normalized, f = [0 : 1.0] (percent of nyquest)
    float64 f = 2.0 * cutoff_hz * sample_time_;

    k_ = 3.6 * f - 1.6 * f * f - 1.0;
    p_ = (k_ + 1.0) * 0.5;

    float64 scale = std::exp((1.0 - p_) * 1.386249);

    r_ = resonance * scale;
}


float64
FilterLowPassMoogVcf::
filter(const float64 & input)
{
    // Inverted feed back for corner peaking

    float64 x = input - r_ * y3_;

    // Four cascaded onepole filters (binlinear transform)

    y0_ = (  x + oldx_ ) * p_ - k_ * y0_;
    y1_ = (y0_ + oldy0_) * p_ - k_ * y1_;
    y2_ = (y1_ + oldy1_) * p_ - k_ * y2_;
    y3_ = (y2_ + oldy2_) * p_ - k_ * y3_;

    // Clipper band limited sigmoid
    y3_ = y3_ - y3_ * y3_ * y3_ / 6.0;

    oldx_  = x;
    oldy0_ = y0_;
    oldy1_ = y1_;
    oldy2_ = y2_;

    return y3_;
}


float64
FilterLowPassMoogVcf::
filter(const float64 & input, const float64 & cutoff_hz)
{
    _make_filter(cutoff_hz, resonance_);

    return filter(input);
}


float64
FilterLowPassMoogVcf::
filter(
    const float64 & input,
    const float64 & cutoff_hz,
    const float64 & resonance)
{
    _make_filter(cutoff_hz, resonance);

    return filter(input);
}


void
FilterLowPassMoogVcf::
plot(boolean show_fc, boolean show_phase)
{
    char title[128];
    sprintf(title,
        "Moog 24 dB Resonant Lowpass\n"
        "Resonance = %.3f, Cutoff = %0.1f Hz, sr = %0.1f Hz",
        resonance_,
        cutoff_,
        sample_rate_);

    Filter::plot(show_phase);

    Plotter pylab;

    uint32 n_rows = 1;

    if(show_phase)
    {
        n_rows = 2;
    }

    if(show_fc)
    {
        pylab.subplot(n_rows, 1, 1);

        pylab.axvline(cutoff_,  "color='red'");

        pylab.title(title);
    }
}


void
FilterLowPassMoogVcf::
reset()
{
    oldx_ = 0;
    y0_   = 0;
    y1_   = 0;
    y2_   = 0;
    y3_   = 0;
    oldy0_ = 0;
    oldy1_ = 0;
    oldy2_ = 0;

    _make_filter(cutoff_, resonance_);
}

