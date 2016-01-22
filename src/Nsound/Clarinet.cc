//-----------------------------------------------------------------------------
//
//  $Id: Clarinet.cc 874 2014-09-08 02:21:29Z weegreenblobbie $
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
#include <Nsound/Clarinet.h>
#include <Nsound/DelayLine.h>
#include <Nsound/FilterHighPassIIR.h>
#include <Nsound/FilterLowPassIIR.h>
#include <Nsound/FilterTone.h>
#include <Nsound/Sine.h>

using namespace Nsound;

//-----------------------------------------------------------------------------
Clarinet::
Clarinet(const float64 & sample_rate)
    :
    Instrument(sample_rate)
{
}

//-----------------------------------------------------------------------------
Clarinet::
~Clarinet()
{
}

AudioStream
Clarinet::
play()
{
    AudioStream y(sample_rate_, 1);

    //     duration,    freq
    y << play(0.600, 261.616)
      << play(0.600, 293.656)
      << play(0.600, 329.609)
      << play(0.600, 349.218)
      << play(0.600, 391.973)
      << play(1.800, 445.000);

    return y;
}

AudioStream
Clarinet::
play(const float64 & duration, const float64 & frequency)
{
    return play(duration, frequency, 1.8, 1000.0, 0.2);
}

AudioStream
Clarinet::
play(
    const float64 & duration,
    const float64 & frequency,
    const float64 & pressure,
    const float64 & filter,
    const float64 & reed_stiffness)
{
    DelayLine  delay1(sample_rate_, (1.0 / frequency));
    DelayLine  delay2(sample_rate_, (1.0 / frequency));
    Sine       sin(sample_rate_);
    FilterLowPassIIR   tone(sample_rate_, 2, filter, 0.0);
    FilterHighPassIIR atone(sample_rate_, 2, filter, 0.0);

    Buffer reed_wave = sin.drawLine( 80.0 / 256.0,  1.0,  1.0)
                    << sin.drawLine(156.0 / 256.0,  1.0, -1.0)
                    << sin.drawLine( 20.0 / 256.0, -1.0, -1.0);

    float64 env_peak = (0.55 + 0.3 * pressure);

    Buffer env1 = sin.drawLine(0.005, 0.0, env_peak)
               << sin.drawLine(duration - 0.015, env_peak, env_peak)
               << sin.drawLine(0.01, env_peak, 0.0);

    Buffer vibrato_env = sin.drawLine(duration, 0.0, 0.1);

    Buffer pressurem = vibrato_env * sin.generate(duration, 5.0) + 1.0;

    pressurem *= (0.3 * env1);

    float64 reed_bell = 0.0;

    uint32 n_samples = pressurem.getLength();

    AudioStream y(sample_rate_, 1);

    Buffer xxx;

    for(uint32 n = 0; n < n_samples; ++n)
    {
        float64 filter_out = tone.filter(reed_bell);

        float64 x = -1.0 * pressurem[n] - 0.95 * filter_out - reed_stiffness;

        xxx << x;

        uint32 reed_index = static_cast<uint32>((0.25 * x + 0.5) * sample_rate_);

        x *= reed_wave[reed_index];

        x += pressurem[n];

        delay2.write(x);
        reed_bell = delay2.read();

        x = atone.filter(reed_bell);

        y << x;

    }

    y.normalize();

    float64 dt = 1.0 / frequency;

    // Declick, at the very beginning and ends, the reed_wave values goes crazy

    y = y.substream(dt, duration - dt);

    Buffer out_env = sin.drawLine(0.01, 0.0, 1.0)
                  << sin.drawLine(duration - 0.02, 1.0, 1.0)
                  << sin.drawLine(0.01, 1.0, 0.0);

    y *= out_env;

    return y;
}


// :mode=c++: jEdit modeline
