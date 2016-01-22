//-----------------------------------------------------------------------------
//
//  $Id: DrumBD01.h 874 2014-09-08 02:21:29Z weegreenblobbie $
//
//  Nsound is a C++ library and Python module for audio synthesis featuring
//  dynamic digital filters. Nsound lets you easily shape waveforms and write
//  to disk or plot them. Nsound aims to be as powerful as Csound but easy to
//  use.
//
//  Copyright (c) 2009-Present Nick Hilton
//
//  Simulates a bass drum.  Based on a Csound drum.
//
//  source: http://www.csounds.com/istvan/html/drums.html
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

#ifndef _NSOUND_DRUM_BD01_H_
#define _NSOUND_DRUM_BD01_H_

#include <Nsound/Instrument.h>

namespace Nsound
{

// Forward Declarations
class AudioStream;
class Buffer;

//-----------------------------------------------------------------------------
//!  Class DrumBD01
class DrumBD01 : public Instrument
{
    public:

    //! Creates a Drum
    DrumBD01(const float64 & sample_rate);

    //! Destructor
    ~DrumBD01();

    //! Plays a demo for this instrument.
    AudioStream play();

    //! Plays a static note for this instrument.
    AudioStream play(const float64 & duration, const float64 & frequency);

    //! Static play method.
    AudioStream play(
        const float64 & duration,
        const float64 & frequency,
        const float64 & velocity);

    //! Nsound::DrumBD01 information.
    std::string getInfo()
    {
        return
            "Nsound::DrumBD01 by Nick Hilton on 2009-05-02\n"
            "Simulates a bass drum.  Based on a Csound drum.\n"
            "source: http://www.csounds.com/istvan/html/drums.html\n";
    };

};

};

// :mode=c++: jEdit modeline
#endif
