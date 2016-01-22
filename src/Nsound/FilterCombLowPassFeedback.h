//-----------------------------------------------------------------------------
//
//  $Id: FilterCombLowPassFeedback.h 874 2014-09-08 02:21:29Z weegreenblobbie $
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
#ifndef _NSOUND_FILTER_COMB_LOW_PASS_FEEDBACK_H_
#define _NSOUND_FILTER_COMB_LOW_PASS_FEEDBACK_H_

#include <Nsound/Filter.h>

namespace Nsound
{

// Forward class declarations
class AudioStream;
class Buffer;
class FilterDelay;

//-----------------------------------------------------------------------------
//! A class for filtering audio in the frequecy domain.
class FilterCombLowPassFeedback : public Filter
{
    public:

    FilterCombLowPassFeedback(
        const float64 & sample_rate,
        const float64 & delay_time_seconds,
        const float64 & feedback_gain,
        const float64 & low_pass_frequency_Hz);

    FilterCombLowPassFeedback(const FilterCombLowPassFeedback & copy);

    virtual
    ~FilterCombLowPassFeedback();

    AudioStream
    filter(const AudioStream & x);

    AudioStream
    filter(const AudioStream & x, const Buffer & frequencies);

    Buffer
    filter(const Buffer & x);

    Buffer
    filter(const Buffer & x, const Buffer & frequencies);

    float64
    filter(const float64 & x);

    float64
    filter(const float64 & x, const float64 & frequency);

    FilterCombLowPassFeedback &
    operator=(const FilterCombLowPassFeedback & rhs);

    void
    plot(boolean show_fc = true, boolean show_phase = false);

    void
    reset();

    protected:

    FilterDelay * delay_;

    float64 feedback_gain_;

    float64 damp1_;
    float64 damp2_;

    float64 y_history_;
};

};

// :mode=c++:  jEdit modeline
#endif
