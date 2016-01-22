//-----------------------------------------------------------------------------
//
//  $Id: FilterStageIIR.h 874 2014-09-08 02:21:29Z weegreenblobbie $
//
//  Nsound is a C++ library and Python module for audio synthesis featuring
//  dynamic digital filters. Nsound lets you easily shape waveforms and write
//  to disk or plot them. Nsound aims to be as powerful as Csound but easy to
//  use.
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
#ifndef _NSOUND_FILTER_STAGE_IIR_H_
#define _NSOUND_FILTER_STAGE_IIR_H_

#include <Nsound/Filter.h>

#include <set>

namespace Nsound
{

// Forward class declarations
class AudioStream;
class Buffer;

//-----------------------------------------------------------------------------
//! A class for filtering audio in the frequecy domain.
class FilterStageIIR : public Filter
{
    public:

    enum Type
    {
        LOW_PASS,
        HIGH_PASS
    };

    FilterStageIIR(
        Type type,
        const float64 & sample_rate,
        uint32 n_poles,
        const float64 & frequency,
        const float64 & percent_ripple);

    FilterStageIIR(const FilterStageIIR & copy);

    virtual ~FilterStageIIR();

    AudioStream
    filter(const AudioStream & x);

    AudioStream
    filter(const AudioStream & x, const float64 & f);

    AudioStream
    filter(const AudioStream & x, const Buffer & frequencies);

    Buffer
    filter(const Buffer & x);

    Buffer
    filter(const Buffer & x, const float64 & f);

    Buffer
    filter(const Buffer & x, const Buffer & frequencies);

    float64
    filter(const float64 & x);

    float64
    filter(const float64 & x, const float64 & frequency);

    void
    makeKernel(const float64 & frequency);

    FilterStageIIR &
    operator=(const FilterStageIIR & rhs);

    void
    reset();

    protected:

    void
    makeIIRKernelHelper(
        const float64 & frequency,
        float64 * a,
        float64 * b,
        uint32 p);

    Type    type_;
    uint32  n_poles_;
    float64 frequency_;
    float64 percent_ripple_;

    float64 * a_;
    float64 * b_;

    float64 * x_history_;
    float64 * x_ptr_;
    float64 * x_end_ptr_;

    float64 * y_history_;
    float64 * y_ptr_;
    float64 * y_end_ptr_;

    //! A class to store calculated kernels.
    struct Kernel
    {
        Kernel(const uint32 & frequency);

        bool operator<(const Kernel & rhs) const;

        float64 * b_;
        float64 * a_;

        uint32 frequency_;
    };

    typedef std::set<Kernel> KernelCache;

    KernelCache kernel_cache_;

};

};

// :mode=c++: jEdit modeline

#endif
