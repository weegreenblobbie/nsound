//-----------------------------------------------------------------------------
//
//  $Id: FilterDelay.cc 874 2014-09-08 02:21:29Z weegreenblobbie $
//
//  Copyright (c) 2007 Nick Hilton
//
//  weegreenblobbie2_gmail_com (replace '_' with '@' and '.')
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

#include <Nsound/AudioStream.h>
#include <Nsound/Buffer.h>
#include <Nsound/FilterDelay.h>
#include <Nsound/Plotter.h>

#include <algorithm>
#include <cmath>
#include <iostream>

using namespace Nsound;

using std::cerr;
using std::endl;

//-----------------------------------------------------------------------------
FilterDelay::
FilterDelay(
    const float64 & sample_rate,
    const float64 & max_delay_time_seconds)
    :
    Filter(sample_rate),
    delay_(max_delay_time_seconds)
{
    M_ASSERT_VALUE(delay_, >, 0.0);

    // The longest delay's samples and one more for the newest sample.
    buffer_.resize(static_cast<uint32>(std::ceil(sample_rate_ * delay_)) + 1);

    FilterDelay::reset();
}

AudioStream
FilterDelay::
filter(const AudioStream & x)
{
    return Filter::filter(x);
}

AudioStream
FilterDelay::
filter(const AudioStream & x, const float64 & delay)
{
    return Filter::filter(x, delay);
}

AudioStream
FilterDelay::
filter(const AudioStream & x, const Buffer & delay)
{
    return Filter::filter(x, delay);
}

Buffer
FilterDelay::
filter(const Buffer & x)
{
    return Filter::filter(x);
}

Buffer
FilterDelay::
filter(const Buffer & x, const float64 & delay)
{
    return Filter::filter(x, delay);
}

Buffer
FilterDelay::
filter(const Buffer & x, const Buffer & delay)
{
    return Filter::filter(x, delay);
}

float64
FilterDelay::
filter(const float64 & x)
{
    return FilterDelay::filter(x, delay_);

}

float64
FilterDelay::
filter(const float64 & x, const float64 & delay)
{
    const uint32 n_samples = static_cast<uint32>(buffer_.size());

    buffer_[write_index_] = x;

    // The newest sample is at write_index_, then it moves on.
    uint32 newest = write_index_;

    write_index_ = (write_index_ + 1) % n_samples;

    float64 del = delay;

    // limit delay
    if(del > delay_)   del = delay_;
    else if(del < 0.0) del = 0.0;

    uint32 d = static_cast<uint32>(sample_rate_ * del);

    // d samples before the newest, round the circle.
    return buffer_[(newest + n_samples - d) % n_samples];
}

void
FilterDelay::
reset()
{
    std::fill(buffer_.begin(), buffer_.end(), 0.0);

    write_index_ = static_cast<uint32>(buffer_.size()) - 1;
}


