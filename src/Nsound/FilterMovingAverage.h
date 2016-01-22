//////////////////////////////////////////////////////////////////////////////
//
//  $Id: FilterMovingAverage.h 825 2014-02-22 03:39:23Z weegreenblobbie $
//
//  Nsound is a C++ library and Python module for audio synthesis featuring
//  dynamic digital filters. Nsound lets you easily shape waveforms and write
//  to disk or plot them. Nsound aims to be as powerful as Csound but easy to
//  use.
//
//  Copyright (c) 2007 Nick Hilton
//
//  weegreenblobbie_yahoo_com (replace '_' with '@' and '.')
//
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
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
//////////////////////////////////////////////////////////////////////////////
#ifndef _NSOUND_FILTER_MOVING_AVERAGE_H_
#define _NSOUND_FILTER_MOVING_AVERAGE_H_

#include <Nsound/Filter.h>

namespace Nsound
{

// Forward class declarations
class AudioStream;
class Buffer;
class FilterDelay;

//////////////////////////////////////////////////////////////////////////////
class FilterMovingAverage : public Filter
{
    //////////////////////////////////////////////////////////////////////////
    public:

    //////////////////////////////////////////////////////////////////////////
    FilterMovingAverage(uint32 n_samples_to_average);

    FilterMovingAverage(const FilterMovingAverage & copy);

    //////////////////////////////////////////////////////////////////////////
    ~FilterMovingAverage();

    //////////////////////////////////////////////////////////////////////////
    AudioStream
    filter(const AudioStream & x);

    //////////////////////////////////////////////////////////////////////////
    AudioStream
    filter(const AudioStream & x, const Buffer & frequencies)
    { return filter(x); };

    //////////////////////////////////////////////////////////////////////////
    Buffer
    filter(const Buffer & x);

    //////////////////////////////////////////////////////////////////////////
    Buffer
    filter(const Buffer & x, const Buffer & frequencies)
    { return filter(x); };

    //////////////////////////////////////////////////////////////////////////
    float64
    filter(const float64 & x);

    //////////////////////////////////////////////////////////////////////////
    float64
    filter(const float64 & x, const float64 & frequency)
    { return filter(x); };

    FilterMovingAverage &
    operator=(const FilterMovingAverage & rhs);

    //////////////////////////////////////////////////////////////////////////
    void
    reset();

    //////////////////////////////////////////////////////////////////////////
    protected:

    bool init_sum_;

    float64 n_samples_to_average_;
    float64 running_sum_;

    FilterDelay * delay_;     // used as a circular buffer
};

}; // Nsound

// :mode=c++: jEdit modeline

#endif
