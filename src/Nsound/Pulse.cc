//-----------------------------------------------------------------------------
//
//  $Id: Pulse.cc 874 2014-09-08 02:21:29Z weegreenblobbie $
//
//  Copyright (c) 2004-2006 Nick Hilton
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

#include <Nsound/Buffer.h>
#include <Nsound/Pulse.h>

#include <cmath>
#include <iostream>

using namespace Nsound;

using std::cout;
using std::cerr;
using std::endl;

Pulse::
Pulse(
    const float64 & sample_rate,
    const float64 & pulse_width,
    const PulseUnits & units)
    :
    Generator(sample_rate),
    rise_t_(0.0),
    fall_t_(-1.0),
    pulse_width_(pulse_width),
    units_(units)
{
}

Pulse::
~Pulse()
{}

float64
Pulse::
generate(const float64 & frequency)
{
    return generate2(frequency, pulse_width_);
}

float64
Pulse::
generate2(const float64 & frequency, const float64 & pulse_width)
{
    float64 sample = 0.0;

    if(units_ == PULSE_SECONDS)
    {
        fall_t_ = rise_t_ + pulse_width;
    }
    else
    {
        fall_t_ = rise_t_ + (pulse_width / frequency);
    }

    if(t_ >= rise_t_)
    {
        if(t_ < fall_t_)
        {
            sample = 1.0;
        }
        else
        {
            rise_t_ += 1.0 / frequency;
        }
    }

    t_ += sample_time_;

    return sample;
}

Buffer
Pulse::
generate2(
    const float64 & duration,
    const float64 & frequency,
    const float64 & pulse_width)
{
    return Generator::generate2(duration, frequency, pulse_width);
}

void
Pulse::
reset()
{
    t_ = 0.0;
    rise_t_ = 0.0;
    fall_t_ = -1.0;
}

