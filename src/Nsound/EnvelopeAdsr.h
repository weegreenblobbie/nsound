//-----------------------------------------------------------------------------
//
//  $Id: EnvelopeAdsr.h 874 2014-09-08 02:21:29Z weegreenblobbie $
//
//  Nsound is a C++ library and Python module for audio synthesis featuring
//  dynamic digital filters. Nsound lets you easily shape waveforms and write
//  to disk or plot them. Nsound aims to be as powerful as Csound but easy to
//  use.
//
//  Copyright (c) 2005-2006 Nick Hilton
//
//  weegreenblobbie_yahoo_com (replace '_' with '@' and '.')
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

//-----------------------------------------------------------------------------
//
//  This class will generate an Attack Delay Sustain Release (ADSR) envlepe.
//
//  1.0 |        /|
//      |       /  |
//      |      /    |
//      |     /      |
//      |    /        |-----sustain-amplitude------|
//      |   /                                       |
//      |  /                                         |
//      | /                                           |
//      |/                                             |
//  0.0 ------------------------------------------------
//      |--------|
//        Attack |------|
//                Delay |--------------------------|
//                         Sustain                 |-----|
//                                                   Release
//
//  Currently the curves are linear.
//
//-----------------------------------------------------------------------------

#ifndef _NSOUND_ENVELOPE_ADSR_H_
#define _NSOUND_ENVELOPE_ADSR_H_

namespace Nsound
{

// Forward declarations
class AudioStream;
class Buffer;

//-----------------------------------------------------------------------------
class EnvelopeAdsr
{
    public:

    //! Constructor
    EnvelopeAdsr(
        const float64 & sample_rate,
        const float64 & attack_time,
        const float64 & delay_time,
        const float64 & sustain_amplitude,
        const float64 & release_time);

    void reset();

	void setAttackTime(const float64 & time);
    void setDelayTime(const float64 & time);
    void setSustainAmplitude(const float64 & amp);
    void setReleaseTime(const float64 & time);

    //! Shapes the AudioStream inplace.
    AudioStream shape(const AudioStream & as);

    //! Shapes the Buffer inplace.
    Buffer shape(const Buffer & buf);

    //! Realtime interface.
    float64 shape(float64 sample, bool key_on);
    bool    is_done() const { return mode_ == done; }

	protected:

	float64  sample_rate_;
	float64  attack_slope_;
	float64  attack_time_;
	float64  delay_slope_;
	float64  delay_time_;
	float64  sustain_amp_;
	float64  release_slope_;
	float64  release_time_;
	float64  scale_;

	enum Mode { attacking, delaying, sustaining, releasing, done };

	Mode mode_;

};

};

#endif
