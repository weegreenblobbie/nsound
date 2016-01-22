//-----------------------------------------------------------------------------
//
//  $Id: FilterLeastSquaresFIR.h 874 2014-09-08 02:21:29Z weegreenblobbie $
//
//  Nsound is a C++ library and Python module for audio synthesis featuring
//  dynamic digital filters. Nsound lets you easily shape waveforms and write
//  to disk or plot them. Nsound aims to be as powerful as Csound but easy to
//  use.
//
//  Copyright (c) 2010 - Present Nick Hilton
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
#ifndef _NSOUND_FILTER_LEAST_SQUARES_FIR_H_
#define _NSOUND_FILTER_LEAST_SQUARES_FIR_H_

#include <Nsound/Filter.h>
#include <Nsound/WindowType.h>

#include <set>

namespace Nsound
{

//-----------------------------------------------------------------------------
//! A FIR filter that is defined as the least square error to the desired requency response.
//
//! Based on Matlab's firls function.
class FilterLeastSquaresFIR : public Filter
{
    public:

    FilterLeastSquaresFIR(
        const float64 & sample_rate,
        uint32 kernel_size,
        const Buffer & freq_axis,
        const Buffer & amplitude_axis,
        const float64 & beta = 5.0);

    FilterLeastSquaresFIR(const FilterLeastSquaresFIR & copy);

    virtual ~FilterLeastSquaresFIR();

    Buffer
    getKernel() const;

    void
    setKernel(const Buffer & k);

    Buffer
    getKernelFrequencies();

    Buffer
    getKernelAmplitudes();

    AudioStream
    filter(const AudioStream & x);

    // ! This filter type does not support dynamic frequency response.
    AudioStream
    filter(const AudioStream & x, const float64 & frequency)
    { return filter(x); };

    // ! This filter type does not support dynamic frequency response.
    AudioStream
    filter(const AudioStream & x, const Buffer & frequencies)
    { return filter(x); };

    Buffer
    filter(const Buffer & x);

    // ! This filter type does not support dynamic frequency response.
    Buffer
    filter(const Buffer & x, const float64 & frequency)
    { return filter(x); };

    // ! This filter type does not support dynamic frequency response.
    Buffer
    filter(const Buffer & x, const Buffer & frequencies)
    { return filter(x); };

    virtual
    float64
    filter(const float64 & x);

    // ! This filter type does not support dynamic frequency response.
    float64
    filter(const float64 & x, const float64 & frequency_Hz)
    { return filter(x); };

    ///////////////////////////////////////////////////////////////////////////////
    Buffer
    getImpulseResponse()
    {return Filter::getImpulseResponse();};

    FilterLeastSquaresFIR &
    operator=(const FilterLeastSquaresFIR & rhs);

    void
    plot(boolean show_fc = true, boolean show_phase = false);

    //! Resets interal history buffer and sets the cut off frequency to the one
    //! used at declaration.
    void
    reset();

    void
    setWindow(WindowType type);

    void
    makeKernel(
        const Buffer & freq_axis,
        const Buffer & amplitude_axis);

    protected:

    float64 * b_;
    float64 * window_;

    float64 * x_history_;
    float64 * x_ptr_;
    float64 * x_end_ptr_;

    Buffer * f_axis_;
    Buffer * a_axis_;

}; // LowPassFilter

}; // Nsound

// :mode=c++: jEdit modeline
#endif
