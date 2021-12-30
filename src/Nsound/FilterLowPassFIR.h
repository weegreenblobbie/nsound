//-----------------------------------------------------------------------------
//
//  $Id: FilterLowPassFIR.h 874 2014-09-08 02:21:29Z weegreenblobbie $
//
//  Nsound is a C++ library and Python module for audio synthesis featuring
//  dynamic digital filters. Nsound lets you easily shape waveforms and write
//  to disk or plot them. Nsound aims to be as powerful as Csound but easy to
//  use.
//
//  Copyright (c) 2006 - Present Nick Hilton
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
#ifndef _NSOUND_FILTER_LOW_PASS_FIR_H_
#define _NSOUND_FILTER_LOW_PASS_FIR_H_

#include <Nsound/Filter.h>

#include <set>

namespace Nsound
{

//-----------------------------------------------------------------------------
class FilterLowPassFIR : public Filter
{
    public:

    FilterLowPassFIR(
        const float64 & sample_rate,
        uint32 kernel_size,
        const float64 & cutoff_frequency_Hz);

    virtual ~FilterLowPassFIR();

    AudioStream
    filter(const AudioStream & x);

    AudioStream
    filter(const AudioStream & x, const float64 & frequency);

    AudioStream
    filter(const AudioStream & x, const Buffer & frequencies);

    Buffer
    filter(const Buffer & x);

    Buffer
    filter(const Buffer & x, const float64 & frequency);

    Buffer
    filter(const Buffer & x, const Buffer & frequencies);

    virtual
    float64
    filter(const float64 & x);

    float64
    filter(const float64 & x, const float64 & frequency_Hz);

    float64
    getFrequency() const {return frequency_1_Hz_;};

    ///////////////////////////////////////////////////////////////////////////////
    Buffer
    getImpulseResponse();

    void
    plot(boolean show_fc = true, boolean show_phase = false);

    //! Resets interal history buffer and sets the cut off frequency to the one
    //! used at declaration.
    void
    reset();

    //! Sets the cut off frequency (Hz).
    void
    setCutoff(const float64 & fc);

    protected:

    void
    makeKernel(const float64 & frequency1);

    float64 * b_;
    float64 * window_;

    // These are for point by point filtering. filter_(float64)
    float64 * x_history_;
    float64 * x_ptr_;
    float64 * x_end_ptr_;

    float64 frequency_1_Hz_;

    #ifndef SWIG
    //! A class to store calculated kernels.
    struct Kernel
    {
        Kernel(const uint32 & frequency);

        bool operator<(const Kernel & rhs) const;

        float64 * b_;

        uint32 frequency_;
    };

    typedef std::set<Kernel> KernelCache;

    KernelCache lp_cache_;
    #endif

    private:

    FilterLowPassFIR(const FilterLowPassFIR & copy);

    FilterLowPassFIR &
    operator=(const FilterLowPassFIR & rhs){return *this;};
;

}; // LowPassFilter

}; // Nsound

// :mode=c++: jEdit modeline
#endif
