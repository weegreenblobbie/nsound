//-----------------------------------------------------------------------------
//
//  $Id: Pluck.h 874 2014-09-08 02:21:29Z weegreenblobbie $
//
//  Copyright (c) 2004-2007 Nick Hilton
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

#ifndef _NSOUND_PLUCK_H_
#define _NSOUND_PLUCK_H_

#include <Nsound/Generator.h>

namespace Nsound
{

class Buffer;

//-----------------------------------------------------------------------------
//! Implements a simple Karplus-Strong String Synthesis algorithim.
//
class Pluck : public Generator
{
    public:

    Pluck(const float64 & sample_rate, uint32 n_smooth_samples);

    virtual ~Pluck();

    //! Implements simple Karplus-Strong plucked string.
    Buffer
    generate(const float64 & duration, const float64 & frequency);

    // These don't do anything yet.
    float64 generate(const float64 & f){return 0.0;}
    float64 generate2(const float64 & f, const float64 & p){return 0.0;}

    Buffer generate2(
        const float64 & d,
        const float64 & f,
        const float64 & p){return Buffer();}

    Buffer generate(
        const float64 & d,
        const Buffer &  f){return Buffer();}

    Buffer generate2(
        const float64 & d,
        const float64 & f,
        const Buffer &  p){return Buffer();}

    Buffer generate2(
        const float64 & d,
        const Buffer &  f,
        const float64 & p){return Buffer();}

    Buffer generate2(
        const float64 & d,
        const Buffer &  f,
        const Buffer &  p){return Buffer();}

    protected:

    uint32 n_smooth_samples_;

};

};

// :mode=c++: jEdit modeline

#endif
