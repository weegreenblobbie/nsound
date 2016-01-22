//-----------------------------------------------------------------------------
//
//  $Id: Mixer.cc 874 2014-09-08 02:21:29Z weegreenblobbie $
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
#include <Nsound/Buffer.h>
#include <Nsound/Mixer.h>
#include <Nsound/MixerNode.h>
#include <Nsound/Nsound.h>

#include <iostream>
#include <sstream>
#include <string.h>

using std::cout;
using std::endl;
using std::flush;

using namespace Nsound;

//-----------------------------------------------------------------------------
Mixer::
Mixer()
    : max_channels_(0),
      mixer_set_()
{}

//-----------------------------------------------------------------------------
Mixer::
~Mixer()
{}

void
Mixer::
add(
    float64 first_beat_time,
    float64 beats_per_minute,
    const AudioStream & audio_stream)
{
    M_ASSERT_VALUE(first_beat_time, >=, 0.0);

    if(beats_per_minute < 0.0)
    {
        beats_per_minute = 0.0;
    }

    MixerNode new_node = MixerNode(
        first_beat_time, beats_per_minute, audio_stream);

    mixer_set_.insert(new_node);

    if(audio_stream.getNChannels() > max_channels_)
    {
        max_channels_ = audio_stream.getNChannels();
    }
}

void
Mixer::
clear()
{
   mixer_set_.erase(mixer_set_.begin(), mixer_set_.end());
}

AudioStream
Mixer::
getStream(float64 end_time)
{
    return getStream(0,end_time);
}

//-----------------------------------------------------------------------------
//
//  Some definitions:
//
//  BPM = beats per minute
//
//  Absolute Mixer view:
//
//  ........[___________]........[___________]........[___________]....
//  ^       ^                    ^                    ^
//  time 0  beat                 next beat            next beat ...
//
//
//  Showing start and end times:
//
//  ........[___________]........[___________]........[___________]....
//        ^                                                          ^
//        start                                                      end
//
//  Prduces and AudioStream:
//
//        ..[___________]........[___________]........[___________]...
//        ^                                                          ^
//        start                                                      end
//
AudioStream
Mixer::
getStream(float64 start_time, float64 end_time)
{
    M_ASSERT_VALUE(mixer_set_.size(), >, 0);
    M_ASSERT_VALUE(start_time, <, end_time);

    if(mixer_set_.size() == 0) return AudioStream(1,1);
    if(start_time >= end_time) return AudioStream(1,1);

    MixerSet::const_iterator node = mixer_set_.begin();

    // Grab the sample rate
    float64 sample_rate = node->audio_stream_->getSampleRate();

    // Create the new stream to return.  By default it is 2 channels and
    // has the same sample_rate as the first AudioStream.

    AudioStream new_stream(sample_rate, max_channels_);

    // Fill the stream with zeros.
    uint32 n_samples = static_cast<uint32>(
        sample_rate * (end_time - start_time));

    for(uint32 i = 0; i < n_samples; ++i)
    {
        new_stream << 0.0;
    }

    // Loop through the list of MixerNodes.  On each node, loop from start
    // time to end time and add the node's AudioStream data to new_stream.

    while(node != mixer_set_.end())
    {
        // Beats per minute is a frequency, but we want to calculate the
        // the time between beats.  Remember that f = 1/t, so to get time
        // we invert the equation, t = 1/f.  Here, we also need to multiply
        // by 60 to get beats per second.
        //
        // Therefore, t = 60/f

        float64 beat_time = 60.0 / node->bpm_;

        float64 beat_length =
            static_cast<float64>(node->audio_stream_->getLength())
            / static_cast<float64>(sample_rate);

        // We also need to keep track of the next_beat_time and end_time.
        float64 next_beat_time = node->first_beat_time_;
        float64 next_end_time = next_beat_time + beat_length;

        // initialize some indexes
        uint32 new_stream_offset_index = 0;
        uint32 node_max_index = node->audio_stream_->getLength();

        float64 mixer_time = start_time;

        // Go to the first beat that ends after current time.
        while(next_end_time < mixer_time)
        {
            next_beat_time += beat_time;
            next_end_time = next_beat_time + beat_length;
        }

        float64 new_stream_time = next_beat_time - start_time;
        float64 new_stream_end_time = end_time - start_time;

        // Case 1:  Start time is before next beat.
        //  ....[________].....[________].....[________]..
        //    ^
        //    start

        if(mixer_time < next_beat_time)
        {
            mixer_time = next_beat_time;
        }

        // Case 2:  Start time is inside next beat.
        //  ....[________].....[________].....[________]..
        //          ^
        //          start

        else if(next_beat_time < mixer_time)
        {
            // Add the remaing AudioStream to the output.

            uint32 node_index = static_cast<uint32>((next_end_time - mixer_time) * sample_rate);

            new_stream_offset_index = static_cast<uint32>(
                (mixer_time - start_time) * sample_rate);

            AudioStream temp = node->audio_stream_->substream(node_index);

            new_stream.add(temp, new_stream_offset_index);

            next_beat_time += beat_time;
            next_end_time = next_beat_time + beat_length;
            mixer_time = next_beat_time;
            new_stream_time = next_beat_time - start_time;
        }

        while(new_stream_time < new_stream_end_time)
        {
            new_stream_offset_index = static_cast<uint32>
                (new_stream_time * sample_rate);

            // add into new_stream
            uint32 add_n_samples = node_max_index;

            if(next_end_time > end_time)
            {
                add_n_samples -= static_cast<uint32>((next_end_time - end_time)
                    * static_cast<float64>(sample_rate));
            }

            new_stream.add(
                *node->audio_stream_,
                new_stream_offset_index,
                add_n_samples);

            next_beat_time += beat_time;
            next_end_time = next_beat_time + beat_length;
            mixer_time = next_beat_time;
            new_stream_time += beat_time;
        }

        // Move to the next MixerNode in the list.
        ++node;

    } // while(node != NULL)

    return new_stream;
}
