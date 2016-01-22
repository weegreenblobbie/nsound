//-----------------------------------------------------------------------------
//
//  $Id: FilterIIR.cc 874 2014-09-08 02:21:29Z weegreenblobbie $
//
//  Copyright (c) 2009 to Present Nick Hilton
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
#include <Nsound/FilterIIR.h>
#include <Nsound/FFTransform.h>
#include <Nsound/Kernel.h>
#include <Nsound/Plotter.h>
#include <Nsound/RngTausworthe.h>

//~#include <cmath>
#include <cstdio>
#include <cstring>
#include <iomanip>
#include <iostream>

#include <stdlib.h>
#include <math.h>    // isnan, isinf

using namespace Nsound;

using std::cerr;
using std::cout;
using std::endl;

//~using std::isnan;
//~using std::isinf;

#define PRINT_LINE "FilterIIR.cc:" << __LINE__ << ": "

//-----------------------------------------------------------------------------
FilterIIR::
FilterIIR(
    const float64 & sample_rate,
    uint32 n_poles)
    :
    Filter(sample_rate),
    n_poles_(n_poles),
    kernel_(NULL),
    x_history_(NULL),
    x_ptr_(NULL),
    x_end_ptr_(NULL),
    y_history_(NULL),
    y_ptr_(NULL),
    y_end_ptr_(NULL),
    rng_(NULL)
{
    kernel_ = new Kernel(n_poles_, n_poles_);

    x_history_ = new float64 [n_poles_ + 1];
    x_ptr_     = x_history_;
    x_end_ptr_ = x_history_ + n_poles_ + 1;

    y_history_ = new float64 [n_poles_ + 1];
    y_ptr_     = y_history_;
    y_end_ptr_ = y_history_ + n_poles_ + 1;

    rng_ = new RngTausworthe();

    reset();
}

//-----------------------------------------------------------------------------
FilterIIR::
FilterIIR(const FilterIIR & copy)
    :
    Filter(copy.sample_rate_),
    n_poles_(copy.n_poles_),
    kernel_(NULL),
    x_history_(NULL),
    x_ptr_(NULL),
    x_end_ptr_(NULL),
    y_history_(NULL),
    y_ptr_(NULL),
    y_end_ptr_(NULL),
    rng_(NULL)
{
    x_history_ = new float64 [n_poles_];
    y_history_ = new float64 [n_poles_];

    rng_ = new RngTausworthe();

    *this = copy;

    reset();
}

//-----------------------------------------------------------------------------
FilterIIR::
~FilterIIR()
{
    delete kernel_;
    delete [] x_history_;
    delete [] y_history_;

    delete rng_;
}

Buffer
FilterIIR::
designFrequencyResponse(
    const Buffer &  frequency_response,
    const float64 & max_error,
    const int32     max_iterations)
{
    return designKernel(
        frequency_response,
        max_error,
        max_iterations,
        FREQUENCY_RESSPONSE);
}

Buffer
FilterIIR::
designImpulseResponse(
    const Buffer &  impulse_response,
    const float64 & max_error,
    const int32     max_iterations)
{
    return designKernel(
        impulse_response,
        max_error,
        max_iterations,
        IMPULSE_RESPONSE);
}
//~
//~     ///////////////////////////////////////////////////////////////////////////////
//~     float64
//~     FilterIIR::
//~     findZScore(
//~         const Kernel & k,
//~         const Buffer & response,
//~         const SignalType type)
//~     {
//~         Kernel * orig_k = kernel_;
//~
//~         kernel_ = const_cast<Kernel *>(&k);
//~
//~         reset();
//~
//~         Buffer r;
//~
//~         if(type == FREQUENCY_RESSPONSE)
//~         {
//~             r = getFrequencyResponse();
//~         }
//~         else
//~         {
//~             r = getImpulseResponse();
//~         }
//~
//~         kernel_ = orig_k;
//~
//~     //~     // Check for nans or infs or really big values
//~     //~     for(Buffer::const_iterator itor = r.begin();
//~     //~         itor != r.end();
//~     //~         ++itor)
//~     //~     {
//~     //~         if(isnan(*itor) || isinf(*itor) || -100.0 > *itor || *itor > 100.0)
//~     //~         {
//~     //~             return 1.0;
//~     //~         }
//~     //~     }
//~
//~         Buffer diff(response);
//~
//~         while(diff.getLength() < r.getLength())
//~             diff << 0.0;
//~     //~
//~     //~     diff -= r;
//~     //~
//~     //~     diff *= diff;
//~     //~
//~     //~     return diff.getSum();
//~
//~         diff.zNorm();
//~         r.zNorm();
//~
//~         return (diff * r).getSum();
//~
//~     }

float64
FilterIIR::
getRMS(
    const Kernel & k,
    const Buffer & ref_response,
    const SignalType type)
{
    Kernel * orig_k = kernel_;

    kernel_ = const_cast<Kernel *>(&k);

    reset();

    Buffer r;

    if(type == FREQUENCY_RESSPONSE)
    {
        r = getFrequencyResponse(ref_response.getLength() * 2);
    }
    else
    {
        r = getImpulseResponse(ref_response.getLength() * 2);
    }

    kernel_ = orig_k;

    Buffer delta = ref_response - r;
    delta *= delta;

    float64 rms = delta.getSum() / static_cast<float64>(delta.getLength());

    rms = ::sqrt(rms);

    return rms;
}

//-----------------------------------------------------------------------------
//! This method is VERY EXPERMENTAL!  Use at your own risk!
Buffer
FilterIIR::
designKernel(
    const Buffer  &  ref_response,
    const float64 &  max_rms_error,
    const int32      max_iterations,
    const SignalType type)
{
    // The reference response curve must be a power of two to guarentee correct
    // comparision to the designed kernel response.

    uint32 p2 = 2;
    uint32 ref_size = ref_response.getLength();

    while( p2 < ref_size )
    {
        p2 <<= 1;
    }

    M_ASSERT_VALUE(ref_size, ==, p2);

    if(ref_size != p2) ::exit(1);

    // Create the initial 2 parents
    Kernel * mom = new Kernel(n_poles_, n_poles_);

    float64 LARGE_RMS = 1.0;
    float64 STALE_RMS = 0.001;

    float64 rms_error = LARGE_RMS;
    int32 dead_count = 0;
    while(rms_error > LARGE_RMS && dead_count < 1000)
    {
        mom->randomize(-0.1, 0.1);
        rms_error = getRMS(*mom, ref_response, type);
        dead_count++;
    }

    M_ASSERT_VALUE(rms_error, <=, LARGE_RMS);

    if(rms_error > LARGE_RMS) ::exit(1);

    Kernel * dad = new Kernel(n_poles_, n_poles_);

    dad->randomize(-0.1, 0.1);

    int32 N_CHILDREN = 14;

    // Allocate N children
    Kernel ** child = new Kernel * [N_CHILDREN];

    for(int32 i = 0; i < N_CHILDREN; ++i)
    {
        child[i] = new Kernel(n_poles_, n_poles_);
    }

    Buffer rms_history;

    rms_history << rms_error;

    float64 dad_rms_error = getRMS(*dad, ref_response, type);

    cout << std::setprecision(10)
         << "mom's RMS = " << rms_error << endl
         << "dad's RMS = " << dad_rms_error << endl;

    float64 rms_error1 = LARGE_RMS;
    float64 rms_error2 = LARGE_RMS;

    Kernel * low1 = NULL;
    Kernel * low2 = NULL;

    float64 last_rms_error = rms_error;
    int32 stale_count = 0;
    int32 major_stale_count = 0;
    int32 count = 0;

    while(count < max_iterations && rms_error > max_rms_error)
    {
        ++count;

        // First two children just swap mom & dads a & b coeffs;
        *child[0] = *mom;
        *child[1] = *dad;
        child[0]->ga_swap_ab(*child[1]);

        #ifdef GA_DEBUG
            cout << __LINE__ << "\tmom.getSum() = " << mom->getSum()
                 << "\terror = " << error << endl << endl;
        #endif

        // Next two children interleave coeffs
        *child[2] = *mom;
        *child[3] = *dad;

        child[2]->ga_interleave(*child[3]);

        #ifdef GA_DEBUG
            cout << __LINE__ << "\tmom.getSum() = " << mom->getSum()
                 << "\terror = " << error << endl << endl;
        #endif

        // The next two children randomly mutate one coeff from mom & dad
        *child[4] = *mom;
        *child[5] = *dad;

        for(uint32 i = 4; i < 6; ++i)
        {
            float64 delta = rng_->get(-0.1f, 0.1f);

            uint32 index = rng_->get(0, n_poles_);

            child[i]->setB(child[i]->getB(index) + delta, index);

            delta = rng_->get(-0.1f, 0.1f);

            index = rng_->get(0, n_poles_);

            child[i]->setA(child[i]->getA(index) + delta, index);
        }

        #ifdef GA_DEBUG
            cout << __LINE__ << "\tmom.getSum() = " << mom->getSum()
                 << "\terror = " << error << endl << endl;
        #endif

        // The next two children randomly mutate sign
        *child[6] = *mom;
        *child[7] = *dad;

        for(uint32 i = 6; i < 8; ++i)
        {
            uint32 index = rng_->get(0, n_poles_);

            float64 a_or_b = rng_->get(-1.0f, 1.0f);

            if(a_or_b > 0.0)
            {
                child[i]->setB(child[i]->getB(index) * -1.0, index);
            }
            else
            {
                child[i]->setA(child[i]->getA(index) * -1.0, index);
            }
        }

        #ifdef GA_DEBUG
            cout << __LINE__ << "\tmom.getSum() = " << mom->getSum()
                 << "\terror = " << error << endl << endl;
        #endif

        // These two children randomly multiply one coef.
        *child[8] = *mom;
        *child[9] = *dad;

        for(uint32 i = 8; i < 10; ++i)
        {
            uint32 index = rng_->get(0, n_poles_);

            float64 a_or_b = rng_->get(-1.0f, 1.0f);

            float64 delta = rng_->get(-0.1f, 0.1f);

            if(a_or_b > 0.0)
            {
                child[i]->setB(child[i]->getB(index) * delta, index);
            }
            else
            {
                child[i]->setA(child[i]->getA(index) * delta, index);
            }
        }

        #ifdef GA_DEBUG
            cout << __LINE__ << "\tmom.getSum() = " << mom->getSum()
                 << "\terror = " << error << endl << endl;
        #endif

        // These two children randomly swap mom & dad coeffs
        *child[10] = *mom;
        *child[11] = *dad;

        uint32 start_index = rng_->get(0, n_poles_);
        uint32 stop_index  = rng_->get(0, n_poles_);

        if(stop_index >= n_poles_) stop_index = n_poles_ - 1;

        if(start_index > stop_index)
        {
            uint32 temp = start_index;
            start_index = stop_index;
            stop_index = temp;
        }

        for(uint32 j = start_index; j < stop_index; ++j)
        {
            child[10]->setB(dad->getB(j), j);
            child[11]->setB(mom->getB(j), j);
        }

        #ifdef GA_DEBUG
            cout << __LINE__ << "\tmom.getSum() = " << mom->getSum()
                 << "\terror = " << error << endl << endl;
        #endif

        start_index = rng_->get(0, n_poles_);
        stop_index  = rng_->get(0, n_poles_);

        if(stop_index >= n_poles_) stop_index = n_poles_ - 1;

        if(start_index > stop_index)
        {
            int32 temp = start_index;
            start_index = stop_index;
            stop_index = temp;
        }

        for(uint32 j = start_index; j < stop_index; ++j)
        {
            child[10]->setA(dad->getA(j), j);
            child[11]->setA(mom->getA(j), j);
        }

        #ifdef GA_DEBUG
            cout << __LINE__ << "\tmom.getSum() = " << mom->getSum()
                 << "\terror = " << error << endl << endl;
        #endif

        // These two children randomly add mom & dad coeffs
        *child[12] = *mom;
        *child[13] = *dad;

        for(uint32 i = 12; i < 14; ++i)
        {
            for(uint32 j = 0; j < n_poles_; ++j)
            {
                child[i]->setB(child[i]->getB(j) + rng_->get(-0.1f, 0.1f), j);
                child[i]->setA(child[i]->getA(j) + rng_->get(-0.1f, 0.1f), j);
            }
        }

        // Score the kiddies.

        rms_error1 = getRMS(*mom, ref_response, type);
        rms_error2 = getRMS(*dad, ref_response, type);

        low1 = mom;
        low2 = dad;

        for(int32 i = 0; i < N_CHILDREN; ++i)
        {
            float64 rms = getRMS(*child[i], ref_response, type);

            if(rms > LARGE_RMS) continue;

            if(rms < rms_error1)
            {
                rms_error1 = rms;
                low1 = child[i];
            }
            else if(rms < rms_error2)
            {
                rms_error2 = rms;
                low2 = child[i];
            }
        }

        #ifdef GA_DEBUG
            cout << __LINE__ << "\tmom.getSum() = " << mom->getSum()
                 << "\terror = " << error << endl << endl;
        #endif

        *mom = *low1;
        *dad = *low2;

        rms_error     = getRMS(*mom, ref_response, type);
        dad_rms_error = getRMS(*dad, ref_response, type);

        rms_history << rms_error;

        #ifdef GA_DEBUG
            cout << __LINE__ << "\tmom.getSum() = " << mom->getSum()
                 << "\terror = " << error << endl;
        #endif

        // Check if dad is unstable.
        if(dad_rms_error > LARGE_RMS)
        {
            cout << PRINT_LINE << "dad's RMS > 10, ballistic!" << endl;
            dad->randomize(-0.01f, 0.01f);
        }

        // Check if our evolution is getting stale
        if(std::fabs(rms_error - last_rms_error) < STALE_RMS)
        {
            ++stale_count;
        }
        else
        {
            stale_count = 0;
        }

        last_rms_error = rms_error;

        if(count % 200 == 0)
        {
            cout << "Generation " << count << endl
                 << "    1st maximum RMS = " << rms_error     << endl
                 << "    2nd maximum RMS = " << dad_rms_error << endl;
//~                  << "    major_stale_count = " << major_stale_count << endl;
        }

        if(stale_count > 6)
        {
            for(uint32 i = 0; i < n_poles_; ++i)
            {
                dad->setB(dad->getB(i) + rng_->get(-0.10501f, 0.10501f), i);
                dad->setA(dad->getA(i) + rng_->get(-0.10501f, 0.10501f), i);
            }

            stale_count = 0;
            ++major_stale_count;

            if(major_stale_count > 1)
            {
                for(uint32 i = 0; i < n_poles_; ++i)
                {
                    dad->setB(dad->getB(i) * rng_->get(-0.501f, 0.501f), i);
                    dad->setA(dad->getA(i) * rng_->get(-0.501f, 0.501f), i);
                }

                major_stale_count = 0;
            }
        }

        #ifdef GA_DEBUG
            cout << __LINE__ << "\tmom.getSum() = " << mom->getSum()
                 << "\terror = " << error << endl;
        #endif

    }

    cout << "count = " << count << "  RMS = " << rms_error << endl;

    // copy mom into the filter.
    *kernel_ = *mom;

    for(int32 i = 0; i < N_CHILDREN; ++i)
    {
        delete child[i];
    }

    delete [] child;
    delete mom;
    delete dad;

    return rms_history;
}


float64
FilterIIR::
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
    const float64 * bb = kernel_->getB();
    const float64 * b_end = bb + n_poles_;
    for(const float64 * b = bb; b < b_end; ++b)
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
    const float64 * aa = kernel_->getA();
    const float64 * a_end = aa + n_poles_;
    for(const float64 * a = aa + 1; a < a_end; ++a)
    {
        // When we enter this loop, y_hist is pointing at y[n + 1]
        --y_hist;

        // Bounds check
        if(y_hist < y_history_)
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
        y_ptr_ = y_history_;
    }

    return y;
}

float64
FilterIIR::
filter(const float64 & x, const float64 & frequency)
{
    return FilterIIR::filter(x);
}

//-----------------------------------------------------------------------------
FilterIIR &
FilterIIR::
operator=(const FilterIIR & rhs)
{
    if(this == &rhs)
    {
        return *this;
    }

    sample_rate_ = rhs.sample_rate_;

    if(n_poles_ != rhs.n_poles_)
    {
        delete [] x_history_;
        delete [] y_history_;

        n_poles_ = rhs.n_poles_;

        x_history_ = new float64 [n_poles_];
        y_history_ = new float64 [n_poles_];
    }

    memcpy(x_history_, rhs.x_history_, sizeof(float64) * n_poles_);
    memcpy(y_history_, rhs.y_history_, sizeof(float64) * n_poles_);

    *kernel_ = *rhs.kernel_;
    *rng_ = *rhs.rng_;

    return *this;
}

//-----------------------------------------------------------------------------
std::ostream &
Nsound::
operator<<(std::ostream & out,const FilterIIR & rhs)
{
    out << "kernel_.b_length_ = " << rhs.kernel_->getBLength() << endl
        << "kernel_.a_length_ = " << rhs.kernel_->getALength() << endl;
    return out << *rhs.kernel_;
}

void
FilterIIR::
reset()
{
    memset(x_history_, 0, sizeof(float64) * (n_poles_ + 1));
    memset(y_history_, 0, sizeof(float64) * (n_poles_ + 1));

    x_ptr_ = x_history_;
    y_ptr_ = y_history_;
}

void
FilterIIR::
savePlot(
    const Kernel & k,
    const Buffer & fr_reference,
    uint32 n,
    const float64 & error)
{
    Kernel * orig_k = kernel_;

    kernel_ = const_cast<Kernel *>(&k);

    Buffer fr_ref = fr_reference;
    Buffer fr = getFrequencyResponse();
    Buffer faxis = getFrequencyAxis();

    while(fr_ref.getLength() < fr.getLength())
    {
        fr_ref << 0.0;
    }

    float64 max = fr_ref.getMax();

    Plotter pylab;

    pylab.figure();
    pylab.plot(faxis, fr_ref);
    pylab.plot(faxis, fr);

    pylab.xlabel("Frequency Hz");
    pylab.ylabel("Frequency Response");

    char buffer[256];

    sprintf(buffer, "Generation %06d", n);

    pylab.title(std::string(buffer));

    pylab.xlim(0.0, faxis[fr_reference.getLength() - 1]);
    pylab.ylim(0.0, 1.1 * max);

    sprintf(buffer, "Error = %5.2f", error);

    pylab.text(
        faxis[static_cast<uint32>(faxis.getLength() * 0.66)],
        0.66,
        std::string(buffer));

    sprintf(buffer, "generation_%06d.png", n);

//~    pylab.savefig(std::string(buffer));

    kernel_ = orig_k;

    reset();
}
