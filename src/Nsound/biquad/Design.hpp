//-----------------------------------------------------------------------------
//
//  $Id: Design.h 932 2015-12-12 17:13:47Z weegreenblobbie $
//
//  Nsound is a C++ library and Python module for audio synthesis featuring
//  dynamic digital filters. Nsound lets you easily shape waveforms and write
//  to disk or plot them. Nsound aims to be as powerful as Csound but easy to
//  use.
//
//  Copyright (c) 2009 to Present Nick Hilton
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
#ifndef _NSOUND_BIQUAD_DESIGN_HPP_
#define _NSOUND_BIQUAD_DESIGN_HPP_

#include <Nsound/Nsound.h>

#include <Nsound/biquad/Kernel.hpp>

namespace Nsound
{
namespace biquad
{

// Reference: Sophocles J. Orfanidis, "High-Order Digital Parametric Equalizer Design," J. Audio Eng. Soc., vol. 53, pp. 1026-1046, Nov. 2005.
//            http://www.ece.rutgers.edu/~orfanidi/ece346/hpeq.pdf


enum DesignType { BUTTERWORTH, CHEBYSHEV_1, CHEBYSHEV_2, ELLIPTIC };


struct BandEdge
{
    BandEdge();
    BandEdge(float64 sample_rate, float64 freq_center_hz, float64 bandwidth_hz);
    float64 _lo_hz;
    float64 _hi_hz;
};


BiquadKernel
hpeq_design(
    float64    sample_rate,
    uint32     order,
    float64    freq_center_hz,
    float64    bandwidth_hz,
    float64    gain_reference_db,
    float64    gain_peak_db,
    float64    gain_bandwidth_db,
    DesignType type = BUTTERWORTH);


} // namespace
} // namespace

#endif