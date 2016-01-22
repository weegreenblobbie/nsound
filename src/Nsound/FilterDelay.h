//-----------------------------------------------------------------------------
//
//  $Id: FilterDelay.h 874 2014-09-08 02:21:29Z weegreenblobbie $
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
#ifndef _NSOUND_FILTER_DELAY_H_
#define _NSOUND_FILTER_DELAY_H_

#include <Nsound/Filter.h>

namespace Nsound
{

// Forward class declarations
class AudioStream;
class Buffer;

//-----------------------------------------------------------------------------
//! A class for filtering audio in the frequecy domain.
class FilterDelay : public Filter
{
    public:

    FilterDelay(
        const float64 & sample_rate,
        const float64 & max_delay_time_seconds);

    FilterDelay(const FilterDelay & copy);

    ~FilterDelay();

    AudioStream
    filter(const AudioStream & x);

    AudioStream
    filter(const AudioStream & x, const float64 & delay_in_seconds);

    AudioStream
    filter(const AudioStream & x, const Buffer & delay_in_seconds);

    Buffer
    filter(const Buffer & x);

    Buffer
    filter(const Buffer & x, const float64 & delay_in_seconds);

    Buffer
    filter(const Buffer & x, const Buffer & delay_in_seconds);

    float64
    filter(const float64 & x);

    float64
    filter(const float64 & x, const float64 & delay_in_seconds);

    FilterDelay &
    operator=(const FilterDelay & rhs);

    void
    reset();

    protected:

    float64 * buffer_;
    float64 * buffer_end_ptr_;
    float64 * read_ptr_;
    float64 * write_ptr_;

    float64 delay_;

    uint32 n_samples_;

};

};

// :mode=c++:  jEdit modeline
#endif
