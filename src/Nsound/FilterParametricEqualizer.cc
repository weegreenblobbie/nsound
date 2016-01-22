//-----------------------------------------------------------------------------
//
//  $Id: FilterParametricEqualizer.cc 874 2014-09-08 02:21:29Z weegreenblobbie $
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
#include <Nsound/FilterParametricEqualizer.h>
#include <Nsound/Plotter.h>

#include <cmath>
#include <cstdio>
#include <cstring>
#include <iostream>

using namespace Nsound;

static const uint64 N_POLES = 3;

//-----------------------------------------------------------------------------
FilterParametricEqualizer::
FilterParametricEqualizer(
    const Type & type,
    const float64 & sample_rate,
    const float64 & frequency,
    const float64 & resonance,
    const float64 & boost)
    :
    Filter(sample_rate),
    type_(type),
    frequency_(frequency),
    resonance_(resonance),
    boost_(boost),
    a_(new float64 [N_POLES]),
    b_(new float64 [N_POLES]),
    x_history_(NULL),
    x_ptr_(NULL),
    x_end_ptr_(NULL),
    y_history_(NULL),
    y_ptr_(NULL),
    y_end_ptr_(NULL),
    kernel_cache_()
{
    x_history_ = new float64 [N_POLES + 1];
    x_ptr_     = x_history_;
    x_end_ptr_ = x_history_ + N_POLES + 1;

    y_history_ = new float64 [N_POLES + 1];
    y_ptr_     = y_history_;
    y_end_ptr_ = y_history_ + N_POLES + 1;

    reset();
}

//-----------------------------------------------------------------------------
FilterParametricEqualizer::
~FilterParametricEqualizer()
{
    delete [] a_;
    delete [] b_;
    delete [] x_history_;
    delete [] y_history_;
}

AudioStream
FilterParametricEqualizer::
filter(const AudioStream & x)
{
    uint32 n_channels = x.getNChannels();

    AudioStream y(x.getSampleRate(), n_channels);

    for(uint32 channel = 0; channel < n_channels; ++channel)
    {
        y[channel] = filter(x[channel]);
    }

    return y;
}

AudioStream
FilterParametricEqualizer::
filter(const AudioStream & x, const Buffer & frequencies)
{
    uint32 n_channels = x.getNChannels();

    AudioStream y(x.getSampleRate(), n_channels);

    for(uint32 channel = 0; channel < n_channels; ++channel)
    {
        y[channel] = filter(x[channel], frequencies);
    }

    return y;
}

AudioStream
FilterParametricEqualizer::
filter(
    const AudioStream & x,
    const Buffer & frequencies,
    const Buffer & resonance)
{
    uint32 n_channels = x.getNChannels();

    AudioStream y(x.getSampleRate(), n_channels);

    for(uint32 channel = 0; channel < n_channels; ++channel)
    {
        y[channel] = filter(x[channel], frequencies, resonance);
    }

    return y;
}

AudioStream
FilterParametricEqualizer::
filter(
    const AudioStream & x,
    const Buffer & frequencies,
    const Buffer & resonance,
    const Buffer & boost)
{
    uint32 n_channels = x.getNChannels();

    AudioStream y(x.getSampleRate(), n_channels);

    for(uint32 channel = 0; channel < n_channels; ++channel)
    {
        y[channel] = filter(x[channel], frequencies, resonance, boost);
    }

    return y;
}


Buffer
FilterParametricEqualizer::
filter(const Buffer & x)
{
    reset();

    uint32 n_samples = x.getLength();

    Buffer y;

    for(uint32 n = 0; n < n_samples; ++n)
    {
        y << filter(x[n]);
    }

    return y;
}

Buffer
FilterParametricEqualizer::
filter(const Buffer & x, const Buffer & frequencies)
{
    reset();

    uint32 n_samples = x.getLength();
    uint32 n_freqs   = frequencies.getLength();

    Buffer y;

    for(uint32 n = 0; n < n_samples; ++n)
    {
        y << filter(x[n], frequencies[n % n_freqs]);
    }

    return y;
}

Buffer
FilterParametricEqualizer::
filter(
    const Buffer & x,
    const Buffer & frequencies,
    const Buffer & resonance)
{
    reset();

    uint32 n_samples = x.getLength();
    uint32 n_freqs   = frequencies.getLength();
    uint32 n_rezzies  = resonance.getLength();

    Buffer y;

    for(uint32 n = 0; n < n_samples; ++n)
    {
        y << filter(x[n], frequencies[n % n_freqs], resonance[n % n_rezzies]);
    }

    return y;
}

Buffer
FilterParametricEqualizer::
filter(
    const Buffer & x,
    const Buffer & frequencies,
    const Buffer & resonance,
    const Buffer & boost)
{
    reset();

    uint32 n_samples = x.getLength();
    uint32 n_freqs   = frequencies.getLength();
    uint32 n_rezzies = resonance.getLength();
    uint32 n_boosts  = boost.getLength();

    Buffer y;

    for(uint32 n = 0; n < n_samples; ++n)
    {
        y << filter(x[n],
            frequencies[n % n_freqs],
            resonance[n % n_rezzies],
            boost[n % n_boosts]);
    }

    return y;
}

float64
FilterParametricEqualizer::
filter(const float64 & x)
{
    // Write x to history
    *x_ptr_ = x;

    // Increment history pointer
    ++x_ptr_;

    // Bounds check
    if(x_ptr_ >= x_end_ptr_)
    {
        x_ptr_ = x_history_;
    }

    float64 y = 0.0;
    float64 * x_hist = x_ptr_;
    for(float64 * b = b_; b < b_ + N_POLES; ++b)
    {
        // When we enter this loop, x_hist is pointing at x[n + 1]
        --x_hist;

        // Bounds check
        if(x_hist < x_history_)
        {
            x_hist = x_end_ptr_ - 1;
        }

        y += *b * *x_hist;
    }

    float64 * y_hist = y_ptr_;
    for(float64 * a = a_ + 1; a < a_ + N_POLES; ++a)
    {
        // When we enter this loop, y_hist is pointing at y[n + 1]
        --y_hist;

        // Bounds check
        if(y_hist < y_history_)
        {
            y_hist = y_end_ptr_ - 1;
        }

        y -= *a * *y_hist;

    }

    // insert output into history buffer
    *y_ptr_ = y;

    // Increment history pointer
    ++y_ptr_;

    // Bounds check
    if(y_ptr_ >= y_end_ptr_)
    {
        y_ptr_ = y_history_;
    }

    return y;
}

float64
FilterParametricEqualizer::
filter(const float64 & x, const float64 & frequency)
{
    makeKernel(frequency, resonance_, boost_);

    return filter(x);
}

float64
FilterParametricEqualizer::
filter(
    const float64 & x,
    const float64 & frequency,
    const float64 & resonance)
{
    makeKernel(frequency, resonance, boost_);

    return filter(x);
}

float64
FilterParametricEqualizer::
filter(
    const float64 & x,
    const float64 & frequency,
    const float64 & resonance,
    const float64 & boost)
{
    makeKernel(frequency, resonance, boost);

    return filter(x);
}

void
FilterParametricEqualizer::
makeKernel(
    const float64 & frequency,
    const float64 & resonance,
    const float64 & boost)
{
    Kernel new_kernel(N_POLES, N_POLES,
        static_cast<int32>(std::fabs(frequency)),
        static_cast<int32>(resonance * 1e3),
        static_cast<int32>(boost * 1e3));

    // See if the kernel is in the cache.
    KernelCache::iterator itor = kernel_cache_.find(new_kernel);

    // Kernel was found in the cache, set pointers and return.
    if(itor != kernel_cache_.end())
    {
        memcpy(a_, itor->getA(), sizeof(float64) * N_POLES);
        memcpy(b_, itor->getB(), sizeof(float64) * N_POLES);

        return;
    }

    memset(a_, 0, sizeof(float64) * N_POLES);
    memset(b_, 0, sizeof(float64) * N_POLES);

    float64 omega = two_pi_over_sample_rate_ * frequency;

    switch(type_)
    {
        case PEAKING:
        {

//~            K     = tan(omega/2)
//~
//~            b0 =  1 + V*K + K^2
//~            b1 =  2*(K^2 - 1)
//~            b2 =  1 - V*K + K^2
//~
//~            a0 =  1 + K/Q + K^2
//~            a1 =  2*(K^2 - 1)
//~            a2 =  1 - K/Q + K^2

            float64 K = tan(omega / 2.0);
            float64 KK = K * K;
            float64 vkdq = boost * K / resonance;

            b_[0] = 1.0 + vkdq + KK;
            b_[1] = 2.0 * (KK - 1.0);
            b_[2] = 1.0 - vkdq + KK;

            a_[0] = 1.0 + K / resonance + KK;
            a_[1] = b_[1];
            a_[2] = 1.0 - K / resonance + KK;

            break;
        }

        case LOW_SHELF:
        {

//~            K     = tan(omega/2)
//~
//~            b0    = 1 + sqrt(2*V)*K + V*K^2
//~            b1    = 2*(V*K^2 - 1)
//~            b2    = 1 - sqrt(2*V)*K + V*K^2
//~
//~            a0    = 1 + K/Q + K^2
//~            a1    = 2*(K^2 - 1)
//~            a2    = 1 - K/Q + K^2

            float64 omega = 2.0 * M_PI * frequency / sample_rate_;
            float64 K = tan(omega / 2.0);
            float64 KK = K * K;

            float64 sqvk = sqrt(2.0 * boost) * K;
            float64 vkk = boost * KK;

            b_[0] = 1.0 + sqvk + vkk;
            b_[1] = 2.0 * (vkk - 1.0);
            b_[2] = 1.0 - sqvk + vkk;

            a_[0] = 1.0 + K / resonance + KK;
            a_[1] = 2.0 * (KK - 1.0);
            a_[2] = 1.0 - K / resonance + KK;

            break;
        }

        case HIGH_SHELF:
        {

//~            K     = tan((pi-omega)/2)
//~
//~            b0    = 1 + sqrt(2*V)*K + V*K^2
//~            b1    = -2*(V*K^2 - 1)
//~            b1    = 1 - sqrt(2*V)*K + V*K^2
//~
//~            a0    = 1 + K/Q + K^2
//~            a1    = -2*(K^2 - 1)
//~            a2    = 1 - K/Q + K^2

            float64 K = tan((M_PI - omega) * 0.5);
            float64 KK = K * K;
            float64 vkk = boost * KK;

            float64 sq = sqrt(2.0 * boost);

            b_[0] =  1.0 + sq * K + vkk;
            b_[1] = -2.0 * (vkk - 1.0);
            b_[2] =  1.0 - sq * K + vkk;

            a_[0] =  1.0 + K / resonance + KK;
            a_[1] = -2.0 * (KK - 1.0);
            a_[2] =  1.0 - K / resonance + KK;

            break;
        }
    }

    a_[0] = 1.0 / a_[0];

    a_[1] *= a_[0];
    a_[2] *= a_[0];

    b_[0] *= a_[0];
    b_[1] *= a_[0];
    b_[2] *= a_[0];

    new_kernel.setB(b_);
    new_kernel.setA(a_);

    // Add the new kernel to the cache.
    kernel_cache_.insert(new_kernel);
}


void
FilterParametricEqualizer::
plot(boolean show_fc, boolean show_phase)
{
    Filter::plot(show_phase);

    if(show_fc)
    {
        Plotter pylab;

        pylab.axvline(frequency_,"color='red'");

        char title[128];
        sprintf(title,
            "Parametric Equalizing Frequency Response\n"
            "order = %.0f, f = %0.1f Hz, sr = %0.1f Hz",
            static_cast<float64>(N_POLES - 1),
            frequency_,
            static_cast<float64>(sample_rate_));
        pylab.title(title);
    }
}

void
FilterParametricEqualizer::
reset()
{
    memset(x_history_, 0, sizeof(float64) * (N_POLES + 1));
    memset(y_history_, 0, sizeof(float64) * (N_POLES + 1));

    x_ptr_ = x_history_;
    y_ptr_ = y_history_;

    makeKernel(frequency_, resonance_, boost_);
}

