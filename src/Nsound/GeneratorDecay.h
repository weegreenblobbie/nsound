//-----------------------------------------------------------------------------
//
//  $Id: GeneratorDecay.h 874 2014-09-08 02:21:29Z weegreenblobbie $
//
//  Nsound is a C++ library and Python module for audio synthesis featuring
//  dynamic digital filters. Nsound lets you easily shape waveforms and write
//  to disk or plot them. Nsound aims to be as powerful as Csound but easy to
//  use.
//
//  Copyright (c) 2005-Present Nick Hilton
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

#include <Nsound/Nsound.h>
#include <Nsound/Generator.h>
#include <Nsound/WindowType.h>

namespace Nsound
{

// Forwared declarion of Buffer
class Buffer;
class RandomNumberGeneratorDecay;

//-----------------------------------------------------------------------------
//  GeneratorDecay Class
//! A class tha generates decay curves.
class GeneratorDecay : public Generator
{
    public:

    //! Creates a generator with the specified sample rate.
    GeneratorDecay(const float64 & sample_rate);

    //! This is a real-time method for the decay curve ossilator.
    //
    //! \param frequency \f$\left(f\right)\f$ the frequency in Hz
    //! \return float64
    //!
    //! This is a special case of
    //! generate2(const float64 & frequency, const float64 & alpha)
    //! with alpha = 1.0.
    float64
    generate(const float64 & frequency) override;

    // Generator's Buffer generate() overloads, which the one above would
    // hide.
    using Generator::generate;

    //! This is a real-time method for the wavetable oscillator.
    //
    //! \param frequency \f$\left(f\right)\f$ the frequency in Hz
    //! \param alpha \f$\left(\varphi\right)\f$ the decay constant
    //! \return float64
    //!
    float64
    generate2(const float64 & frequency, const float64 & alpha) override;

    //! This method oscillates a decay curve with specified alpha.
    //
    //! \param duration the number of seconds to oscillate the waveform
    //! \param frequency \f$\left(f\right)\f$ the frequency in Hz
    //! \param alpha \f$\left(\varalpha\right)\f$ the decay constant
    //! \return Buffer
    //!
    //! \par Example:
    //! \code
    //! // C++
    //! GeneratorDecay gen(48000);
    //! Buffer b = gen.generate2(1.0, 3.0, 6);
    //!
    //! // Python
    //! gen = GeneratorDecay(48000)
    //! b = gen.generate(1.0, 3.0, 6)
    //! \endcode
    Buffer
    generate2(
        const float64 & duration,
        const float64 & frequency,
        const float64 & alpha) override;

    //! This method oscillates decay curve with specified alpha.
    //
    //! \param duration the number of seconds to oscillate the waveform
    //! \param frequency \f$\left(f\right)\f$ the frequency in Hz
    //! \param alpha \f$\left(\varalpha\right)\f$ the decay constant
    //! \return Buffer
    //!
    //! This is a special case of
    //! generate2(const float64 & duration, const Buffer & frequencies, const Buffer & phase)
    //! with constant frequency.
    Buffer
    generate2(
        const float64 & duration,
        const float64 & frequency,
        const Buffer &  alpha) override;

    //! This method oscillates decay curve with specified alpha.
    //
    //! \param duration the number of seconds to oscillate the waveform
    //! \param frequency \f$\left(f\right)\f$ the frequency in Hz
    //! \param alpha \f$\left(\varalpha\right)\f$ the decay constant
    //! \return Buffer
    //!
    //! This is a special case of
    //! generate2(const float64 & duration, const Buffer & frequencies, const Buffer & phase)
    //! with constant phase.
    Buffer
    generate2(
        const float64 & duration,
        const Buffer &  frequencies,
        const float64 & alpha) override;

    //! This method oscillates the waveform stored in this generator.
    //
    //! \param duration the number of seconds to oscillate the waveform
    //! \param frequency \f$\left(f\right)\f$ the frequency in Hz
    //! \param phase \f$\left(\varphi\right)\f$ the natural phase (0.0 to 1.0)
    //! \return Buffer
    Buffer
    generate2(
        const float64 & duration,
        const Buffer &  frequencies,
        const Buffer &  alpha) override;

    protected:

    //! DOXME
    GeneratorDecay();

};

}; // Nsound

// :mode=c++: jEdit modeline
