//-----------------------------------------------------------------------------
//
//  $Id: FilterHighPassFIR.cc 874 2014-09-08 02:21:29Z weegreenblobbie $
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
//-----------------------------------------------------------------------------

#include <Nsound/AudioStream.h>
#include <Nsound/Buffer.h>
#include <Nsound/FilterHighPassFIR.h>
#include <Nsound/Plotter.h>

#include <cmath>
#include <cstdio>
#include <cstring>
#include <iostream>

using namespace Nsound;

using std::cerr;
using std::endl;

//-----------------------------------------------------------------------------
FilterHighPassFIR::
FilterHighPassFIR(
    const float64 & sample_rate,
    uint32 kernel_size,
    const float64 & cutoff_frequency_Hz)
    :
    FilterLowPassFIR(sample_rate, kernel_size, cutoff_frequency_Hz),
    hp_cache_()
{
    frequency_1_Hz_ = cutoff_frequency_Hz;
    FilterHighPassFIR::reset();
}

//-----------------------------------------------------------------------------
FilterHighPassFIR::
~FilterHighPassFIR()
{
    KernelCache::iterator itor = hp_cache_.begin();
    KernelCache::iterator  end = hp_cache_.end();

    while(itor != end)
    {
        delete [] itor->b_;
        ++itor;
    }
}

AudioStream
FilterHighPassFIR::
filter(const AudioStream & x)
{
    return Filter::filter(x);
}

AudioStream
FilterHighPassFIR::
filter(const AudioStream & x, const float64 & f)
{
    return Filter::filter(x, f);
}

AudioStream
FilterHighPassFIR::
filter(const AudioStream & x, const Buffer & frequencies)
{
    return Filter::filter(x, frequencies);
}

Buffer
FilterHighPassFIR::
filter(const Buffer & x)
{
    return Filter::filter(x);
}

Buffer
FilterHighPassFIR::
filter(const Buffer & x, const float64 & f)
{
    // Instead of calling Filter::filter(x,f), we implement this function here
    // to avoid calling makeKernel(f) for each sample.

    FilterHighPassFIR::reset();
    FilterHighPassFIR::makeKernel(f);

    Buffer::const_iterator itor = x.begin();
    Buffer::const_iterator end = x.end();

    Buffer y;
    while(itor != end)
    {
        y << FilterLowPassFIR::filter(*itor);
        ++itor;
    }

    return y;
}

Buffer
FilterHighPassFIR::
filter(const Buffer & x, const Buffer & frequencies)
{
    return Filter::filter(x, frequencies);
}

float64
FilterHighPassFIR::
filter(const float64 & x)
{
    return FilterLowPassFIR::filter(x);
}

float64
FilterHighPassFIR::
filter(const float64 & x, const float64 & frequency_Hz)
{
    FilterHighPassFIR::makeKernel(frequency_Hz);
    return FilterLowPassFIR::filter(x);
}

Buffer
FilterHighPassFIR::
getImpulseResponse()
{
    return Filter::getImpulseResponse();
}

void
FilterHighPassFIR::
makeKernel(const float64 & cutoff_frequency_Hz)
{
    // Here we create a key into the kernel cache.  I'm only storing kernels
    // with freqs chopped off to the 10th Hz.  So filtering with a kernel
    // designed at 440.1567 Hz will get stored as 440.1.  Any other frequency
    // that starts at 440.1 will not get a kernel calculated and just use the
    // one in the cache.

    FilterLowPassFIR::Kernel new_kernel(
        static_cast<uint32>(cutoff_frequency_Hz));

    // See if the kernel is in the cache.

    FilterLowPassFIR::
    KernelCache::
    const_iterator itor = hp_cache_.find(new_kernel);

    if(itor != hp_cache_.end())
    {
        // The kernel was found in the cache.
        b_ = itor->b_;
        return;
    }

    // The filter wasn't in the cache, need to make it.
    new_kernel.b_ = new float64[kernel_size_];

    if(cutoff_frequency_Hz < 0.10)
    {
        // Create All pass filter.
        new_kernel.b_[0] = 1.0;

        b_ = new_kernel.b_;
    }
    else
    {
        float64 fc = sample_rate_ / 2.0 - cutoff_frequency_Hz;

        FilterLowPassFIR::makeKernel(fc);

        // Now b_ is pointing to a low pass kernel, copy it into new_kernel.
        memcpy(new_kernel.b_, b_, sizeof(float64) * kernel_size_);

        // Point to new kernel.
        b_ = new_kernel.b_;

        // Perform the spectra inversion on the kernel to turn it into a high
        // pass filter.
        spectraReversal_();
    }

    // Add the new kernel to the cache.
    hp_cache_.insert(new_kernel);
}

void
FilterHighPassFIR::
plot(
    boolean show_fc,
    boolean show_phase)
{
    char title[128];
    sprintf(title,
        "High Pass FIR Frequency Response\n"
        "order = %d, fc = %0.1f Hz, sr = %0.1f Hz",
        kernel_size_ - 1, frequency_1_Hz_, sample_rate_);

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

        pylab.axvline(frequency_1_Hz_,"color='red'");

        pylab.title(title);
    }

}

void
FilterHighPassFIR::
reset()
{
    // Don't call FilterLowPassFIR::reset(), if we did, we would be creating
    // a kernel twice.

    memset(x_history_, 0, sizeof(float64) * (kernel_size_ + 1));
    x_ptr_ = x_history_;

    FilterHighPassFIR::makeKernel(frequency_1_Hz_);
}

void
FilterHighPassFIR::
spectraReversal_()
{
    for(uint32 i = 0; i < kernel_size_; ++i)
    {
        if(i % 2)
        {
            b_[i] *= -1.00;
        }
    }
}

