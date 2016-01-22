//////////////////////////////////////////////////////////////////////////////
//
//  $Id: Granulator.h 416 2009-12-03 05:56:54Z weegreenblobbie $
//
//  Nsound is a C++ library and Python module for audio synthesis featuring
//  dynamic digital filters. Nsound lets you easily shape waveforms and write
//  to disk or plot them. Nsound aims to be as powerful as Csound but easy to
//  use.
//
//  Copyright (c) 2005-2007 Nick Hilton
//
//  weegreenblobbie_yahoo_com (replace '_' with '@' and '.')
//
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
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
//////////////////////////////////////////////////////////////////////////////
#ifndef _NSOUND_GRANULATOR_H_
#define _NSOUND_GRANULATOR_H_

#include <Nsound/Nsound.h>

namespace Nsound
{

// Forwared declarion of Nsound::Buffer
class Buffer;
class Generator;
class Sine;

//////////////////////////////////////////////////////////////////////////////
//  Granulator Class
//
// DOXME
//////////////////////////////////////////////////////////////////////////////
class Granulator
{
    public:

    enum EnvelopeType_e
    {
        CUSTOM,
        GAUSSIAN,
        GAUSSIAN_90,
        GAUSSIAN_70,
        GAUSSIAN_50,
        GAUSSIAN_30,
        GAUSSIAN_10,
        DECAY,
        REVERSE_DECAY
    };

    //////////////////////////////////////////////////////////////////////////
    //  Constructor
    //
    //! Creates a generator with the specified sample rate.
    //
    //////////////////////////////////////////////////////////////////////////
    Granulator(
        const float64  & sample_rate,
        EnvelopeType_e   env_type,
        const float64  & envelope_noise = 0.0,
        const Buffer   * custom_envelope = NULL);

    //////////////////////////////////////////////////////////////////////////
    // DOXME
    //////////////////////////////////////////////////////////////////////////
    Granulator(const Nsound::Granulator & gran);

    //////////////////////////////////////////////////////////////////////////
    // DOXME
    //////////////////////////////////////////////////////////////////////////
    virtual ~Granulator();

    //////////////////////////////////////////////////////////////////////////
    //  generate()
    //
    // DOXME
    //////////////////////////////////////////////////////////////////////////
    Nsound::Buffer
    generate(
        const float64 & duration,
        const float64 & grain_frequency,
        const float64 & waves_per_grain,
        const float64 & grains_per_second);

    //////////////////////////////////////////////////////////////////////////
    //  generate()
    //
    // DOXME
    //////////////////////////////////////////////////////////////////////////
    Nsound::Buffer
    generate(
        const float64 & duration,
        const Nsound::Buffer & grain_frequency,
        const Nsound::Buffer & waves_per_grain,
        const Nsound::Buffer & grains_per_second) const;

    //////////////////////////////////////////////////////////////////////////
    //! Assignment operator.
    //////////////////////////////////////////////////////////////////////////
    Nsound::Granulator &
    operator=(const Nsound::Granulator & rhs);

    protected:
    Granulator();

    float64 sample_rate_;

    Generator * envelope_generator_;

};//Granulators

}; // Nsound

// :mode=c++:
#endif
