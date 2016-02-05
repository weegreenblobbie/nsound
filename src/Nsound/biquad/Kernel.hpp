//-----------------------------------------------------------------------------
//
//  $Id: Kernel.h 932 2015-12-12 17:13:47Z weegreenblobbie $
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
#ifndef _NSOUND_BIQUAD_KERNEL_HPP_
#define _NSOUND_BIQUAD_KERNEL_HPP_

#include <Nsound/Nsound.h>

#include <Nsound/biquad/Vector.hpp>

namespace Nsound
{
namespace biquad
{


struct BiquadKernel
{
    std::vector<float64> _b;
    std::vector<float64> _a;
};


// cascade two filters into one direct form.
BiquadKernel cas2dir(const BiquadKernel & lhs, const BiquadKernel & rhs);


//-----------------------------------------------------------------------------
// inline implementation

inline
BiquadKernel
cas2dir(const BiquadKernel & lhs, const BiquadKernel & rhs)
{
    return {convolve(lhs._b, rhs._b), convolve(lhs._a, rhs._a)};
}


} // namespace
} // namespace

#endif