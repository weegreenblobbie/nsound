//-----------------------------------------------------------------------------
//
//  $Id$
//
//  Nsound is a C++ library and Python module for audio synthesis featuring
//  dynamic digital filters. Nsound lets you easily shape waveforms and write
//  to disk or plot them. Nsound aims to be as powerful as Csound but easy to
//  use.
//
//  Copyright (c) 2015 to Present Nick Hilton
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
#ifndef _NSOUND_BIQUAD_VECTOR_HPP_
#define _NSOUND_BIQUAD_VECTOR_HPP_

#include <Nsound/Nsound.h>

#include <algorithm>


namespace Nsound
{
namespace biquad
{

// Some vector utilities to be shared between Design and BiquadKernel

typedef std::vector<float64> Vector;

Vector operator * (const Vector & lhs, const Vector & rhs);
Vector operator * (const Vector & lhs, float64 rhs);
Vector operator * (float64 lhs, const Vector & rhs);

Vector convolve(const Vector & x, const Vector & h);


//-----------------------------------------------------------------------------
// inline implementation

inline
Vector operator * (const Vector & lhs, const Vector & rhs)
{
    M_ASSERT_MSG(
        lhs.size() == rhs.size(),
        "Size mismatch (" << lhs.size() << " != " << rhs.size() << ")");

    Vector out;

    std::transform(
        lhs.begin(), lhs.end(),
        rhs.begin(), std::back_inserter(out),
        std::multiplies<float64>() );

    return out;
}


inline
Vector operator * (const Vector & lhs, float64 rhs)
{
    Vector out(lhs);

    for(auto & x : out)
    {
        x *= rhs;
    }

    return out;
}


inline
Vector operator * (float64 lhs, const Vector & rhs)
{
    Vector out(rhs);

    for(auto & x : out)
    {
        x *= lhs;
    }

    return out;
}


inline
Vector
convolve(const Vector & x, const Vector & h)
{
    Vector y(x.size() + h.size() - 1, 0.0);

    for(auto i = 0u; i < x.size(); ++i)
    {
        for(auto j = 0u; j < h.size(); ++j)
        {
            y[i + j] += x[i] * h[j];
        }
    }

    return y;
}


} // namespace
} // namespace


#endif