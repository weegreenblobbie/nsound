//-----------------------------------------------------------------------------
//
//  $Id: DelayLine.cc 908 2015-07-08 02:04:41Z weegreenblobbie $
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

#include <algorithm>

#include <Nsound/AudioStream.h>
#include <Nsound/Buffer.h>
#include <Nsound/DelayLine.h>


namespace Nsound
{


DelayLine::
DelayLine(float64 sample_rate, float64 max_delay_in_seconds)
    :
    sample_rate_(sample_rate),
    max_delay_time_(max_delay_in_seconds),
    delay_time_(max_delay_in_seconds),
    buffer_(
        static_cast<uint64>(sample_rate * max_delay_in_seconds + 1),
        0.0),
    wr_idx_(0)
{
    M_ASSERT_VALUE(sample_rate, >, 0.0);
    M_ASSERT_VALUE(max_delay_in_seconds, >, 0.0);
}


Buffer
DelayLine::
delay(const Buffer & x, const Buffer & delay_time)
{
    if(!is_realtime_) reset();

    Buffer y;

    auto dt = delay_time.cbegin();

    for(auto sample : x)
    {
        y << delay(sample, *dt);
        ++dt;
    }

    return y;
}


float64
DelayLine::
delay(float64 x, float64 delay_time)
{
    write(x);
    return read(delay_time);
}


void
DelayLine::
reset()
{
    wr_idx_ = 0;
    delay_time_ = max_delay_time_;
    std::fill(buffer_.begin(), buffer_.end(), 0.0);
}


float64
DelayLine::
read()
{
    M_ASSERT_MSG(
        delay_time_ <= max_delay_time_,
        "delay time exceeds maximum ("
        << delay_time_ << " > " << max_delay_time_ << ")");

    uint32 offset = static_cast<uint32>(sample_rate_ * delay_time_ + 0.5);

    if(offset > wr_idx_)
    {
        return buffer_[buffer_.size() - (offset - wr_idx_)];
    }

    return buffer_[wr_idx_ - offset];
}


float64
DelayLine::
read(float64 delay)
{
    M_ASSERT_MSG(delay > 0.0, "delay must be > 0.0, got " << delay);

    delay_time_ = delay;

    // limit

    if(delay_time_ > max_delay_time_) delay_time_ = max_delay_time_;

    return read();
}


void
DelayLine::
write(float64 x)
{
    buffer_[wr_idx_++] = x;

    if(wr_idx_ >= buffer_.size())
    {
        wr_idx_ = 0;
    }
}


} // namespace
