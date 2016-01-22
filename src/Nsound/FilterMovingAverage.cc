//-----------------------------------------------------------------------------
//
//  $Id: FilterMovingAverage.cc 874 2014-09-08 02:21:29Z weegreenblobbie $
//
//  Copyright (c) 2006 to Present Nick Hilton
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
#include <Nsound/FilterDelay.h>
#include <Nsound/FilterMovingAverage.h>

using namespace Nsound;

//-----------------------------------------------------------------------------
FilterMovingAverage::
FilterMovingAverage(uint32 n_samples_to_average)
    :
    Filter(1.0),
    init_sum_(true),
    n_samples_to_average_(n_samples_to_average),
    running_sum_(0.0),
    delay_(NULL)
{
    delay_ = new FilterDelay(1.0, n_samples_to_average_);

    reset();
}

//-----------------------------------------------------------------------------
FilterMovingAverage::
FilterMovingAverage(const FilterMovingAverage & copy)
    :
    Filter(1.0),
    init_sum_(copy.init_sum_),
    n_samples_to_average_(copy.n_samples_to_average_),
    running_sum_(copy.running_sum_),
    delay_(new FilterDelay(*copy.delay_))
{
}

//-----------------------------------------------------------------------------
FilterMovingAverage::
~FilterMovingAverage()
{
    delete delay_;
}

AudioStream
FilterMovingAverage::
filter(const AudioStream & x)
{
    FilterMovingAverage::reset();
    return Filter::filter(x);
}

Buffer
FilterMovingAverage::
filter(const Buffer & x)
{
    FilterMovingAverage::reset();
    return Filter::filter(x);
}

float64
FilterMovingAverage::
filter(const float64 & x)
{
    if(init_sum_)
    {
        init_sum_ = false;

        for(uint32 i = 0; i < static_cast<uint32>(n_samples_to_average_); ++i)
        {
            delay_->filter(x);
        }

        running_sum_ += (n_samples_to_average_ - 1.0) * x;
    }

    float64 last_nth_x = delay_->filter(x);

    running_sum_ += x - last_nth_x;

    return running_sum_ / n_samples_to_average_;
}

//-----------------------------------------------------------------------------
FilterMovingAverage &
FilterMovingAverage::
operator=(const FilterMovingAverage & rhs)
{
    if(this == & rhs)
    {
        return *this;
    }
    init_sum_ = rhs.init_sum_;
    n_samples_to_average_ = rhs.n_samples_to_average_;
    running_sum_ = rhs.running_sum_;

    *delay_ = *rhs.delay_;

    return *this;
}

void
FilterMovingAverage::
reset()
{
    init_sum_ = true;
    running_sum_ = 0;
    delay_->reset();
}
