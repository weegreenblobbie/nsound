//////////////////////////////////////////////////////////////////////////////
//
//  $Id: EnvelopeAdsr.cc 833 2014-05-01 02:41:25Z weegreenblobbie $
//
//  Copyright (c) 2005-2006 Nick Hilton
//
//  weegreenblobbie_yahoo_com (replace '_' with '@' and '.')
//
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
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
//////////////////////////////////////////////////////////////////////////////

#include <Nsound/AudioStream.h>
#include <Nsound/Buffer.h>
#include <Nsound/EnvelopeAdsr.h>
#include <Nsound/Generator.h>

#include <iostream>

using namespace Nsound;

EnvelopeAdsr::
EnvelopeAdsr(
    const float64 & sample_rate,
    const float64 & attack_time,
    const float64 & delay_time,
    const float64 & sustain_amplitude,
    const float64 & release_time)
    :
    sample_rate_(sample_rate),
    attack_slope_(0.0),
    attack_time_(attack_time),
    delay_slope_(0.0),
    delay_time_(delay_time),
    sustain_amp_(sustain_amplitude),
    release_slope_(0.0),
    release_time_(release_time),
    scale_(0.0),
    mode_(attacking)
{
    M_ASSERT_VALUE(sample_rate, >, 0.0);
    M_ASSERT_VALUE(attack_time, >=, 0.0);
    M_ASSERT_VALUE(delay_time, >=, 0.0);
    M_ASSERT_VALUE(sustain_amplitude, >, 0.0);
    M_ASSERT_VALUE(sustain_amplitude, <=, 1.0);
    M_ASSERT_VALUE(release_time, >=, 0.0);

    M_ASSERT_VALUE(attack_time + delay_time + release_time, >, 0.0);

    setAttackTime(attack_time);
    setDelayTime(delay_time);
    setReleaseTime(release_time);
}

void
EnvelopeAdsr::
reset()
{
    mode_ = attacking;
    scale_ = 0.0;
}

void
EnvelopeAdsr::
setAttackTime(const float64 & time)
{
    M_ASSERT_VALUE(time, >=, 0.0);

    uint32 n_samples = static_cast<uint32>(time * sample_rate_ + 0.5);

    n_samples -= 1;

    M_ASSERT_VALUE(n_samples, >, 0);

    attack_slope_ = 1.0 / static_cast<float64>(n_samples);
    attack_time_ = time;
}

void
EnvelopeAdsr::
setDelayTime(const float64 & time)
{
    M_ASSERT_VALUE(time, >=, 0.0);

    uint32 n_samples = static_cast<uint32>(time * sample_rate_ + 0.5);

    n_samples -= 1;

    M_ASSERT_VALUE(n_samples, >, 0);

    delay_slope_ = -1.0 / static_cast<float64>(n_samples);
    delay_time_ = time;
}

void
EnvelopeAdsr::
setSustainAmplitude(const float64 & amp)
{
    M_ASSERT_VALUE(amp, >, 0.0);

    sustain_amp_ = amp;
}

void
EnvelopeAdsr::
setReleaseTime(const float64 & time)
{
    M_ASSERT_VALUE(time, >=, 0.0);

    uint32 n_samples = static_cast<uint32>(time * sample_rate_ + 0.5);

    n_samples -= 1;

    M_ASSERT_VALUE(n_samples, >, 0);

    release_slope_ = -1.0 / static_cast<float64>(n_samples);
    release_time_ = time;
}

AudioStream
EnvelopeAdsr::
shape(const AudioStream & as)
{
    AudioStream out(as.getSampleRate(), as.getNChannels());

    for(uint32 i = 0; i < as.getNChannels(); ++i)
    {
        out[i] = shape(as[i]);
    }

    return out;
}

Buffer
EnvelopeAdsr::
shape(const Buffer & buf)
{
    float64 duration = static_cast<float64>(buf.getLength()) / sample_rate_;

    float64 sustain_time =
        duration - attack_time_ - delay_time_ - release_time_;

    if(sustain_time < 0) sustain_time = 0;

    uint32 sustain_samples = static_cast<uint32>(sustain_time * sample_rate_);

    mode_ = attacking;

    Buffer::const_iterator itor = buf.begin();
    Buffer::const_iterator end = buf.end();

    Buffer out;

    // Attack

    while(mode_ != sustaining && itor != end)
    {
        out << shape(*itor, true);
        ++itor;
    }

    // Sustain

    uint32 count = 0;

    while(count < sustain_samples && itor != end)
    {
        out << shape(*itor, true);
        ++itor;
        ++count;
    }

    // Relase

    while(!is_done() && itor != end)
    {
        out << shape(*itor, false);
        ++itor;
    }

    return out;
}

float64
EnvelopeAdsr::
shape(float64 sample, bool key_on)
{
    switch(mode_)
    {
        case attacking:
        {
            scale_ += attack_slope_;
            if(scale_ >= 1.0)
            {
                scale_ = 0.999;
                mode_ = delaying;
            }
            if(!key_on) mode_ = releasing;

            break;
        }

        case delaying:
        {
            scale_ += delay_slope_;
            if(scale_ <= sustain_amp_) mode_ = sustaining;
            if(!key_on) mode_ = releasing;

            break;
        }

        case sustaining:
        {
            scale_ = sustain_amp_;
            if(!key_on) mode_ = releasing;

            break;
        }

        case releasing:
        {
            scale_ += release_slope_;

            if(scale_ <= 0.0)
            {
                scale_ = 0.0;
                mode_ = done;
            }

            break;
        }

        case done:
        {
            scale_ = 0.0;
        }
    }

    M_ASSERT_VALUE(scale_, <, 1.0);

    return sample * scale_;
}

