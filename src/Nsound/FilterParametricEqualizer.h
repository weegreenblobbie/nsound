//-----------------------------------------------------------------------------
//
//  $Id: FilterParametricEqualizer.h 874 2014-09-08 02:21:29Z weegreenblobbie $
//
//  Nsound is a C++ library and Python module for audio synthesis featuring
//  dynamic digital filters. Nsound lets you easily shape waveforms and write
//  to disk or plot them. Nsound aims to be as powerful as Csound but easy to
//  use.
//
//  Copyright (c) 2008 Nick Hilton
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
#ifndef _NSOUND_FILTER_PARAMETRIC_EQUALIZER_H_
#define _NSOUND_FILTER_PARAMETRIC_EQUALIZER_H_

#include <Nsound/Filter.h>
#include <Nsound/Kernel.h>

namespace Nsound
{

// Forward class declarations
class AudioStream;
class Buffer;

//-----------------------------------------------------------------------------
//
//! Implementation of Zoelzer's parametric equalizer filters, with some modifications by the author.
//
//! Below are the formulas for the 3 different types of parametric equalizer filters.
//!
//! f   =   Frequency in Hz
//! sr  =   Sample Rate in samples per second
//! V   =   Percent boost (1.0 = no boost, < 1.0 = cut signal, > 1.0 boost signal)
//! Q   =   Resonance or Q of the filter, sqrt(1/2) == no resonance
//!
//! omega = 2*pi*f/sr
//! K     = tan(omega/2)
//!
//! a0    = 1 + sqrt(2*V)*K + V*K^2
//! a1    = 2*(V*K^2 - 1)
//! a2    = 1 - sqrt(2*V)*K + V*K^2
//!
//! b0    = 1 + K/Q + K^2
//! b1    = 2*(K^2 - 1)
//! b2    = 1 - K/Q + K^2
//!
//!
//! The formula for the High Shelf filter is:
//!
//!
//! omega = 2*pi*f/sr
//! K     = tan((pi-omega)/2)
//!
//! a0    = 1 + sqrt(2*V)*K + V*K^2
//! a1    = -2*(V*K^2 - 1)
//! a1    = 1 - sqrt(2*V)*K + V*K^2
//!
//! b0    = 1 + K/Q + K^2
//! b1    = -2*(K^2 - 1)
//! b2    = 1 - K/Q + K^2
//!
//!
//! The formula for the Peaking filter is:
//!
//!
//! omega = 2*pi*f/sr
//! K     = tan(omega/2)
//!
//! a0 =  1 + V*K/2 + K^2
//! a1 =  2*(K^2 - 1)
//! a2 =  1 - V*K/2 + K^2
//!
//! b0 =  1 + K/Q + K^2
//! b1 =  2*(K^2 - 1)
//! b2 =  1 - K/Q + K^2
//
class FilterParametricEqualizer : public Filter
{
    public:

    enum Type
    {
        PEAKING,
        LOW_SHELF,
        HIGH_SHELF
    };

    //! boost is in dB
    FilterParametricEqualizer(
        const Type & type,
        const float64 & sample_rate,
        const float64 & frequency,
        const float64 & resonance = 0.707106781187,
        const float64 & boost_dB = 0.0);

    virtual ~FilterParametricEqualizer();

    AudioStream
    filter(const AudioStream & x);

    AudioStream
    filter(const AudioStream & x, const Buffer & frequencies);

    AudioStream
    filter(
        const AudioStream & x,
        const Buffer & frequencies,
        const Buffer & resonance);

    //! boost is in dB
    AudioStream
    filter(
        const AudioStream & x,
        const Buffer & frequencies,
        const Buffer & resonance,
        const Buffer & boost_dB);

    Buffer
    filter(const Buffer & x);

    Buffer
    filter(const Buffer & x, const Buffer & frequencies);

    Buffer
    filter(
        const Buffer & x,
        const Buffer & frequencies,
        const Buffer & resonance);

    //! boost is in dB
    Buffer
    filter(
        const Buffer & x,
        const Buffer & frequencies,
        const Buffer & resonance,
        const Buffer & boost_dB);

    float64
    filter(const float64 & x);

    float64
    filter(const float64 & x, const float64 & frequency);

    //! boost is in dB
    float64
    filter(
        const float64 & x,
        const float64 & frequency,
        const float64 & resonance);

    //! boost is in dB
    float64
    filter(
        const float64 & x,
        const float64 & frequency,
        const float64 & resonance,
        const float64 & boost_dB);

    void
    makeKernel(
        const float64 & frequency,
        const float64 & resonance,
        const float64 & boost);

    void
    plot(
        boolean show_fc = true,
        boolean show_phase = false);

    void
    reset();

    protected:

    Type    type_;
    float64 frequency_;
    float64 resonance_;
    float64 boost_;

    float64 * a_;
    float64 * b_;

    float64 * x_history_;
    float64 * x_ptr_;
    float64 * x_end_ptr_;

    float64 * y_history_;
    float64 * y_ptr_;
    float64 * y_end_ptr_;

    KernelCache kernel_cache_;

    private:

    FilterParametricEqualizer(const FilterParametricEqualizer & copy);

    FilterParametricEqualizer &
    operator=(const FilterParametricEqualizer & rhs);

};

};

// :mode=c++:  jEdit modeline
#endif
