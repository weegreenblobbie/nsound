//-----------------------------------------------------------------------------
//
//  $Id: FilterStageIIR.cc 874 2014-09-08 02:21:29Z weegreenblobbie $
//
//  Copyright (c) 2008 Nick Hilton
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
#include <Nsound/FilterStageIIR.h>

#include <algorithm>
#include <cmath>
#include <string.h>
#include <iostream>

using namespace Nsound;

using std::cout;
using std::endl;

//-----------------------------------------------------------------------------
FilterStageIIR::
FilterStageIIR(
    Type type,
    const float64 & sample_rate,
    uint32 n_poles,
    const float64 & frequency,
    const float64 & percent_ripple)
    :
    Filter(sample_rate),
    type_(type),
    n_poles_(n_poles),
    frequency_(frequency),
    percent_ripple_(percent_ripple)
{
    if(n_poles_ > 20)
    {
        n_poles_ = 20;
    }
    else if(n_poles_ < 2)
    {
        n_poles_ = 2;
    }

    if(n_poles_ % 2 != 0)
    {
        ++n_poles_;
    }

    allocateHistory_();

    reset();
}

//-----------------------------------------------------------------------------
FilterStageIIR::
FilterStageIIR(const FilterStageIIR & copy)
    :
    Filter(copy.sample_rate_),
    type_(copy.type_),
    n_poles_(copy.n_poles_),
    frequency_(copy.frequency_),
    percent_ripple_(copy.percent_ripple_)
{
    *this = copy;
}

//-----------------------------------------------------------------------------
void
FilterStageIIR::
allocateHistory_()
{
    x_history_.assign(n_poles_ + 1, 0.0);
    x_ptr_     = x_history_.data();
    x_end_ptr_ = x_history_.data() + x_history_.size();

    y_history_.assign(n_poles_ + 1, 0.0);
    y_ptr_     = y_history_.data();
    y_end_ptr_ = y_history_.data() + y_history_.size();
}

AudioStream
FilterStageIIR::
filter(const AudioStream & x)
{
    return Filter::filter(x);
};


AudioStream
FilterStageIIR::
filter(const AudioStream & x, const float64 & f)
{
    return Filter::filter(x, f);
}

AudioStream
FilterStageIIR::
filter(const AudioStream & x, const Buffer & frequencies)
{
    return Filter::filter(x, frequencies);
}

Buffer
FilterStageIIR::
filter(const Buffer & x)
{
    return Filter::filter(x);
}

Buffer
FilterStageIIR::
filter(const Buffer & x, const float64 & f)
{
    // Instead of calling Filter::filter(x,f), we implement this function here
    // to avoid calling makeKernel(f) for each sample.

//~    cout << "f = " << f << endl;

    FilterStageIIR::reset();
    FilterStageIIR::makeKernel(f);

    Buffer::const_iterator itor = x.begin();
    Buffer::const_iterator end = x.end();

    Buffer y(x.getLength());
    while(itor != end)
    {
        y << FilterStageIIR::filter(*itor);
        ++itor;
    }

    return y;
}

Buffer
FilterStageIIR::
filter(const Buffer & x, const Buffer & frequencies)
{
    return Filter::filter(x, frequencies);
}

//-----------------------------------------------------------------------------
FilterStageIIR &
FilterStageIIR::
operator=(const FilterStageIIR & rhs)
{
    if(this == &rhs) return *this;

    uint32 n_poles_orig = n_poles_;

    type_           = rhs.type_;
    n_poles_        = rhs.n_poles_;
    frequency_      = rhs.frequency_;
    percent_ripple_ = rhs.percent_ripple_;

    // Copy the kernel cache, reset() points b_ and a_ into this one.
    kernel_cache_ = rhs.kernel_cache_;

    // Setup history memory, the copy constructor has none yet.
    if(n_poles_orig != rhs.n_poles_ || x_history_.empty())
    {
        allocateHistory_();
    }

    reset();

    return *this;
}

void
FilterStageIIR::
reset()
{
    std::fill(x_history_.begin(), x_history_.end(), 0.0);
    std::fill(y_history_.begin(), y_history_.end(), 0.0);

    x_ptr_ = x_history_.data();
    y_ptr_ = y_history_.data();

    makeKernel(frequency_);
}

float64
FilterStageIIR::
filter(const float64 & x)
{
    // Write x to history
    *x_ptr_ = x;

    // Increment history pointer
    ++x_ptr_;

    // Bounds check
    if(x_ptr_ >= x_end_ptr_)
    {
        x_ptr_ = x_history_.data();
    }

    float64 y = 0.0;
    float64 * x_hist = x_ptr_;
    for(const float64 * b = b_; b != b_ + n_poles_ + 1; ++b)
    {
        // When we enter this loop, x_hist is pointing at x[n + 1]
        --x_hist;

        // Bounds check
        if(x_hist < x_history_.data())
        {
            x_hist = x_end_ptr_ - 1;
        }

        y += *b * *x_hist;
    }

    float64 * y_hist = y_ptr_;
    for(const float64 * a = a_ + 1; a != a_ + n_poles_ + 1; ++a)
    {
        // When we enter this loop, y_hist is pointing at y[n + 1]
        --y_hist;

        // Bounds check
        if(y_hist < y_history_.data())
        {
            y_hist = y_end_ptr_ - 1;
        }

        y += *a * *y_hist;

    }

    // insert output into history buffer
    *y_ptr_ = y;

    // Increment history pointer
    ++y_ptr_;

    // Bounds check
    if(y_ptr_ >= y_end_ptr_)
    {
        y_ptr_ = y_history_.data();
    }

    return y;
}

float64
FilterStageIIR::
filter(const float64 & x, const float64 & frequency)
{
    makeKernel(frequency);
    return FilterStageIIR::filter(x);
}

void
FilterStageIIR::
makeKernel(const float64 & frequency)
{
    FilterStageIIR::
    Kernel new_kernel(static_cast<uint32>(frequency));

    // See if the kernel is in the cache.

    FilterStageIIR::
    KernelCache::const_iterator itor = kernel_cache_.find(new_kernel);

    if(itor != kernel_cache_.end())
    {
        b_ = itor->b_.data();
        a_ = itor->a_.data();

        return;
    }

    // The new kernel, it starts at 0.
    new_kernel.b_.assign(n_poles_ + 1, 0.0);
    new_kernel.a_.assign(n_poles_ + 1, 0.0);

    // No kernel in cache, must make it.

    if(type_ == LOW_PASS && frequency < 1.0)
    {
        // create no pass filter
        std::fill(new_kernel.b_.begin(), new_kernel.b_.end(), 0.0);
        std::fill(new_kernel.a_.begin(), new_kernel.a_.end(), 0.0);
    }

    else if(type_ == LOW_PASS && frequency >= (sample_rate_ / 2.0))
    {
        // create all pass
        std::fill(new_kernel.b_.begin(), new_kernel.b_.end(), 0.0);
        std::fill(new_kernel.a_.begin(), new_kernel.a_.end(), 0.0);

        new_kernel.b_[0] = 1.0;
    }

    else if(type_ == HIGH_PASS && frequency < 1.0)
    {
        // create all pass
        std::fill(new_kernel.b_.begin(), new_kernel.b_.end(), 0.0);
        std::fill(new_kernel.a_.begin(), new_kernel.a_.end(), 0.0);

        new_kernel.b_[0] = 1.0;
    }

    else if(type_ == HIGH_PASS && frequency >= (sample_rate_ / 2.0))
    {
        // create no pass filter
        std::fill(new_kernel.b_.begin(), new_kernel.b_.end(), 0.0);
        std::fill(new_kernel.a_.begin(), new_kernel.a_.end(), 0.0);
    }

    else
    {

        float64 B[23];
        float64 A[23];

        float64 TB[23];
        float64 TA[23];

        memset(B,0,sizeof(float64) * 23);
        memset(A,0,sizeof(float64) * 23);
        memset(TB,0,sizeof(float64) * 23);
        memset(TA,0,sizeof(float64) * 23);

        B[2] = A[2] = 1.0;

        for(uint32 p = 1; p <= n_poles_ / 2; ++p)
        {
            float64 b[3];
            float64 a[3];

            makeIIRKernelHelper(frequency, b, a, p);

            memcpy(TB,B, sizeof(float64) * 23);
            memcpy(TA,A, sizeof(float64) * 23);

            for(uint32 i = 2; i <= 22; ++i)
            {
                B[i] = b[0] * TB[i] + b[1] * TB[i - 1] + b[2] * TB[i - 2];
                A[i] =        TA[i] - a[1] * TA[i - 1] - a[2] * TA[i - 2];
            }
        }

        A[2] = 0.0;
        for(uint32 i = 0; i <= 20; ++i)
        {
            B[i] =        B[i + 2];
            A[i] = -1.0 * A[i + 2];
        }

        float64 sumB = 0.0;
        float64 sumA = 0.0;

        for(int32 i = 0; i <= 22; ++i)
        {
            if(type_ == LOW_PASS)
            {
                sumB += B[i];
                sumA += A[i];
            }
            else if(type_ == HIGH_PASS)
            {
                sumB += B[i] * pow(-1.0,i);
                sumA += A[i] * pow(-1.0,i);
            }
        }
        float64 gain = sumB / (1.0 - sumA);

        for(uint32 i = 0; i <= 22; ++i)
        {
            B[i] /= gain;
        }

        std::copy(B, B + n_poles_ + 1, new_kernel.b_.begin());
        std::copy(A, A + n_poles_ + 1, new_kernel.a_.begin());
    }

    // Add the new kernel to the cache and point to it there.
    KernelCache::const_iterator cached =
        kernel_cache_.insert(std::move(new_kernel)).first;

    b_ = cached->b_.data();
    a_ = cached->a_.data();
}

void
FilterStageIIR::
makeIIRKernelHelper(
    const float64 & frequency,
    float64 * b,
    float64 * a,
    uint32 p)
{
    float64 n_poles = static_cast<float64>(n_poles_);

    float64 RP = -1.0 * cos(M_PI / (n_poles * 2.0) + (p - 1) * (M_PI / n_poles));
    float64 IP = sin(M_PI / (n_poles * 2.0) + (p - 1) * (M_PI / n_poles));

    float64 ES = 0.0;
    float64 VX = 0.0;
    float64 KX = 0.0;

    if(percent_ripple_ != 0.0)
    {
        float64 temp = (1.0 / (1.0 - percent_ripple_));

        ES = sqrt(temp * temp - 1.0);

        VX = (1.0 / n_poles) * log((1.0 / ES) + sqrt((1.0 /(ES * ES)) + 1.0));
        KX = (1.0 / n_poles) * log((1.0 / ES) + sqrt((1.0 /(ES * ES)) - 1.0));
        KX = (exp(KX) + exp(-KX)) / 2.0;
        RP = RP * ( (exp(VX) - exp(-1.0 * VX) ) / 2.0) / KX;
        IP = IP * ( (exp(VX) + exp(-1.0 * VX) ) / 2.0) / KX;
    }

    /* DEBUG
    if(p == 1)
    {
        std::cout << endl
                  << "RP = " << RP << endl
                  << "IP = " << IP << endl
                  << "ES = " << ES << endl
                  << "VX = " << VX << endl
                  << "KX = " << KX << endl << endl;
    }
    */


    float64 T = 2.0 * tan(0.5);
    float64 W = 2.0 * M_PI * (frequency / static_cast<float64>(sample_rate_));
    float64 M = RP * RP + IP * IP;
    float64 D = 4.0 - (4.0 * RP * T) + (M * T * T);
    float64 X0 = T * T / D;
    float64 X1 = 2.0 * X0;
    float64 X2 = X0;
    float64 Y1 = (8.0 - 2.0 * M * T * T) / D;
    float64 Y2 = (-4.0 - (4.0 * RP * T) - M * T * T) / D;

    /* DEBUG
    if(p == 1)
    {
        std::cout << "T  = " << T << endl
                  << "W  = " << W << endl
                  << "M  = " << M << endl
                  << "D  = " << D << endl
                  << "X0 = " << X0 << endl
                  << "X1 = " << X1 << endl
                  << "X2 = " << X2 << endl
                  << "Y1 = " << Y1 << endl
                  << "Y2 = " << Y2 << endl << endl;
    }
    */


    // Low Pass
    float64 K = 0.0;

    if(type_ == LOW_PASS)
    {
        K = sin(0.5 - W / 2.0) / sin(0.5 + W / 2.0);
    }

    // High Pass
    else if(type_ == HIGH_PASS)
    {
        K = -1.0 * cos(W / 2.0 + 0.5) / cos(W / 2.0 - 0.5);
    }

    D = 1.0 + Y1 * K - Y2 * K * K;
    b[0] = (X0 - X1 * K + X2 * K * K) / D;
    b[1] = (-2.0 * X0 *K + X1 + X1 * K * K - 2.0 * X2 * K) / D;
    b[2] = (X0 * K * K - X1 * K + X2) / D;
    a[1] = (2.0 * K + Y1 + Y1 * K * K - 2.0 * Y2 * K) / D;
    a[2] = (-1.0 * K * K - Y1 * K + Y2) / D;

    /* DEBUG
    if(p == 1)
    {
        std::cout << "W  = " << W    << endl
                  << "K  = " << K    << endl
                  << "D  = " << D    << endl
                  << "A0 = " << a[0] << endl
                  << "A1 = " << a[1] << endl
                  << "A2 = " << a[2] << endl
                  << "B1 = " << b[1] << endl
                  << "B2 = " << b[2] << endl << endl;
    }
    */

    if(type_ == HIGH_PASS)
    {
        b[1] = -b[1];
        a[1] = -a[1];
    }

    /* DEBUG
    if(p == 1)
    {
        std::cout << "A0 = " << a[0] << endl
                  << "A1 = " << a[1] << endl
                  << "A2 = " << a[2] << endl
                  << "B1 = " << b[1] << endl
                  << "B2 = " << b[2] << endl << endl;
    }
    */
}

//-----------------------------------------------------------------------------
FilterStageIIR::
Kernel::
Kernel(const uint32 & frequency)
    :
    frequency_(frequency)
{
}

bool
FilterStageIIR::
Kernel::
operator<(const Kernel & rhs) const
{
    return frequency_ <  rhs.frequency_;
}

