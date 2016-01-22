//-----------------------------------------------------------------------------////
//
//  $Id: FilterDC.cc 874 2014-09-08 02:21:29Z weegreenblobbie $
//
//  Copyright (c) 2007 Nick Hilton
//
//  weegreenblobbie_yahoo_com (replace '_' with '@' and '.')
//
//-----------------------------------------------------------------------------////

//-----------------------------------------------------------------------------////
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
//-----------------------------------------------------------------------------////

#include <Nsound/AudioStream.h>
#include <Nsound/Buffer.h>
#include <Nsound/FilterDC.h>

using namespace Nsound;

//-----------------------------------------------------------------------------
FilterDC::
FilterDC(const float64 & feedback_gain)
    :
    feedback_gain_(feedback_gain),
    last_input_(0.0),
    last_output_(0.0)
{
}

AudioStream
FilterDC::
filter(const AudioStream & x)
{
    uint32 n_channels = x.getNChannels();

    AudioStream y(x.getSampleRate(), n_channels);

    for(uint32 channel = 0; channel < n_channels; ++channel)
    {
        y[channel] = filter(x[channel]);
    }

    return y;
}


Buffer
FilterDC::
filter(const Buffer & x)
{
    uint32 n_samples = x.getLength();

    Buffer y;

    for(uint32 n = 0; n < n_samples; ++n)
    {
        y << filter(x[n]);
    }

    return y;
}

float64
FilterDC::
filter(const float64 & x)
{
    float64 out = x - last_input_ + feedback_gain_ * last_output_;

    last_input_ = x;
    last_output_ = out;

    return out;
}

