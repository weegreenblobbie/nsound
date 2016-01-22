//-----------------------------------------------------------------------------
//
//  $Id: FilterAllPass.cc 874 2014-09-08 02:21:29Z weegreenblobbie $
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
#include <Nsound/FilterAllPass.h>
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
FilterAllPass::
FilterAllPass(
    const float64 & sample_rate,
    const float64 & max_delay_time_seconds,
    const float64 & gain)
    :
    Filter(sample_rate),
    x_delay_(NULL),
    y_delay_(NULL),
    gain_(gain),
    y_history_(0.0)
{
    x_delay_ = new FilterDelay(sample_rate_, max_delay_time_seconds);
    y_delay_ = new FilterDelay(sample_rate_, max_delay_time_seconds);

    FilterAllPass::reset();
}

//-----------------------------------------------------------------------------
FilterAllPass::
FilterAllPass(const FilterAllPass & copy)
    :
    Filter(copy.sample_rate_),
    x_delay_(NULL),
    y_delay_(NULL),
    gain_(copy.gain_),
    y_history_(0.0)
{
    x_delay_ = new FilterDelay(*copy.x_delay_);
    y_delay_ = new FilterDelay(*copy.y_delay_);

    *this = copy;
}

//-----------------------------------------------------------------------------
FilterAllPass::
~FilterAllPass()
{
    delete x_delay_;
    delete y_delay_;
}

AudioStream
FilterAllPass::
filter(const AudioStream & x)
{
    return Filter::filter(x);
}

AudioStream
FilterAllPass::
filter(const AudioStream & x, const float64 & delay_in_seconds)
{
    return Filter::filter(x, delay_in_seconds);
}

AudioStream
FilterAllPass::
filter(const AudioStream & x, const Buffer & delay_in_seconds)
{
    return Filter::filter(x, delay_in_seconds);
}

Buffer
FilterAllPass::
filter(const Buffer & x)
{
    return Filter::filter(x);
}

Buffer
FilterAllPass::
filter(const Buffer & x, const float64 & delay_in_seconds)
{
    return Filter::filter(x, delay_in_seconds);
}

Buffer
FilterAllPass::
filter(const Buffer & x, const Buffer & delay_in_seconds)
{
    return Filter::filter(x, delay_in_seconds);
}

float64
FilterAllPass::
filter(const float64 & x)
{
    float64 y = gain_ * x + x_delay_->filter(x)
              - gain_ * y_delay_->filter(y_history_);

    y_history_ = y;

    return y;
}

float64
FilterAllPass::
filter(const float64 & x, const float64 & delay)
{
    float64 y = gain_ * x + x_delay_->filter(x, delay)
              - gain_ * y_delay_->filter(y_history_, delay);

    y_history_ = y;

    return y;
}

//-----------------------------------------------------------------------------
FilterAllPass &
FilterAllPass::
operator=(const FilterAllPass & rhs)
{
    if(this == &rhs)
    {
        return *this;
    }

    *x_delay_ = *rhs.x_delay_;
    *y_delay_ = *rhs.y_delay_;

    gain_ = rhs.gain_;
    y_history_ = rhs.y_history_;

    return *this;
}

void
FilterAllPass::
plot(boolean show_fc, boolean show_phase)
{
    char title[128];
    sprintf(title,
        "All Pass Frequency Response\n"
        "sr = %0.1f Hz",
        sample_rate_);

    Filter::plot(show_phase);

    Plotter pylab;

    uint32 n_rows = 1;
    uint32 n_cols = 1;

    if(show_phase)
    {
        n_rows = 2;
    }

    if(show_fc)
    {
        pylab.subplot(n_rows, n_cols, 1);

        pylab.title(title);
    }
}

void
FilterAllPass::
reset()
{
    x_delay_->reset();
    y_delay_->reset();
    y_history_ = 0.0;
}
