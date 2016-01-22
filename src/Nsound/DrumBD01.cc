//-----------------------------------------------------------------------------
//
//  $Id: DrumBD01.cc 874 2014-09-08 02:21:29Z weegreenblobbie $
//
//  Copyright (c) 2009-Present Nick Hilton
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
#include <Nsound/DrumBD01.h>
#include <Nsound/FilterLowPassIIR.h>
#include <Nsound/FilterTone.h>
#include <Nsound/Sine.h>

using namespace Nsound;

//-----------------------------------------------------------------------------
DrumBD01::
DrumBD01(const float64 & sample_rate)
    :
    Instrument(sample_rate){}

//-----------------------------------------------------------------------------
DrumBD01::
~DrumBD01(){}

AudioStream
DrumBD01::
play()
{
    AudioStream y(sample_rate_, 1);

    y << play(0.5, 50.0, 0.25)
      << play(0.5, 50.0, 0.50)
      << play(0.5, 50.0, 0.75)
      << play(0.5, 50.0, 1.0);

    return y;
}

AudioStream
DrumBD01::
play(
    const float64 & duration,
    const float64 & frequency)
{
    return play(duration, frequency, 1.0);
}


AudioStream
DrumBD01::
play(
    const float64 & duration,
    const float64 & frequency,
    const float64 & velocity)
{
    float64 v = velocity;

    if(v > 1.0) v = 1.0;
    if(v < 0.0) v = 0.0;

    float64 fdec = 0.025 * (v * v + 0.1072);

    // Create a sine generator
    Sine sin(sample_rate_);

    Buffer kcps1 = sin.drawDecay(10.0*fdec, 7.0)
                << sin.drawLine(duration - fdec, 0.0, 0.0);

    Buffer kcps2 = sin.drawDecay(10.0*0.32, 7.0)
                << sin.drawLine(duration - 0.32, 0.0, 0.0);

    Buffer kcps = (4.3333 * kcps1 + kcps2) * frequency;

    Buffer klpf = sin.drawDecay(10.0*0.04, 7.0)
               << sin.drawLine(duration - 0.04, 0.0, 0.0);

    Buffer klpf2 = 8.0 * sin.drawDecay(10.0*0.0005, 7.0)
                << sin.drawLine(duration - 0.0005, 0.0, 0.0);

    Buffer kamp = 1.0
                - 0.5*sin.drawDecay(0.15, 7.0)
               << sin.drawLine(duration - 0.15, 1.0, 1.0);

    Buffer a1 = sin.generate(duration, kcps);

    a1.limit(-0.25, 0.25);

    Buffer freq = kcps * (0.5 + klpf2);

    FilterTone tone(sample_rate_, freq[0]);

    a1 = tone.filter(a1, freq);

    Buffer a2 = sin.whiteNoise(duration) * sin.whiteNoise(duration);

    tone.makeKernel(500.0);

    a2 -= tone.filter(a2);

    freq = 4.0 * (klpf + klpf2) * kcps;

    FilterLowPassIIR lpf(sample_rate_, 2, freq[0], 0.01);

    a1 = lpf.filter(kamp * a1, freq);

    a2 = lpf.filter(kamp * a2, kcps);
    a2 *= 5.0 * klpf.getSqrt();

    a1 += a2 * 1.0;

    a1.normalize();

    Buffer env = sin.drawLine(duration - 0.01, 1.0, 1.0)
              << sin.drawLine(0.005, 1.0, 0.0)
              << sin.drawLine(0.005, 0.0, 0.0);

    AudioStream as(sample_rate_, 1);

    as << env * a1;

    return as;
}

// :mode=c++: jEdit modeline
