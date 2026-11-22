//-----------------------------------------------------------------------------
//
//  $Id: FilterLowPassMoogVcf.h 910 2015-07-09 03:05:58Z weegreenblobbie $
//
//  Nsound is a C++ library and Python module for audio synthesis featuring
//  dynamic digital filters. Nsound lets you easily shape waveforms and write
//  to disk or plot them. Nsound aims to be as powerful as Csound but easy to
//  use.
//
//  Copyright (c) 2006 - Present Nick Hilton
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

#include <Nsound/Filter.h>

#include <set>

namespace Nsound
{


class FilterLowPassMoogVcf : public Filter
{

public:

    FilterLowPassMoogVcf(
        const float64 & sample_rate,
        const float64 & cutoff_hz,
        const float64 & resonance);

    // The Buffer and AudioStream overloads, which the float64 ones below
    // would hide.
    using Filter::filter;

    float64
    filter(const float64 & x) override;

    float64
    filter(const float64 & x, const float64 & cutoff_hz) override;

    virtual
    float64
    filter(
        const float64 & x,
        const float64 & cutoff_hz,
        const float64 & resonance);

    void
    plot(boolean show_fc = true, boolean show_phase = false);

    //! Resets interal history buffer and sets the cut off frequency to the one
    //! used at declaration.
    void
    reset() override;

protected:

    void _make_filter(float64 cutoff_hz, float64 resonance);

    float64 resonance_;
    float64 cutoff_;
    float64 k_ = 0.0;
    float64 p_ = 0.0;
    float64 r_ = 0.0;

    float64 oldx_ = 0.0;
    float64 y0_ = 0.0, y1_ = 0.0, y2_ = 0.0, y3_ = 0.0;
    float64 oldy0_ = 0.0, oldy1_ = 0.0, oldy2_ = 0.0;


}; // class FilterLowPassMoogVcf


} // namespace Nsound

// :mode=c++: jEdit modeline
