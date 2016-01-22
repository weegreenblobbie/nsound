//-----------------------------------------------------------------------------
//
//  $Id: Pulse.h 875 2014-09-27 22:25:13Z weegreenblobbie $
//
//  Nsound is a C++ library and Python module for audio synthesis featuring
//  dynamic digital filters. Nsound lets you easily shape waveforms and write
//  to disk or plot them. Nsound aims to be as powerful as Csound but easy to
//  use.
//
//  Copyright (c) 2010-Present Nick Hilton
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
#ifndef _NSOUND_PULSE_H_
#define _NSOUND_PULSE_H_

#include <Nsound/Generator.h>

namespace Nsound
{

class Buffer;

//-----------------------------------------------------------------------------
//! A Pulse Generator.
//
//! This class produces square pules at the given frequency for the given
//! pulse width duration.  The pulse width can be specified in either
//! miliseconds or percentage of the wavelength.
//!
class Pulse : public Generator
{
    public:

    enum PulseUnits
    {
        PULSE_SECONDS,
        PULSE_PERCENT, // percentage of wavelength
    };

    //! Creates a new Pulse Generator.
    //
    //! \param sample_rate - the numer of samples per second to generate
    //! \param pulse_width - either a time in msec or a percentage
    //! \param untis - the units that the pulse_width are in (PULSE_SECONDS or
    //!                PULSE_PERCENT)
    Pulse(
        const float64 & sample_rate,
        const float64 & pulse_width,
        const PulseUnits & units = PULSE_PERCENT);

    ~Pulse();

    //! Sets the PulseUnits to units for this Pulse Generator.
    //
    //! \param untis - the units that the pulse width is in (PULSE_SECONDS or
    //!                PULSE_PERCENT)
    void
    setUnits(const PulseUnits & units)
    {units_ = units;};

    //! This is a real-time method for the Pulse generator.
    //
    //! \param frequency \f$\left(f\right)\f$ the frequency in Hz
    //! \return float64
    //!
    float64
    generate(const float64 & frequency);

    //! This is a real-time method for the Pulse generator.
    //
    //! \param frequency \f$\left(f\right)\f$ the frequency in Hz
    //! \param pulse_width \f$\left(pw\right)\f$ the pulse width in PulseUnits
    //! \return float64
    //!
    //! WARNING: the phase term is ignored!
    float64
    generate2(
        const float64 & frequency,
        const float64 & pulse_width);

    using Generator::generate;

    Buffer
    generate2(
        const float64 & duration,
        const float64 & frequency,
        const float64 & pulse_width);

    //! This does nothing for now.
    Buffer
    generate(
        const float64 & duration,
        const float64 & frequency,
        const float64 & pulse_width){return Buffer();};

    //! This does nothing for now.
    Buffer
    generate(
        const float64 & duration,
        const float64 & frequency,
        const Buffer &  pulse_widths){return Buffer();};

    //! This does nothing for now.
    Buffer
    generate(
        const float64 & duration,
        const Buffer &  frequencies,
        const Buffer &  pulse_widths){return Buffer();};

    //! This does nothing for now.
    Buffer
    generate2(
        const float64 & duration,
        const float64 & frequency,
        const Buffer &  pulse_width){return Buffer();};

    //! This does nothing for now.
    Buffer
    generate2(
        const float64 & duration,
        const Buffer &  frequency,
        const float64 & pulse_width){return Buffer();};

    //! This does nothing for now.
    Buffer
    generate2(
        const float64 & duration,
        const Buffer &  frequency,
        const Buffer &  pulse_width){return Buffer();};

    void reset();

    float64 riseTime()
    {return rise_t_;};

    float64 fallTime()
    {return fall_t_;};

    protected:

    float64    rise_t_;      //! The rising edge time for the pulse.
    float64    fall_t_;      //! The falling edge time for the pulse.
    float64    pulse_width_; //! The default pulse width to use.
    PulseUnits units_;       //! The units the pulse width is in.

};

};

#endif

// :mode=c++: jEdit modeline
