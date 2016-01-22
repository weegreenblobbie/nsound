//-----------------------------------------------------------------------------
//
//  $Id: RandomNumberGenerator.h 874 2014-09-08 02:21:29Z weegreenblobbie $
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
#ifndef _NSOUND_RANDOM_NUMBER_GENERATOR_H_
#define _NSOUND_RANDOM_NUMBER_GENERATOR_H_

#include <Nsound/Nsound.h>

namespace Nsound
{

//-----------------------------------------------------------------------------
//! Base class for random number generators.
class RandomNumberGenerator
{
    public:

    virtual
    ~RandomNumberGenerator(){};

    //! Get a random number.
    virtual
    uint32
    get() = 0;

    //! Get a random float64 between min & max.
    virtual
    float64
    get(const float64 & min, const float64 & max) = 0;

    //! Set the seed to use.
    virtual
    void
    setSeed(uint32 seed) = 0;

}; // class Filter

} // namespace

// :mode=c++: jEdit modeline
#endif
