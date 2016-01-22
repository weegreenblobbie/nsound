//-----------------------------------------------------------------------------
//
//  $Id: FilterHighPassIIR.h 874 2014-09-08 02:21:29Z weegreenblobbie $
//
//  Copyright (c) 2007 Nick Hilton
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
#ifndef _NSOUND_FILTER_HIGH_PASS_IIR_H_
#define _NSOUND_FILTER_HIGH_PASS_IIR_H_

#include <Nsound/FilterStageIIR.h>

namespace Nsound
{

// Forward class declarations
class AudioStream;
class Buffer;

//-----------------------------------------------------------------------------
//! A class for filtering audio in the frequecy domain.
class FilterHighPassIIR : public FilterStageIIR
{
    public:

    FilterHighPassIIR(
        const float64 & sample_rate,
        uint32 n_poles,
        const float64 & frequency,
        const float64 & percent_ripple = 0.0);

    AudioStream
    filter(const AudioStream & x);

    AudioStream
    filter(const AudioStream & x, const float64 & f);

    AudioStream
    filter(const AudioStream & x, const Buffer & frequencies);

    Buffer
    filter(const Buffer & x);

    Buffer
    filter(const Buffer & x, const float64 & f);

    Buffer
    filter(const Buffer & x, const Buffer & frequencies);

    float64
    filter(const float64 & x);

    float64
    filter(const float64 & x, const float64 & frequency);

    float64
    getFrequency() const {return frequency_;};

    void
    makeKernel(const float64 & frequency);

    void
    plot(boolean show_fc = true, boolean show_phase = false);

    void
    reset();

};

};

// :mode=c++:  jEdit modeline
#endif
