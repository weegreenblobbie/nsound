//-----------------------------------------------------------------------------////
//
//  $Id: FilterBandRejectFIR.cc 874 2014-09-08 02:21:29Z weegreenblobbie $
//
//  Copyright (c) 2006 Nick Hilton
//
//  weegreenblobbie_yahoo_com (replace '_' with '@' and '.')
//
//-----------------------------------------------------------------------------////

//-----------------------------------------------------------------------------////
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
#include <Nsound/FilterBandRejectFIR.h>
#include <Nsound/Plotter.h>

#include <cmath>
#include <cstdio>
#include <cstring>
#include <iostream>

using namespace Nsound;

using std::cerr;
using std::endl;

#define CERR_HEADER __FILE__ << ":" << __LINE__ << ": "

FilterBandRejectFIR::
FilterBandRejectFIR(
    const float64 & sample_rate,
    uint32 kernel_size,
    const float64 & frequency_Hz_low,
    const float64 & frequency_Hz_high)
    :
    FilterHighPassFIR(sample_rate, kernel_size, frequency_Hz_high),
    frequency_2_Hz_(frequency_Hz_high),
    kernel_cache_()
{
    frequency_1_Hz_ = frequency_Hz_low;

    reset();
}

FilterBandRejectFIR::
~FilterBandRejectFIR()
{
    FilterBandRejectFIR::KernelCache::iterator itor =
        FilterBandRejectFIR::kernel_cache_.begin();

    FilterBandRejectFIR::KernelCache::iterator  end =
        FilterBandRejectFIR::kernel_cache_.end();

    while(itor != end)
    {
        delete [] itor->b_;
        ++itor;
    }
}

float64
FilterBandRejectFIR::
getFrequencyLow() const
{
    return frequency_1_Hz_;
}

float64
FilterBandRejectFIR::
getFrequencyHigh() const
{
    return frequency_2_Hz_;
}


AudioStream
FilterBandRejectFIR::
filter(const AudioStream & x)
{
    return Filter::filter(x);
}

AudioStream
FilterBandRejectFIR::
filter(
    const AudioStream & x,
    const float64 & f_low,
    const float64 & f_high)
{
    uint32 n_channels = x.getNChannels();

    AudioStream y(x.getSampleRate(), n_channels);

    for(uint32 channel = 0; channel < n_channels; ++channel)
    {
        y[channel] = FilterBandRejectFIR::filter(x[channel], f_low, f_high);
    }

    return y;
}

AudioStream
FilterBandRejectFIR::
filter(
    const AudioStream & x,
    const Buffer & frequencies_Hz_low,
    const Buffer & frequencies_Hz_high)
{
    if(!is_realtime_) reset();

    uint32 n_channels = x.getNChannels();

    AudioStream y(x.getSampleRate(), n_channels);

    for(uint32 channel = 0; channel < n_channels; ++channel)
    {
        y[channel] = FilterBandRejectFIR::filter(
            x[channel],
            frequencies_Hz_low,
            frequencies_Hz_high);
    }

    return y;
}

Buffer
FilterBandRejectFIR::
filter(const Buffer & x)
{
    return FilterLowPassFIR::filter(x);
}

Buffer
FilterBandRejectFIR::
filter(
    const Buffer & x,
    const float64 & f_low,
    const float64 & f_high)
{
    if(!is_realtime_) reset();

    // Instead of calling filter(x,f_low, f_high), we implement this function
    // here to avoid calling makeKernel(f) for each sample.

    FilterBandRejectFIR::makeKernel(f_low, f_high);

    Buffer::const_iterator itor = x.begin();
    Buffer::const_iterator end = x.end();

    Buffer y(x.getLength());
    while(itor != end)
    {
        y << FilterLowPassFIR::filter(*itor);
        ++itor;
    }

    return y;
}

Buffer
FilterBandRejectFIR::
filter(
    const Buffer & x,
    const Buffer & frequencies_Hz_low,
    const Buffer & frequencies_Hz_high)
{
    if(!is_realtime_) reset();

    uint32 n_high_freqs = frequencies_Hz_high.getLength();
    uint32 n_low_freqs  = frequencies_Hz_low.getLength();

    Buffer y;

    uint32 x_samples = x.getLength();

    for(uint32 i = 0; i < x_samples; ++i)
    {
        y << FilterBandRejectFIR::filter(
            x[i],
            frequencies_Hz_low [i % n_low_freqs],
            frequencies_Hz_high[i % n_high_freqs]);
    }

    return y;
}

float64
FilterBandRejectFIR::
filter(const float64 & x)
{
    return FilterLowPassFIR::filter(x);
}

float64
FilterBandRejectFIR::
filter(const float64 & x, const float64 & frequency_Hz)
{
    return FilterLowPassFIR::filter(x);
}

float64
FilterBandRejectFIR::
filter(
    const float64 & x,
    const float64 & frequencies_Hz_low,
    const float64 & frequencies_Hz_high)
{
    makeKernel(frequencies_Hz_low, frequencies_Hz_high);
    return FilterLowPassFIR::filter(x);
}

void
FilterBandRejectFIR::
makeKernel(const float64 & low_frequency, const float64 & high_frequency)
{
    // Here we create a key into the kernel cache.  I'm only storing kernels
    // with freqs chopped off to the 10th Hz.  So filtering with a kernel
    // designed at 440.1567 Hz will get stored as 440.1.  Any other frequency
    // that starts at 440.1 will not get a kernel calculated and just use the
    // one in the cache.  So the max diff between what's passed in and what's
    // stored in the cache is 0.09999 Hz.

    FilterBandRejectFIR::Kernel new_kernel(
        static_cast<uint32>(low_frequency),
        static_cast<uint32>(high_frequency));

    // See if the kernel is in the cache.

    FilterBandRejectFIR::KernelCache::const_iterator itor =
        FilterBandRejectFIR::kernel_cache_.find(new_kernel);

    if(itor != FilterBandRejectFIR::kernel_cache_.end())
    {
        // The kernel was found in the cache.
        b_ = itor->b_;
        return;
    }

    // The filter wasn't in the cache, need to make it.
    new_kernel.b_ = new float64[kernel_size_];

    if(low_frequency < 0.10 && high_frequency >= 0.10)
    {
        // Create simple high pass filter.
        FilterHighPassFIR::makeKernel(high_frequency);

//~        // b_ is now pointing at a high pass kernel held in the high pass cache
//~        memcpy(new_kernel.b_, b_, kernel_size_ * sizeof(float64));

        return;

    }
    else if(low_frequency >= 0.10 && high_frequency < 0.10)
    {
        // Create simple low pass filter.
        FilterLowPassFIR::makeKernel(low_frequency);

//~        // b_ is now pointing at a high pass kernel held in the low pass cache
//~        memcpy(new_kernel.b_, b_, kernel_size_ * sizeof(float64));

        return;

    }
    else if(low_frequency <= 0.10 && high_frequency < 0.10)
    {
        // Create simple All pass filter
        new_kernel.b_[0] = 1.0;

        for(uint32 i = 1; i < kernel_size_; ++i)
        {
            new_kernel.b_[i] = 0.00000;
        }
    }
    else  // non-zero frequencies
    {
        FilterLowPassFIR::makeKernel(low_frequency);

        // Now b_ is pointing to a low pass kernel held in the low pass cache.
        memcpy(new_kernel.b_, b_, kernel_size_ * sizeof(float64));

        FilterHighPassFIR::makeKernel(high_frequency);

        // b_ is now pointing at a high pass kernel held in the high pass cache

        // To create the band reject, simply add the low pass and high pass
        // kernels.

        for(uint32 i = 0; i < kernel_size_; ++i)
        {
            // low kernel += high kernel
            new_kernel.b_[i] += b_[i];
        }
    }

    // point at the new kernel
    b_ = new_kernel.b_;

    // insert into cache
    FilterBandRejectFIR::kernel_cache_.insert(new_kernel);
}

void
FilterBandRejectFIR::
plot(boolean show_fc,boolean show_phase)
{
    char title[128];
    sprintf(title,
        "Band Reject FIR Frequency Response\n"
        "order = %d, fl = %0.1f Hz, fl = %0.1f Hz, sr = %0.1f Hz",
        kernel_size_ - 1,
        frequency_1_Hz_,
        frequency_2_Hz_,
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

        pylab.axvline(frequency_1_Hz_,"color='red'");
        pylab.axvline(frequency_2_Hz_,"color='red'");

        pylab.title(title);
    }
}

void
FilterBandRejectFIR::
reset()
{
    FilterLowPassFIR::reset();
    makeKernel(frequency_1_Hz_, frequency_2_Hz_);
}

//-----------------------------------------------------------------------------
FilterBandRejectFIR::
Kernel::
Kernel(const uint32 & f1, const uint32 & f2)
    :
    b_(NULL),
    frequency_1_(f1),
    frequency_2_(f2)
{
}

bool
FilterBandRejectFIR::
Kernel::
operator<(const Kernel & rhs) const
{
    return  frequency_1_ < rhs.frequency_1_  ||
           (frequency_1_ == rhs.frequency_1_ &&
                frequency_2_ < rhs.frequency_2_);
}


