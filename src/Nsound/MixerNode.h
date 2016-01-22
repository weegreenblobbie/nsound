//-----------------------------------------------------------------------------
//
//  $Id: MixerNode.h 874 2014-09-08 02:21:29Z weegreenblobbie $
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

#ifndef _NSOUND_MIXER_NODE_H_
#define _NSOUND_MIXER_NODE_H_

#include <Nsound/Nsound.h>

#include <iostream>
#include <string>

namespace Nsound
{

class AudioStream;

///////////////////////////////////////////////////////////////////////////
//
//! This class holds the nodes for use with the Mixer class.
//
//! This class holds the nodes for use with the Mixer class.
//
///////////////////////////////////////////////////////////////////////////

class MixerNode
{
    public:

    ///////////////////////////////////////////////////////////////////
    //
    //! Constructor
    //
    //! Constructor
    //
    ///////////////////////////////////////////////////////////////////
    MixerNode(float64 first_beat_time,
              float64 bpm,
              const AudioStream & as);

    boolean operator==(const MixerNode &) const;
    boolean operator<(const MixerNode &) const;

    ///////////////////////////////////////////////////////////////////
    // print()
    //
    //! This method returns a string that represents the node.
    //
    //! This method returns a string that represents the node.
    //
    ///////////////////////////////////////////////////////////////////
    std::string printNode();

    //! The pointer to the AudioStream.
    const AudioStream * audio_stream_;

    //! The number of beats per minute.
    float64 bpm_;

    //! The time the AudioStream first occurs in the list.
    float64 first_beat_time_;

    uint32 id_;

    static uint32 id_count_;

}; // MixerNode

}; // Nsound

#endif
