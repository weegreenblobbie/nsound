//-----------------------------------------------------------------------------
//
//  $Id: OrganPipe.h 874 2014-09-08 02:21:29Z weegreenblobbie $
//
//  Nsound is a C++ library and Python module for audio synthesis featuring
//  dynamic digital filters. Nsound lets you easily shape waveforms and write
//  to disk or plot them. Nsound aims to be as powerful as Csound but easy to
//  use.
//
//  Copyright (c) 2009-Present Nick Hilton
//
//  Simulates a Pipe Organ.  Based on a Csound Pipe Organ by Hons Mikelson.
//
//  source: http://www.csounds.com/mikelson/
//
//  "A Csound Halloween"  hallown.orc hallown.sco
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

#ifndef _NSOUND_ORGAN_PIPE_H_
#define _NSOUND_ORGAN_PIPE_H_

#include <Nsound/Instrument.h>

namespace Nsound
{

// Forward Declarations
class AudioStream;
class Buffer;

//-----------------------------------------------------------------------------
//!  Class OrganPipe
//
//! Based on a Csound Pipe Organ by Hons Mikelson.
//!
//! source: http://www.csounds.com/mikelson/
//!
//! "A Csound Halloween"  hallown.orc hallown.sco
//
class OrganPipe : public Instrument
{
    public:

    //! Creates an OrganPipe
    OrganPipe(const float64 & sample_rate);

    //! Destructor
    ~OrganPipe();

    //! Plays a demo for this instrument.
    AudioStream play();

    //! Plays a static note for this instrument.
    AudioStream play(const float64 & duration, const float64 & frequency);

    //! Static play method.
    AudioStream play(
        const float64 & duration,
        const float64 & frequency,
        const float64 & pan);

    //! Nsound::OrganPipe information.
    std::string getInfo()
    {
        return
            "Nsound::OrganPipe by Nick Hilton on 2009-10-25\n"
            "Simulates a Pipe Organ.  Based on a Csound Pipe Organ.\n"
            "source: http://www.csounds.com/mikelson/hallown.orc\n";
    };

};

};

// :mode=c++: jEdit modeline
#endif
