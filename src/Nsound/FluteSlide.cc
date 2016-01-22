//-----------------------------------------------------------------------------
//
//  $Id: FluteSlide.cc 874 2014-09-08 02:21:29Z weegreenblobbie $
//
//  Copyright (c) 2004-2008 Nick Hilton
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
#include <Nsound/DelayLine.h>
#include <Nsound/FilterTone.h>
#include <Nsound/FluteSlide.h>
#include <Nsound/Sine.h>

using namespace Nsound;

//-----------------------------------------------------------------------------
FluteSlide::
FluteSlide(const float64 & sample_rate)
    :
    Instrument(sample_rate)
{
}

//-----------------------------------------------------------------------------
FluteSlide::
~FluteSlide()
{
}

AudioStream
FluteSlide::
play()
{
    AudioStream y(sample_rate_, 1);

    //     duration,    freq
    y << play(0.600, 261.616)
      << play(0.600, 293.656)
      << play(0.600, 329.609)
      << play(0.600, 349.218)
      << play(0.600, 391.973)
      << play(0.600, 445.000);

    return y;
}

AudioStream
FluteSlide::
play(const float64 & duration, const float64 & frequency)
{
    return play(duration, frequency, 1.0, 0.05);
}

AudioStream
FluteSlide::
play(
    const float64 & duration,
    const float64 & frequency,
    const float64 & pressure,
    const float64 & breath)
{
    Sine sine(sample_rate_);

    float64 n_delay_seconds = 1.0 / frequency;

    DelayLine delay1(sample_rate_, n_delay_seconds / 2.0);
    DelayLine delay2(sample_rate_, n_delay_seconds);

    FilterTone tone(sample_rate_, 2000.0);


    Buffer env1 =  sine.drawLine(0.1, 0.0, pressure)
                << sine.drawLine(duration - 0.2, pressure, pressure)
                << sine.drawLine(0.1, pressure, 0.0);

    Buffer env2 =  sine.drawLine(0.01, 0.0, 1.0)
                << sine.drawLine(duration - 0.02, 1.0, 1.0)
                << sine.drawLine(0.01, 1.0, 0.0) << 0.0;

    Buffer flow = env1 * sine.whiteNoise(duration);

    Buffer input = breath * flow + env1;

    float64 flute = 0.0;

    uint32 n_samples = input.getLength();

    AudioStream y(sample_rate_, 1);

    for(uint32 n = 0; n < n_samples; ++n)
    {
        float64 x = input[n] + flute * 0.35;

        delay1.write(x);

        x = delay1.read();

        x -= x * x * x;

        x += flute * 0.4;

        x = tone.filter(x);

        y << x;

        delay2.write(x);

        flute = delay2.read();
    }

    y *= env2 * duration;

    y.normalize();

    return y;
}


// :mode=c++: jEdit modeline
