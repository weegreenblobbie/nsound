//-----------------------------------------------------------------------------
//
//  $Id: FilterDelay.cc 874 2014-09-08 02:21:29Z weegreenblobbie $
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

#include <Nsound/AudioStream.h>
#include <Nsound/Buffer.h>
#include <Nsound/FilterDelay.h>
#include <Nsound/Plotter.h>

#include <cmath>
#include <iostream>
#include <string.h>

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
    buffer_(NULL),
    buffer_end_ptr_(NULL),
    read_ptr_(NULL),
    write_ptr_(NULL),
    delay_(max_delay_time_seconds),
    n_samples_(0)
{
    M_ASSERT_VALUE(delay_, >, 0.0);

    n_samples_ = static_cast<uint32>(std::ceil(sample_rate_ * delay_));

    ++n_samples_;

    buffer_ = new float64 [n_samples_];

    buffer_end_ptr_ = buffer_ + n_samples_;

    FilterDelay::reset();
}

//-----------------------------------------------------------------------------
FilterDelay::
FilterDelay(const FilterDelay & copy)
    :
    Filter(copy.sample_rate_),
    buffer_(NULL),
    buffer_end_ptr_(NULL),
    read_ptr_(NULL),
    write_ptr_(NULL),
    delay_(copy.delay_),
    n_samples_(copy.n_samples_)
{
    buffer_ = new float64 [n_samples_];

    buffer_end_ptr_ = buffer_ + n_samples_;

    *this = copy;
}

//-----------------------------------------------------------------------------
FilterDelay::
~FilterDelay()
{
    delete [] buffer_;
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
    *write_ptr_ = x;
    ++write_ptr_;

    // Bounds check
    if(write_ptr_ >= buffer_end_ptr_)
    {
        write_ptr_ = buffer_;
    }

    float64 del = delay;

    // limit delay
    if(del > delay_)   del = delay_;
    else if(del < 0.0) del = 0.0;

    uint32 d = static_cast<uint32>(sample_rate_ * del);

    read_ptr_ = write_ptr_ - d - 1;

    // Bounds check.
    if(read_ptr_ < buffer_)
    {
        read_ptr_ += n_samples_;
    }

    float64 y = *read_ptr_;

    return y;
}


///////////////////////////////////////////////////////////////////////////
FilterDelay &
FilterDelay::
operator=(const FilterDelay & rhs)
{
    if(this == &rhs)
    {
        return *this;
    }

    if(n_samples_ != rhs.n_samples_)
    {
        n_samples_ = rhs.n_samples_;
        delete [] buffer_;
        buffer_ = new float64 [n_samples_];
        buffer_end_ptr_ = buffer_ + n_samples_;
        reset();
    }

    delay_     = rhs.delay_;
    n_samples_ = rhs.n_samples_;

    // Copy the buffer.
    memcpy(buffer_, rhs.buffer_, sizeof(float64) * n_samples_);

    // Set the pointers to the same offsets into the buffer.
    read_ptr_  = buffer_ + (rhs.read_ptr_ - rhs.buffer_);
    write_ptr_ = buffer_ + (rhs.write_ptr_ - rhs.buffer_);

    return *this;
}

void
FilterDelay::
reset()
{
    // Clear all the memory in the buffer.
    memset(buffer_, 0, sizeof(float64) * n_samples_);

    read_ptr_ = buffer_;
    write_ptr_ = buffer_end_ptr_ - 1;
}


