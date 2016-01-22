//-----------------------------------------------------------------------------
//
//  $Id: FilterIIR.h 874 2014-09-08 02:21:29Z weegreenblobbie $
//
//  Nsound is a C++ library and Python module for audio synthesis featuring
//  dynamic digital filters. Nsound lets you easily shape waveforms and write
//  to disk or plot them. Nsound aims to be as powerful as Csound but easy to
//  use.
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
#ifndef _NSOUND_FILTER_IIR_H_
#define _NSOUND_FILTER_IIR_H_

#include <Nsound/Filter.h>

#include <set>

namespace Nsound
{

// Forward class declarations
class AudioStream;
class Buffer;
class Kernel;
class RngTausworthe;

//-----------------------------------------------------------------------------
//! WARNING: This is Experimental, you should not use this class as it may not
//! be working or will change in future releases of Nsound.
class FilterIIR : public Filter
{
    public:

    FilterIIR(
        const float64 & sample_rate,
        uint32 n_poles);

    FilterIIR(const FilterIIR & copy);

    virtual ~FilterIIR();

    //! Designs a filter kernel using a genetic algorithm that trys to match the provided frequency response.
    //
    //! Designs a filter kernel that trys to match the provide frequency
    //! response.  The method uses a genetic algorithm to arrive at a fit
    //! solution and retuns the error per generation.  The error signal is nice
    //! to plot to get an idea of how quickly the solution was found.
    Buffer
    designFrequencyResponse(
        const Buffer &  frequency_response,
        const float64 & max_error = 0.01,
        const int32     max_iterations = 1000);

    //! Designs a filter kernel using a genetic algorithm that trys to match the provided impulse response.
    //
    //! Designs a filter kernel that trys to match the provide impulse response
    //! The method uses a genetic algorithm to arrive at a fit solution and
    //! retuns the error per generation.  The error signal is nice to plot to
    //! get an idea of how quickly the solution was found.
    Buffer
    designImpulseResponse(
        const Buffer &  impulse_response,
        const float64 & max_error = 0.01,
        const int32     max_iterations = 1000);

    AudioStream
    filter(const AudioStream & x)
    { return Filter::filter(x);};

    AudioStream
    filter(const AudioStream & x, const Buffer & frequencies)
    { return Filter::filter(x);};

    Buffer
    filter(const Buffer & x)
    {return Filter::filter(x);};

    Buffer
    filter(const Buffer & x, const Buffer & frequencies)
    {return filter(x);};

    float64
    filter(const float64 & x);

    float64
    filter(const float64 & x, const float64 & frequency);

    uint32
    getKernelSize() const {return n_poles_;};

    Buffer
    getImpulseResponse(const uint32 n_samples = 8192)
    { reset(); return Filter::getImpulseResponse(n_samples); };

    FilterIIR &
    operator=(const FilterIIR & rhs);

    #ifndef SWIG
    //! Prints the coeffents.
    friend
    std::ostream &
    operator<<(std::ostream & out,const FilterIIR & rhs);
    #endif

    void
    reset();

    protected:

    enum SignalType
    {
        FREQUENCY_RESSPONSE = 0,
        IMPULSE_RESPONSE
    };

    //! Designs a filter kernel that trys to match the provide frequency response.
    //
    //! Designs a filter kernel that trys to match the provide frequency
    //! response.  The method uses a genetic algorithm to arrive at a fit
    //! solution and retuns the error per generation.  The error signal is nice
    //! to plot to get an idea of how quickly the solution was found.
    Buffer
    designKernel(
        const Buffer &   response,
        const float64 &  max_rms_error,
        const int32      max_iterations,
        const SignalType type);

    float64
    getRMS(
        const Kernel &   kernel,
        const Buffer &   response,
        const SignalType type);

    void
    savePlot(
        const Kernel &  k,
        const Buffer &  response,
        uint32          n,
        const float64 & error);

    uint32 n_poles_;

    Kernel * kernel_;

    float64 * x_history_;
    float64 * x_ptr_;
    float64 * x_end_ptr_;

    float64 * y_history_;
    float64 * y_ptr_;
    float64 * y_end_ptr_;

    RngTausworthe * rng_;

};

std::ostream &
operator<<(std::ostream & out,const FilterIIR & rhs);

};

// :mode=c++: jEdit modeline

#endif
