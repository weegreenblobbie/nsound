//-----------------------------------------------------------------------------
//
//  $Id: RngTausworthe.h 874 2014-09-08 02:21:29Z weegreenblobbie $
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
#ifndef _NSOUND_RNG_TAUSWORTHE_H_
#define _NSOUND_RNG_TAUSWORTHE_H_

#include <Nsound/Nsound.h>
#include <Nsound/RandomNumberGenerator.h>

namespace Nsound
{

//-----------------------------------------------------------------------------
//! An implementation of the Tausworthe random number algorithm found in the
//! GNU Scientific Library.
class RngTausworthe : public RandomNumberGenerator
{
    public:

    //! Default seed used is the number of seconds from unix epoch.
    RngTausworthe();

    //! Get a random number.
    uint32
    get();

    //! Get a random float64 between min & max.
    int32
    get(const Nsound::int32 min, const Nsound::int32 max);

    //! Get a random float64 between min & max.
    float64
    get(const Nsound::float64 & min, const Nsound::float64 & max);

    //! assignment operator
    RngTausworthe &
    operator=(const RngTausworthe & rhs);

    //! Set the seed to use.
    void
    setSeed(Nsound::uint32 seed);

    private:

    Nsound::uint32 s1_;
    Nsound::uint32 s2_;
    Nsound::uint32 s3_;
};

} // namespace

#endif

// :mode=c++: jEdit modeline
