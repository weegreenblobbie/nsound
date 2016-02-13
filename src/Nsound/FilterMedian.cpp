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
#include <Nsound/FilterMedian.hpp>


namespace Nsound
{


uint32 keep_odd(uint32 n)
{
    if(n % 2 == 0) return n + 1;

    return n;
}


FilterMedian::
FilterMedian(uint32 n_samples_in_pool)
    :
    _h_ptr(0),
    _median(n_samples_in_pool / 2),
    _history(keep_odd(n_samples_in_pool), 0.0),
    _pool(keep_odd(n_samples_in_pool), 0.0)
{
    M_ASSERT_MSG(n_samples_in_pool >= 3, "n_samples_in_pool < 3! (" << n_samples_in_pool << ")");
}


AudioStream
FilterMedian::
filter(const AudioStream & in)
{
    AudioStream out(in.getSampleRate(), in.getNChannels());

    uint32 channel = 0;

    for(auto bptr : in) out[channel++] = filter(*bptr);

    return out;
}


Buffer
FilterMedian::
filter(const Buffer & in)
{
    M_ASSERT_MSG(in.getLength() > 0, "Oops, Buffer is empty!");

    fill(in[0]); // offline rendering only!

    Buffer out(in.getLength());

    for(auto x : in) out << filter(x);

    return out;
}


void
FilterMedian::
fill(float64 x)
{
    _h_ptr = 0;
    std::fill(_history.begin(), _history.end(), x);
    std::fill(_pool.begin(), _pool.end(), x);
}


float64
FilterMedian::
filter(const float64 & in)
{
    // _pool is always sorted

    // step 1, remove oldest vaule from the pool.

    auto last = _history[_h_ptr];

    auto last_pos = std::lower_bound(_pool.begin(), _pool.end(), last);

    _pool.erase(last_pos);

    // step 2, insert new value into pool

    auto insert_pos = std::lower_bound(_pool.begin(), _pool.end(), in);

    _pool.insert(insert_pos, in);

    // step 3, write input value into history.

    _history[_h_ptr] = in;

    _h_ptr = (_h_ptr + 1) % _history.size();

    // median is always the middle of the pool.

    return _pool[_median];
}


} // namespace