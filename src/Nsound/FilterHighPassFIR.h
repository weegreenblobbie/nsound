//-----------------------------------------------------------------------------
//
//  $Id: FilterHighPassFIR.h 874 2014-09-08 02:21:29Z weegreenblobbie $
//
//  Nsound is a C++ library and Python module for audio synthesis featuring
//  dynamic digital filters. Nsound lets you easily shape waveforms and write
//  to disk or plot them. Nsound aims to be as powerful as Csound but easy to
//  use.
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
#ifndef _NSOUND_FILTER_HIGH_PASS_FIR_H_
#define _NSOUND_FILTER_HIGH_PASS_FIR_H_

#include <Nsound/FilterLowPassFIR.h>

namespace Nsound
{

//-----------------------------------------------------------------------------
class FilterHighPassFIR : public FilterLowPassFIR
{
    public:

    FilterHighPassFIR(
        const float64 & sample_rate,
        uint32 kernel_size,
        const float64 & cutoff_frequency_Hz);

    virtual ~FilterHighPassFIR();

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

    virtual
    float64
    filter(const float64 & x, const float64 & frequency_Hz);

    float64
    getFrequency() const {return frequency_1_Hz_;};

    Buffer
    getImpulseResponse();

    void
    makeKernel(const float64 & frequency);

    void
    plot(boolean show_fc = true, boolean show_phase = false);

    //! Resets interal history buffer and sets the cut off frequency to the one
    //! used at declaration.
    virtual
    void
    reset();

    protected:

    FilterLowPassFIR::KernelCache hp_cache_;

    void
    spectraReversal_();

}; // HighPassFilter

}; // Nsound

// :mode=c++: jEdit modeline
#endif
