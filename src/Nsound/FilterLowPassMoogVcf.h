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
//  weegreenblobbie_yahoo_com (replace '_' with '@' and '.')
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
#ifndef _NSOUND_FILTER_LOW_PASS_MOOG_VCF_H_
#define _NSOUND_FILTER_LOW_PASS_MOOG_VCF_H_

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

    virtual ~FilterLowPassMoogVcf() {}

    virtual
    float64
    filter(const float64 & x);

    virtual
    float64
    filter(const float64 & x, const float64 & cutoff_hz);

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
    reset();

protected:

    void _make_filter(float64 cutoff_hz, float64 resonance);

    float64 resonance_;
    float64 cutoff_;
    float64 k_;
    float64 p_;
    float64 r_;

    float64 oldx_;
    float64 y0_, y1_, y2_, y3_;
    float64 oldy0_, oldy1_, oldy2_;


}; // class FilterLowPassMoogVcf


} // namespace Nsound

// :mode=c++: jEdit modeline
#endif
