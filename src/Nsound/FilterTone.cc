//-----------------------------------------------------------------------------
//
//  $Id: FilterTone.cc 874 2014-09-08 02:21:29Z weegreenblobbie $
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
#include <Nsound/FilterTone.h>
#include <Nsound/Plotter.h>

#include <cmath>
#include <cstdio>
#include <iostream>

using namespace Nsound;

//-----------------------------------------------------------------------------
FilterTone::
FilterTone(const float64 & sample_rate, const float64 & half_power_frequency)
    :
    Filter(sample_rate),
    hp_frequency_(half_power_frequency),
    b_(0.0),
    a_(0.0),
    last_output_(0.0),
    kernel_cache_()
{
    reset();
}

//-----------------------------------------------------------------------------
FilterTone::
~FilterTone()
{

}

AudioStream
FilterTone::
filter(const AudioStream & x)
{
    return Filter::filter(x);
}

AudioStream
FilterTone::
filter(const AudioStream & x, const Buffer & hp_frequencies)
{
    return Filter::filter(x, hp_frequencies);
}

Buffer
FilterTone::
filter(const Buffer & x)
{
    return Filter::filter(x);
}

Buffer
FilterTone::
filter(const Buffer & x, const Buffer & hp_frequencies)
{
    return Filter::filter(x, hp_frequencies);
}

float64
FilterTone::
filter(const float64 & x)
{
    float64 y = b_ * x - a_ * last_output_;

    last_output_ = y;

    return y;
}

float64
FilterTone::
filter(const float64 & x, const float64 & hp_frequency)
{
    makeKernel(hp_frequency);

    return FilterTone::filter(x);
}

void
FilterTone::
makeKernel(const float64 & half_power_frequency)
{
    FilterTone::
    Kernel new_kernel(static_cast<uint32>(half_power_frequency * 10));

    // See if the kernel is in the cache.
    KernelCache::const_iterator itor = kernel_cache_.find(new_kernel);

    if(itor != kernel_cache_.end())
    {
        // The kernel was found in the cache.
        b_ = itor->b_;
        a_ = itor->a_;
        return;
    }

    // The filter wasn't in the cache, need to make it.

    float64 temp = 2.0 - ::cos(two_pi_over_sample_rate_ * half_power_frequency);

    a_ = -1.0 * (temp - ::sqrt(temp * temp - 1.0));
    b_ = 1.0 + a_;

    new_kernel.b_ = b_;
    new_kernel.a_ = a_;

    kernel_cache_.insert(new_kernel);
}

void
FilterTone::
plot(boolean show_fc, boolean show_phase)
{
    char title[128];
    sprintf(title,
        "Tone Filter Frequency Response\n"
        "order = %d, fc = %0.1f Hz, sr = %0.1f Hz",
        2, hp_frequency_, sample_rate_);

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

        pylab.axvline(hp_frequency_,"color='red'");

        pylab.title(title);
    }

}

void
FilterTone::
reset()
{
    last_output_ = 0.0;
    makeKernel(hp_frequency_);
}

//-----------------------------------------------------------------------------
FilterTone::
Kernel::
Kernel(const uint32 & frequency)
    :
    b_(0.0),
    a_(0.0),
    frequency_(frequency)
{
}

bool
FilterTone::
Kernel::
operator<(const Kernel & rhs) const
{
    return frequency_ <  rhs.frequency_;
}

