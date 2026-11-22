//-----------------------------------------------------------------------------
//
//  $Id: FilterLowPassFIR.cc 874 2014-09-08 02:21:29Z weegreenblobbie $
//
//  Copyright (c) 2006 to Present Nick Hilton
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
#include <Nsound/FilterLowPassFIR.h>
#include <Nsound/Plotter.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <iostream>

using namespace Nsound;

using std::cerr;
using std::endl;

#define CERR_HEADER __FILE__ << ":" << __LINE__ << ": "

//-----------------------------------------------------------------------------
FilterLowPassFIR::
FilterLowPassFIR(
    const float64 & sample_rate,
    uint32 kernel_size,
    const float64 & cutoff_frequency_Hz)
    :
    Filter(sample_rate),
    frequency_1_Hz_(cutoff_frequency_Hz)
{
    kernel_size_ = kernel_size;

    // Keep kernel_size_ odd.
    if(kernel_size_ % 2 == 0)
    {
        ++kernel_size_;
    }

    x_history_.assign(kernel_size_ + 1, 0.0);
    x_ptr_ = x_history_.data();
    x_end_ptr_ = x_history_.data() + x_history_.size();

    // Create the Blackman window.
    float64 ks = static_cast<float64>(kernel_size_);
    window_.resize(kernel_size_);
    for(uint32 i = 0; i < kernel_size_; ++i)
    {
        float64 di = static_cast<float64>(i);

        window_[i] = 0.420
                   - 0.500 * std::cos((di + 0.5) * 2.0 * M_PI / ks)
                   + 0.080 * std::cos((di + 0.5) * 4.0 * M_PI / ks);

    }

    FilterLowPassFIR::reset();
}

AudioStream
FilterLowPassFIR::
filter(const AudioStream & x)
{
    return Filter::filter(x);
}

AudioStream
FilterLowPassFIR::
filter(const AudioStream & x, const float64 & f)
{
    return Filter::filter(x, f);
}

AudioStream
FilterLowPassFIR::
filter(const AudioStream & x, const Buffer & frequencies)
{
    return Filter::filter(x, frequencies);
}

Buffer
FilterLowPassFIR::
filter(const Buffer & x)
{
    return Filter::filter(x);
}

Buffer
FilterLowPassFIR::
filter(const Buffer & x, const float64 & f)
{
    // Instead of calling Filter::filter(x,f), we implement this function here
    // to avoid calling makeKernel(f) for each sample.

    reset();
    makeKernel(f);

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
FilterLowPassFIR::
filter(const Buffer & x, const Buffer & frequencies)
{
    return Filter::filter(x, frequencies);
}

float64
FilterLowPassFIR::
filter(const float64 & x)
{
    // Write x to history
    *x_ptr_ = x;

    // Increment history pointer
    ++x_ptr_;

    // Bounds check.
    if(x_ptr_ >= x_end_ptr_)
    {
        x_ptr_ = x_history_.data();
    }

    // Perform the convolution.

    // y[n] = kernel_[0] * x[n]
    //      + kernel_[1] * x[n - 1]
    //      + kernel_[2] * x[n - 2]
    //      ...
    //      + kernel_[N] * x[n - N]

    float64 y = 0.0;
    float64 * history = x_ptr_;
    for(const float64 * b = b_; b != b_ + kernel_size_; ++b)
    {
        // When we enter this loop, history is pointing at x[n + 1].
        --history;

        // Bounds check
        if(history < x_history_.data())
        {
            history = x_end_ptr_ - 1;
        }

        y += *b * *history;
    }

    return y;
}

float64
FilterLowPassFIR::
filter(const float64 & x, const float64 & frequency_Hz)
{
    FilterLowPassFIR::makeKernel(frequency_Hz);
    return filter(x);
}

Buffer
FilterLowPassFIR::
getImpulseResponse()
{
    return Filter::getImpulseResponse();
}

void
FilterLowPassFIR::
plot(boolean show_fc, boolean show_phase)
{
    char title[256];
    sprintf(title,
        "Low Pass FIR Frequency Response\n"
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
FilterLowPassFIR::
makeKernel(const float64 & cutoff_frequency_Hz)
{
    // Here we create a key into the kernel cache.  I'm only storing kernels
    // with freqs chopped off to the 10th Hz.  So filtering with a kernel
    // designed at 440.1567 Hz will get stored as 440.1.  Any other frequency
    // that starts at 440.1 will not get a kernel calculated and just use the
    // one in the cache.  So the max diff between what's passed in and what's
    // stored in the cache is 0.09999 Hz.

    FilterLowPassFIR::
    Kernel new_kernel(
        static_cast<uint32>(cutoff_frequency_Hz));

    // See if the kernel is in the cache.
    KernelCache::const_iterator itor = lp_cache_.find(new_kernel);

    if(itor != lp_cache_.end())
    {
        // The kernel was found in the cache.
        b_ = itor->b_.data();
        return;
    }

    // The filter wasn't in the cache, need to make it.
    new_kernel.b_.resize(kernel_size_);

    // Makes a Windowed-sinc filter with cutoff frequency specified.
    //
    //  DSP: A Practical Guide for Engineers and Scientists
    //
    //  ISBN-13: 978-0-7506-7444-7
    //
    //  Equation 16-4

    const float64 ks = static_cast<float64>(kernel_size_);

    const float64 omega = two_pi_over_sample_rate_ * cutoff_frequency_Hz;

    // cut off of zero Hz!
    if(cutoff_frequency_Hz < 0.10)
    {
        // Create no pass filter.
        std::fill(new_kernel.b_.begin(), new_kernel.b_.end(), 0.0);
    }
    else
    {
        float64 sum = 0.0;

        // The center tap of the odd sized kernel, where the window peaks, so
        // the kernel is symmetric (linear phase).
        float64 center = (ks - 1.0) / 2.0;

        float64 * b = new_kernel.b_.data();
        const float64 * w = window_.data();
        for(float64 i = 0.0; i < ks; i += 1.0)
        {
            float64 x = i - center + 1e-16;

            *b = std::sin(omega * x) / x * *w;

            sum += *b;

            ++b;
            ++w;
        }

        // Normalize kernel.
        for(float64 & b : new_kernel.b_)
        {
            b /= sum;
        }
    }

    // Add the new kernel to the cache and point to it there.
    b_ = lp_cache_.insert(std::move(new_kernel)).first->b_.data();

//~    for(int i = 0; i < kernel_size_; ++i)
//~    {
//~        cerr << "b_[" << i + 1 << "] = " << b_[i] << endl;
//~    }
}

void
FilterLowPassFIR::
reset()
{
    std::fill(x_history_.begin(), x_history_.end(), 0.0);
    x_ptr_ = x_history_.data();

    FilterLowPassFIR::makeKernel(frequency_1_Hz_);
}

void
FilterLowPassFIR::
setCutoff(const float64 & fc)
{
    frequency_1_Hz_ = fc;
    reset();
}

//-----------------------------------------------------------------------------
FilterLowPassFIR::
Kernel::
Kernel(const uint32 & frequency)
    :
    frequency_(frequency)
{
}

bool
FilterLowPassFIR::
Kernel::
operator<(const Kernel & rhs) const
{
    return frequency_ <  rhs.frequency_;
}

