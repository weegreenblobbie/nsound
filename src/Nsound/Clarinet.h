//-----------------------------------------------------------------------------
//
//  $Id: Clarinet.h 874 2014-09-08 02:21:29Z weegreenblobbie $
//
//  Nsound is a C++ library and Python module for audio synthesis featuring
//  dynamic digital filters. Nsound lets you easily shape waveforms and write
//  to disk or plot them. Nsound aims to be as powerful as Csound but easy to
//  use.
//
//  Copyright (c) 2009-Present Nick Hilton
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

#ifndef _NSOUND_CLARINET_H_
#define _NSOUND_CLARINET_H_

#include <Nsound/Instrument.h>

namespace Nsound
{

// Forward Declarations
class AudioStream;
class Buffer;

//-----------------------------------------------------------------------------
//!  Class Drum
class Clarinet : public Instrument
{
    public:

    //! Creates a Bass Kick Drum.
    Clarinet(const float64 & sample_rate);

    //! Destructor
    ~Clarinet();

    //! Plays a demo for this instrument.
    AudioStream play();

    //! Plays a static note for this instrument.
    AudioStream play(const float64 & duration, const float64 & frequency);

    //! Plays a static note for this instrument.
    AudioStream play(
        const float64 & duration,
        const float64 & frequency,
        const float64 & presure,
        const float64 & filter,
        const float64 & reed_stiffness);

    //! Nsound::Clarinet information.
    std::string getInfo()
    {
        return
            "Nsound::Clarinet by Nick Hilton\n\n"

            "Based on physical models written in Csound by Hans Mikelson.\n"
            "Originally based on Perry Cook's physical model.\n\n"

            ";PERRY COOK'S CLARINET\n"
            "\n"
            "          instr     5\n"
            "asum2   init   0\n"
            "abellreed init      0\n"
            "aout    init   0\n"
            "kenv1   linseg      0, .005, .55 + .3*p4, p3 - .015, .55 + .3*p4, .01, 0\n"
            "\n"
            "; REED SECTION\n"
            "apressm =           kenv1\n"
            "arefilt tone   abellreed, 1500\n"
            "asum2     =         (-.95)*arefilt - apressm\n"
            "areedtab tablei asum2/4+.34, 1, 1, .5\n"
            "\n"
            "abellreed delayr 1/p5/2\n"
            "          delayw    aout\n"
            "aout delayr 1/p5/2\n"
            "          delayw    apressm + areedtab*asum2\n"
            "\n"
            "          out  aout*10000\n"
            "          endin;\n";
    };

};

};

// :mode=c++: jEdit modeline
#endif
