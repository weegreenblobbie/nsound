//-----------------------------------------------------------------------------
//
//  $Id: Mixer.h 874 2014-09-08 02:21:29Z weegreenblobbie $
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

#ifndef _NSOUND_MIXER_H_
#define _NSOUND_MIXER_H_

#include <Nsound/Nsound.h>

#include <iostream>
#include <set>
#include <string>

namespace Nsound
{

// Forward declaration
class AudioStream;
class MixerNode;

typedef std::set<MixerNode> MixerSet;
//typedef std::set<MixerNode>::iterator MixerIterator;
//typedef std::set<MixerNode>::const_iterator ConstMixerIterator;

//-----------------------------------------------------------------------------
//
//! This class enables easy scheduling and mixing of multiple AudioStreams.
//
//! This class can que up AudioStreams using beats per minute and first
//! beat time.
//
//-----------------------------------------------------------------------------

class Mixer
{
    public:

    //
    //! Constructor
    //
    //! Constructor
    //
    Mixer();

    //
    //! Destuctor
    //
    //! Destuctor
    //
    ~Mixer();

    // add()
    //
    //! This method inserts the AudioStream to the Mixer's LinkList.
    //
    //! This method inserts the AudioStream into the Mixer's LinkList.
    //!
    //! \param first_beat_time - The time this AudioStream starts
    //!                          playing in the Mixer, must be >= zero.
    //!
    //! \param beats_per_minute - The number of times this AudioStream
    //!                           will play per minute.  If this is
    //!                           <= zero, the AudioStream will play
    //!                           only once.
    //!
    //! \param audio_stream - The AudioStream object to be stored.
    //
    void add(float64 first_beat_time,
             float64 beats_per_minute,
             const AudioStream & audio_stream);

    //
    // getStream()
    //
    //! This method returns one AudioStream composed of all AudioStreams stored in the Mixer's LinkList.
    //
    //! This method returns one AudioStream composed of all
    //! AudioStreams stored in the Mixer's LinkList.
    //
    AudioStream getStream(float64 end_time);
    AudioStream getStream(float64 start_time, float64 end_time);

    //
    // clear()
    //
    //! This method removes all streams from the mixer.
    //
    void clear();

    //
    // size()
    //
    //! This method returns the number of stream stored in it.
    //
    uint32 size() const { return static_cast<uint32>(mixer_set_.size()); };

    private:

    //! Stores the maximum number of channels.
    Nsound::uint32 max_channels_;
    //! This stores all the MixerNodes.
    Nsound::MixerSet mixer_set_;


}; // Mixer

}; // Nsound

#endif
