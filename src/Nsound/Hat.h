//-----------------------------------------------------------------------------
//
//  $Id: Hat.h 874 2014-09-08 02:21:29Z weegreenblobbie $
//
//  Nsound is a C++ library and Python module for audio synthesis featuring
//  dynamic digital filters. Nsound lets you easily shape waveforms and write
//  to disk or plot them. Nsound aims to be as powerful as Csound but easy to
//  use.
//
//  Copyright (c) 2009-Present Nick Hilton
//
//  Simulates a Hat Hit.  Based on a Csound Hat by Steven Cook.
//
//  source: http://www.csounds.com/cook/
//
//  Sound Generators: 808HiHat.orc 808HiHat.sco
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

#include <Nsound/FilterHighPassIIR.h>
#include <Nsound/Instrument.h>
#include <Nsound/Square.h>

namespace Nsound
{

// Forward Declarations
class AudioStream;
class Buffer;

//-----------------------------------------------------------------------------
//!  Class Hat
//
//! Simulates a Hat Hit.  Based on a Csound Hat by Steven Cook.
//!
//! source: http://www.csounds.com/cook/
//!
//! Sound Generators: 808HiHat.orc 808HiHat.sco
//
class Hat : public Instrument
{
    public:

    //! Creates a Hat
    Hat(const float64 & sample_rate);

    //! Plays a demo for this instrument.
    AudioStream
    play() override;

    //! Static play method with option of Hat open or closed.
    AudioStream
    play(
        const float64 & duration,
        const float64 & tune) override
    { return play(duration, tune, true); };

    //! Static play method with option of Hat open or closed.
    AudioStream
    play(
        const float64 & duration,
        const float64 & tune,
        boolean is_closed);

    //! Nsound::Hat information.
    std::string getInfo() const override
    {
        return
            "Nsound::Hat by Nick Hilton on 2009-11-13\n"
            "Simulates a Hat Hit.  Based on a Csound Hat by Steven Cook.\n"
            "source: http://www.csounds.com/cook/\n";
    };

    protected:

    Square            square_;
    FilterHighPassIIR hpf_;

};

};

// :mode=c++: jEdit modeline
