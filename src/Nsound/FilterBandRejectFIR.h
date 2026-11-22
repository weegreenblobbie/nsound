//-----------------------------------------------------------------------------
//
//  $Id: FilterBandRejectFIR.h 874 2014-09-08 02:21:29Z weegreenblobbie $
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
#pragma once

#include <Nsound/FilterHighPassFIR.h>

#include <set>

namespace Nsound
{

class AudioStream;
class Buffer;

//-----------------------------------------------------------------------------
class FilterBandRejectFIR : public FilterHighPassFIR
{
    public:

    FilterBandRejectFIR(
        const float64 & sample_rate,
        uint32 kernel_size,
        const float64 & frequency_Hz_low,
        const float64 & frequency_Hz_high);

    float64
    getFrequencyLow() const;

    float64
    getFrequencyHigh() const;

    AudioStream
    filter(const AudioStream & x);

//~//~    AudioStream
//~    filter(const AudioStream & x, const Buffer & frequencies);

    AudioStream
    filter(
        const AudioStream & x,
        const float64 & f_low,
        const float64 & f_high);

    AudioStream
    filter(
        const AudioStream & x,
        const Buffer & low_frequencies,
        const Buffer & high_frequencies);

    Buffer
    filter(const Buffer & x);

//~//~    Buffer
//~    filter(const Buffer & x, const Buffer & frequencies);

    Buffer
    filter(
        const Buffer & x,
        const float64 & f_low,
        const float64 & f_high);

    Buffer
    filter(
        const Buffer & x,
        const Buffer & frequencies_Hz_low,
        const Buffer & frequencies_Hz_high);

    float64
    filter(const float64 & x) override;

    float64
    filter(const float64 & x, const float64 & frequency_Hz) override;

    virtual
    float64
    filter(
        const float64 & x,
        const float64 & frequency_Hz_low,
        const float64 & frequency_Hz_high);

    void
    makeKernel(
        const float64 & frequency_Hz_low,
        const float64 & frequency_Hz_high);

    void
    plot(boolean show_fc = true, boolean show_phase = false);

    //! Resets interal history buffer and sets the cut off frequency to the one
    //! used at declaration.
    void
    reset() override;

    protected:

    float64 frequency_2_Hz_;

    #ifndef SWIG
    //! A class to store calculated kernels.
    struct Kernel
    {
        Kernel(const uint32 & f1, const uint32 & f2);

        bool operator<(const Kernel & rhs) const;

        std::vector<float64> b_;

        uint32 frequency_1_;
        uint32 frequency_2_;
    };

    typedef std::set<Kernel> KernelCache;

    KernelCache kernel_cache_;
    #endif

    public:

    // Not copyable.
    FilterBandRejectFIR(const FilterBandRejectFIR & copy) = delete;

    FilterBandRejectFIR &
    operator=(const FilterBandRejectFIR & rhs) = delete;

}; // class FilterBandRejectFIR

} // namespace Nsound

// :mode=c++: jEdit modeline
