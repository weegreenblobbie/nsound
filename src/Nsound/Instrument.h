//-----------------------------------------------------------------------------
//
//  $Id: Instrument.h 874 2014-09-08 02:21:29Z weegreenblobbie $
//
//  Nsound is a C++ library and Python module for audio synthesis featuring
//  dynamic digital filters. Nsound lets you easily shape waveforms and write
//  to disk or plot them. Nsound aims to be as powerful as Csound but easy to
//  use.
//
//  Copyright (c) 2004, 2005 Nick Hilton
//
//  weegreenblobbie_at_yahoo_com
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

#ifndef _NSOUND_INSTRUMENT_H_
#define _NSOUND_INSTRUMENT_H_

#include <Nsound/Nsound.h>

#include <string>

namespace Nsound
{

// Forward declarations

class AudioStream;

//-----------------------------------------------------------------------------
//! The Nsound Instrument baseclass.  All Nsound instruments extend this class.
class Instrument
{
    public:

    Instrument(const float64 & sample_rate):sample_rate_(sample_rate){};

    virtual
    ~Instrument(){};

    //! Plays a demo for this instrument.
    virtual
    AudioStream
    play() = 0;

    //! Plays a static note for this instrument.
    virtual
    AudioStream
    play(const float64 & duration, const float64 & frequency) = 0;

    //! Returns information about who wrote this instrument and how to use it.
    virtual
    std::string
    getInfo() = 0;

    protected:

    float64 sample_rate_;
};

};

// :mode=c++: jEdit modeline
#endif
