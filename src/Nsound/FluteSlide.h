//-----------------------------------------------------------------------------
//
//  $Id: FluteSlide.h 874 2014-09-08 02:21:29Z weegreenblobbie $
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

#ifndef _NSOUND_FLUTE_SLIDE_H_
#define _NSOUND_FLUTE_SLIDE_H_

#include <Nsound/Instrument.h>

namespace Nsound
{

// Forward Declarations
class AudioStream;
class Buffer;

//-----------------------------------------------------------------------------
//!  Class Drum
class FluteSlide : public Instrument
{
    public:

    //! Creates a Bass Kick Drum.
    FluteSlide(const float64 & sample_rate);

    //! Destructor
    ~FluteSlide();

    //! Plays a demo for this instrument.
    AudioStream play();

    //! Plays a static note for this instrument.
    AudioStream play(const float64 & duration, const float64 & frequency);

    //! Plays a static note for this instrument.
    AudioStream play(
        const float64 & duration,
        const float64 & frequency,
        const float64 & presure,
        const float64 & breath);

    //! Nsound::FluteSlide information.
    std::string getInfo()
    {
        return
            "Nsound::FluteSlide by Nick Hilton\n\n"

            "Based on physical models written in Csound by Hans Mikelson.\n"
            "Originally based on Perry Cook's physical model.\n\n"

            ";PERRY COOK'S SLIDE FLUTE\n"
            "\n"
            "          instr     3\n"
            "\n"
            "aflute1 init   0\n"
            "ifqc    =           cpspch(p5)\n"
            "ipress  =           p6\n"
            "ibreath =           p7\n"
            "\n"
            "; FLOW SETUP\n"
            "kenv1   linseg      0, .1, ipress, p3-.2, ipress, .1, 0\n"
            "kenv2   linseg      0, .01, 1, p3-.02, 1, .01, 0\n"
            "\n"
            "; THE VALUES MUST BE AROUND -1 AND 1 OR THE CUBIC WILL BLOW UP.\n"
            "aflow1    rand      1\n"
            "aflow1    =         aflow1 * kenv1\n"
            "\n"
            "; .0356 CAN BE USED TO ADJUST THE BREATH LEVEL.\n"
            "asum1     =         ibreath*aflow1 + kenv1\n"
            "asum2     =         asum1 + aflute1*.4\n"
            "\n"
            "; EMBOUCHURE DELAY SHOULD BE 1/2 THE BORE DELAY\n"
            "\n"
            "ax        delay     asum2, 1/ifqc/2 - 15/sr\n"
            "\n"
            "apoly     =         ax - ax*ax*ax\n"
            "asum3     =         apoly + aflute1*.4\n"
            "\n"
            "avalue    tone      asum3, 2000\n"
            "\n"
            "; BORE THE BORE LENGTH DETERMINES PITCH.  SHORTER IS HIGHER.\n"
            "aflute1 delay  avalue, 1/ifqc - 15/sr\n"
            "\n"
            "          out  avalue*p4*kenv2\n"
            "\n"
            "          endin;\n";
    };

};

};

// :mode=c++: jEdit modeline
#endif
