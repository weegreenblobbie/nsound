//-----------------------------------------------------------------------------
//
//  $Id: GeneratorDecay.cc 874 2014-09-08 02:21:29Z weegreenblobbie $
//
//  Copyright (c) 2005-2006 Nick Hilton
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

#include <Nsound/Buffer.h>
#include <Nsound/GeneratorDecay.h>

using namespace Nsound;

// Constructor
GeneratorDecay::
GeneratorDecay() : Generator() {};

// Constructor
GeneratorDecay::
GeneratorDecay(const float64 & sample_rate) : Generator(sample_rate) {};

float64
GeneratorDecay::
generate(const float64 & frequency)
{
    return generate2(frequency, 1.0);
}

float64
GeneratorDecay::
generate2(const float64 & frequency, const float64 & alpha)
{
    ++sync_count_;

    if(sync_is_slave_ && !sync_vector_.empty())
    {
        uint32 sync_count = sync_vector_.front();

        if(sync_count_ == sync_count)
        {
            sync_vector_.erase(sync_vector_.begin());
            position_ = 0;
        }
    }

    float64 t = sync_pos_ / sample_rate_;

    float64 sample = ::exp(t * -alpha);

    position_ += frequency;
    sync_pos_ += frequency;

    // limit
    if(position_ > sample_rate_)
    {
        position_ -= sample_rate_;
        sync_pos_ = 0;

        if(sync_is_master_)
        {
            std::set<Generator *>::iterator itor = this->sync_slaves_.begin();

            while(itor != sync_slaves_.end())
            {
                GeneratorDecay * ptr = static_cast<GeneratorDecay *>(*itor);

                ptr->sync_vector_.push_back(sync_count_);
                ++itor;
            }
        }
    }

    return sample;
}

Buffer
GeneratorDecay::
generate2(
    const float64 & duration,
    const float64 & frequency,
    const float64 & alpha)
{
    return Generator::generate2(duration, frequency, alpha);
}

Buffer
GeneratorDecay::
generate2(
    const float64 & duration,
    const float64 & frequency,
    const Buffer &  alpha)
{
    return Generator::generate2(duration, frequency, alpha);
}

Buffer
GeneratorDecay::
generate2(
    const float64 & duration,
    const Buffer &  frequencies,
    const float64 & alpha)
{
    return Generator::generate2(duration, frequencies, alpha);
}

Buffer
GeneratorDecay::
generate2(
    const float64 & duration,
    const Buffer &  frequencies,
    const Buffer &  alpha)
{
    return Generator::generate2(duration, frequencies, alpha);
}

