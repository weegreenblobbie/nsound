//-----------------------------------------------------------------------------
//
//  $Id: FilterCombLowPassFeedback.cc 874 2014-09-08 02:21:29Z weegreenblobbie $
//
//  Copyright (c) 2006 Nick Hilton
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
//-----------------------------------------------------------------------------////

#include <Nsound/AudioStream.h>
#include <Nsound/Buffer.h>
#include <Nsound/FilterCombLowPassFeedback.h>
#include <Nsound/FilterDelay.h>
#include <Nsound/Plotter.h>

#include <cmath>
#include <cstdio>
#include <iostream>

using namespace Nsound;

using std::cerr;
using std::endl;

#define CERR_HEADER __FILE__ << ":" << __LINE__ << ": "

//-----------------------------------------------------------------------------
FilterCombLowPassFeedback::
FilterCombLowPassFeedback(
    const float64 & sample_rate,
    const float64 & delay_time_seconds,
    const float64 & feedback_gain,
    const float64 & low_pass_frequency_Hz)
    :
    Filter(sample_rate),
    delay_(NULL),
    feedback_gain_(feedback_gain),
    damp1_(0.0),
    damp2_(0.0),
    y_history_(0.0)
{
    delay_ = new FilterDelay(sample_rate_, delay_time_seconds);

    FilterCombLowPassFeedback::reset();

    if(feedback_gain_ < 0.0)
    {
        feedback_gain_ = 0.0;
    }

    if(feedback_gain_ >= 1.0)
    {
        feedback_gain_ = 0.999999;
    }

    damp1_ = low_pass_frequency_Hz / sample_rate_;

    if(damp1_ > 0.5)
    {
        damp1_ = 0.5;
    }

    damp2_ = 1.0 - damp1_;
}

//-----------------------------------------------------------------------------
FilterCombLowPassFeedback::
FilterCombLowPassFeedback(const FilterCombLowPassFeedback & copy)
    :
    Filter(copy.sample_rate_),
    delay_(new FilterDelay(*copy.delay_)),
    feedback_gain_(copy.feedback_gain_),
    damp1_(copy.damp1_),
    damp2_(copy.damp2_),
    y_history_(copy.y_history_)
{
}


//-----------------------------------------------------------------------------
FilterCombLowPassFeedback::
~FilterCombLowPassFeedback()
{
    delete delay_;
}

AudioStream
FilterCombLowPassFeedback::
filter(const AudioStream & x)
{
    FilterCombLowPassFeedback::reset();
    return Filter::filter(x);
}

AudioStream
FilterCombLowPassFeedback::
filter(const AudioStream & x, const Buffer & frequencies)
{
    FilterCombLowPassFeedback::reset();
    return Filter::filter(x, frequencies);
}

Buffer
FilterCombLowPassFeedback::
filter(const Buffer & x)
{
    FilterCombLowPassFeedback::reset();
    return Filter::filter(x);
}

Buffer
FilterCombLowPassFeedback::
filter(const Buffer & x, const Buffer & frequencies)
{
    FilterCombLowPassFeedback::reset();
    return Filter::filter(x, frequencies);
}

float64
FilterCombLowPassFeedback::
filter(const float64 & x)
{
    float64 d = delay_->filter(x + y_history_ * feedback_gain_);

    y_history_ = d * damp2_ + y_history_ * damp1_;

    return d;
}

float64
FilterCombLowPassFeedback::
filter(const float64 & x, const float64 & frequency_Hz)
{
    damp1_ = frequency_Hz / sample_rate_;

    if(damp1_ > 0.5)
    {
        damp1_ = 0.5;
    }

    damp2_ = 1.0 - damp1_;

    return FilterCombLowPassFeedback::filter(x);
}

//-----------------------------------------------------------------------------
FilterCombLowPassFeedback &
FilterCombLowPassFeedback::
operator=(const FilterCombLowPassFeedback & rhs)
{
    *delay_ = *rhs.delay_;
    feedback_gain_ = rhs.feedback_gain_;
    damp1_ = rhs.damp1_;
    damp2_ = rhs.damp2_;
    y_history_ = rhs.y_history_;

    return *this;
}

void
FilterCombLowPassFeedback::
plot(boolean show_fc, boolean show_phase)
{
    char title[128];
    sprintf(title,
        "Comb Low Pass Feedback Frequency Response\n"
        "order = %d, fc = %0.1f Hz, sr = %0.1f Hz",
        2,
        damp1_ * sample_rate_,
        sample_rate_);

    Filter::plot(show_phase);

    Plotter pylab;

    uint32 n_rows = 1;

    if(show_phase)
    {
        n_rows = 2;
    }

    if(show_fc)
    {
        pylab.subplot(n_rows, 1, 1);

        pylab.axvline(damp1_ * sample_rate_,"color='red'");

        pylab.title(title);
    }
}

void
FilterCombLowPassFeedback::
reset()
{
    delay_->reset();
    y_history_ = 0.0;
}
