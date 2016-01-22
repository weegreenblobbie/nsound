//-----------------------------------------------------------------------------
//
//  $Id: DrumKickBass.h 874 2014-09-08 02:21:29Z weegreenblobbie $
//
//  Nsound is a C++ library and Python module for audio synthesis featuring
//  dynamic digital filters. Nsound lets you easily shape waveforms and write
//  to disk or plot them. Nsound aims to be as powerful as Csound but easy to
//  use.
//
//  Copyright (c) 2006-Present Nick Hilton
//
//  Simulates a bass kick drum.  Based on a Csound drum.
//
//  source: http://www.adp-gmbh.ch/csound/instruments/base_drum01.html
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

#ifndef _NSOUND_DRUM_KICK_BASS_H_
#define _NSOUND_DRUM_KICK_BASS_H_

#include <Nsound/Instrument.h>

namespace Nsound
{

// Forward Declarations
class AudioStream;
class Buffer;

//-----------------------------------------------------------------------------
//!  Class Drum
class DrumKickBass : public Instrument
{
    public:

    //! Creates a Bass Kick Drum.
    DrumKickBass(
        const float64 & sample_rate,
        const float64 & high_frequency,
        const float64 &  low_frequency);

    //! Destructor
    ~DrumKickBass();

    //! Plays a demo for this instrument.
    AudioStream play();

    //! Plays a static note for this instrument.
    AudioStream play(const float64 & duration, const float64 & frequency);

    //! Static play method.
    AudioStream play(
        const float64 & duration,
        const float64 & high_frequency,
        const float64 &  low_frequency);

    //! Dynamic play method.
    AudioStream play(
        const float64 & duation,
        const Buffer  & high_frequency,
        const Buffer  &  low_frequency);

    //! Nsound::DrumKickBass information.
    std::string getInfo()
    {
        return
            "Nsound::DrumKickBass by Nick Hilton on 2008-10-14\n"
            "Simulates a bass kick drum.  Based on a Csound drum.\n"
            "source: http://www.adp-gmbh.ch/csound/instruments/base_drum01.html\n\n"
            "sr	=  48000\n"
            "ksmps	=  32\n"
            "nchnls	=  2\n"
            "\n"
            "instr 1\n"
            "  i_len = p3+0.2\n"
            "\n"
            "  icps = 51\n"
            "  iamp = 1\n"
            "\n"
            "  kcps	expon 1, 0.022, 0.5\n"
            "  kcps	=  4.3333 * kcps * icps + icps\n"
            "\n"
            "  a1	phasor kcps\n"
            "  a2	phasor kcps, 0.5\n"
            "  a1	=  a1 - a2\n"
            "\n"
            "  kffrq1  expon 1, 0.07, 0.5\n"
            "  kffrq2	expon 1, 0.01, 0.5\n"
            "  kffrq	=  (kffrq1 + kffrq2) * kcps\n"
            "\n"
            "  a1	pareq a1, kffrq, 0, 0.7071, 2\n"
            "\n"
            "  a1	=  taninv(a1 * 20)\n"
            "\n"
            "  a1	pareq a1, kcps * 6, 2, 1, 2\n"
            "  a1	pareq a1, icps * 1.25, 2.5, 1, 0\n"
            "\n"
            "  a2	linseg 1, i_len, 1, 0.01, 0, 1, 0\n"
            "  a1	=  a1 * a2 * iamp * 4500 + (1/1e24)\n"
            "\n"
            "  outs a1,a1\n"
            "endin\n";
    };

    protected:

    float64 hi_f_;
    float64 lo_f_;

};

};

// :mode=c++: jEdit modeline
#endif
