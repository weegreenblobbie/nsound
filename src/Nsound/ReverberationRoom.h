//-----------------------------------------------------------------------------
//
//  $Id: ReverberationRoom.h 874 2014-09-08 02:21:29Z weegreenblobbie $
//
//  Nsound is a C++ library and Python module for audio synthesis featuring
//  dynamic digital filters. Nsound lets you easily shape waveforms and write
//  to disk or plot them. Nsound aims to be as powerful as Csound but easy to
//  use.
//
//  Copyright (c) 2008 to Present Nick Hilton
//
//  weegreenblobbie_yahoo_com (replace '_' with '@' and '.')
//
//  Based on Freeverb by Jezar at Dreampoint
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
#ifndef _NSOUND_REVERBERATION_ROOM_H_
#define _NSOUND_REVERBERATION_ROOM_H_

#include <Nsound/Nsound.h>

namespace Nsound
{

class AudioStream;
class Buffer;
class FilterAllPass;
class FilterCombLowPassFeedback;

//-----------------------------------------------------------------------------
class ReverberationRoom
{
    public:

    //! Default Constructor
    //
    //! sample_rate:   the sample rate
    //! room_feedback: 0.0 to 1.0, how quickly the room dampens the input
    //! wet_percent:   0.0 to 1.0, how much of the filtered input is added to the output
    //! dry_percent:   0.0 to 1.0, how much of the original input is added to the output
    //! low_pass_frequency_Hz: 0.0 to 1/2 sample rate, attenuate frequencies in the feedback
    //! stereo_spread_seconds: >= 0.0, how much delay between the Left and Right channels
    //
    ReverberationRoom(
        const float64 & sample_rate,
        const float64 & room_feedback,
        const float64 & wet_percent = 0.5,
        const float64 & dry_percent = 1.0,
        const float64 & low_pass_frequency_Hz = 7644.9,
        const float64 & stereo_spread_seconds = 0.0005215);

    ReverberationRoom(const ReverberationRoom & copy);

    virtual ~ReverberationRoom();

    AudioStream
    filter(const AudioStream & x);

    AudioStream
    filter(const Buffer & x);

    void
    filter(
        float64 & out_left,
        float64 & out_right,
        const float64 & in_left,
        const float64 & in_right);

    float64
    getSampleRate() const { return sample_rate_; };

    ReverberationRoom &
    operator=(const ReverberationRoom & rhs);

    void
    reset();

    protected:

    float64 sample_rate_;

    float64 wet_percent_;
    float64 dry_percent_;

    FilterCombLowPassFeedback ** comb_left_;
    FilterCombLowPassFeedback ** comb_right_;

    FilterAllPass ** allpass_left_;
    FilterAllPass ** allpass_right_;

    static const float64 ROOM_FEEDBACK_SCALE_;  //  = 0.28;
    static const float64 ROOM_FEEDBACK_OFFSET_; // = 0.7;

    static const uint32 N_COMB_FILTERS_     = 8;
    static const uint32 N_ALL_PASS_FILTERS_ = 4;

    static const float64 COMB_DELAY_TIME_SECONDS_[N_COMB_FILTERS_];

    static const float64 ALL_PASS_DELAY_TIME_SECONDS_[N_ALL_PASS_FILTERS_];

};

} // namespace

// :mode=c++: jEdit modeline
#endif
