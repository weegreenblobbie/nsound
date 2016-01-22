//-----------------------------------------------------------------------------
//
//  $Id: MixerNode.cc 874 2014-09-08 02:21:29Z weegreenblobbie $
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

#include <Nsound/AudioStream.h>
#include <Nsound/MixerNode.h>

#include <iostream>
#include <sstream>
#include <string.h>

using namespace Nsound;

uint32 MixerNode::id_count_ = 0;

///////////////////////////////////////////////////////////////////////////
//
// Constructor
//
///////////////////////////////////////////////////////////////////////////
Nsound::MixerNode::
MixerNode(float64 time, float64 bpm, const AudioStream & as)
    : audio_stream_(&as),
      bpm_(bpm),
      first_beat_time_(time),
      id_(MixerNode::id_count_++)
{}

boolean
Nsound::MixerNode::
operator==(const MixerNode & rhs) const
{
    if(id_ == rhs.id_
        && bpm_ == rhs.bpm_
        && first_beat_time_ == rhs.first_beat_time_)
    {
        return true;
    }

    return false;
}

boolean
Nsound::MixerNode::
operator<(const MixerNode & rhs) const
{
    if(first_beat_time_ < rhs.first_beat_time_
        || id_ < rhs.id_)
    {
        return true;
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////
// print()
//
// This method returns a string that represents the node.
//
///////////////////////////////////////////////////////////////////////////
std::string
Nsound::MixerNode::
printNode()
{
    using std::endl;

    std::stringstream out;

    out << "first_beat_time_: "
        << first_beat_time_
        << endl
        << "bpm_: "
        << bpm_
        << endl
        << "audio_stream_ length: "
        << static_cast<float64>(audio_stream_->getLength())
                / static_cast<float64>(audio_stream_->getSampleRate())
        << " seconds"
        << endl
        << endl;

    return out.str();
}
