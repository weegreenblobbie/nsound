//-----------------------------------------------------------------------------
//
//  $Id: GuitarBass.h 874 2014-09-08 02:21:29Z weegreenblobbie $
//
//  Nsound is a C++ library and Python module for audio synthesis featuring
//  dynamic digital filters. Nsound lets you easily shape waveforms and write
//  to disk or plot them. Nsound aims to be as powerful as Csound but easy to
//  use.
//
//  Copyright (c) 2009-Present Nick Hilton
//
//  weegreenblobbie_yahoo_com (replace '_' with '@' and '.')
//
// Based on physical models written in Csound by Hans Mikelson.
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

#ifndef _NSOUND_GUITAR_BASS_H_
#define _NSOUND_GUITAR_BASS_H_

#include <Nsound/Instrument.h>

namespace Nsound
{

// Forward Declarations
class AudioStream;
class Buffer;

//-----------------------------------------------------------------------------
//!  Class Drum
class GuitarBass : public Instrument
{
    public:

    //! Creates a Bass Kick Drum.
    GuitarBass(const float64 & sample_rate);

    //! Destructor
    ~GuitarBass();

    //! Plays a demo for this instrument.
    AudioStream play();

    //! Plays a static note for this instrument.
    AudioStream play(const float64 & duration, const float64 & frequency);

    //! Nsound::GuitarBass information.
    std::string getInfo()
    {
        return
            "Nsound::GuitarBass by Nick Hilton\n\n"

            "Based on physical models written in Csound by Hans Mikelson.\n\n"

            ";BASS PHYSICAL MODEL\n"
            "          instr     2\n"
            "\n"
            "; INITIALIZATIONS\n"
            "ifqc      =         cpspch(p5)\n"
            "kcount  init   0\n"
            "abody1  init   0\n"
            "abody2  init   0\n"
            "adline  init   0\n"
            "\n"
            "; ENVELOPES\n"
            ";**********\n"
            "; FILTER ENVELOPE\n"
            "kenvflt linseg      0, .01, 1, p3-.11, 1, .1, 0\n"
            "; THIS ENVELOPE LOADS THE STRING WITH A TRIANGLE WAVE.\n"
            "kenvstr linseg      0, 1/ifqc/8, -p4/2, 1/ifqc/4, p4/2, 1/ifqc/8, 0, p3-1/ifqc, 0\n"
            "; THIS ENVELOPE IS FOR THE BODY RESONANCE\n"
            "kenvres linseg      0, .1, 1, p3-.1, 1\n"
            "\n"
            "; DELAY LINE WITH FILTERED FEEDBACK\n"
            "afiltr    tone      adline, ifqc*ifqc/10\n"
            "adline    delay     afiltr + kenvstr, 1/ifqc\n"
            "\n"
            "; RESONANCE OF THE BODY\n"
            "abody1    reson     afiltr, 160, 40\n"
            "abody2    reson     afiltr, 80, 20\n"
            "\n"
            "aout      =         afiltr + .0001*kenvres*abody1 + .00004*kenvres*abody2\n"
            "          out  aout * kenvflt\n"
            "          endin\n";
    };

};

};

// :mode=c++: jEdit modeline
#endif
