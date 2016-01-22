//-----------------------------------------------------------------------------
//
//  $Id: OrganPipe.cc 874 2014-09-08 02:21:29Z weegreenblobbie $
//
//  Nsound is a C++ library and Python module for audio synthesis featuring
//  dynamic digital filters. Nsound lets you easily shape waveforms and write
//  to disk or plot them. Nsound aims to be as powerful as Csound but easy to
//  use.
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
#include <Nsound/OrganPipe.h>
#include <Nsound/FilterLowPassIIR.h>
#include <Nsound/FilterTone.h>
#include <Nsound/Sine.h>

using namespace Nsound;
using std::cout;
using std::endl;

//-----------------------------------------------------------------------------
OrganPipe::
OrganPipe(const float64 & sample_rate)
    :
    Instrument(sample_rate){}

//-----------------------------------------------------------------------------
OrganPipe::
~OrganPipe(){}

AudioStream
OrganPipe::
play()
{
    AudioStream y(sample_rate_, 2);

    // Organ Intro to Bach's Toccata & Fugue in Dminor
    //     Start  Dur  Amp    Fqc   Pan  OutCh1  OutCh2
    // i4  26     .12  200    7.09  .9   1       2
    // i4  +      .1   300    7.07  .8   1       2
    // i4  .      .8   400    7.09  .7   1       2
    // i4  27.2   .16  500    7.07  .6   1       2
    // i4  +      .14  600    7.05  .5   1       2
    // i4  .      .12  700    7.04  .4   1       2
    // i4  .      .12  800    7.02  .3   1       2
    // i4  .      .56  900    7.01  .4   1       2
    // i4  .     1.2   1200   7.02  .5   1       2

    y << play(0.12, 219.98, 0.9)
      << play(0.10, 195.99, 0.8)
      << play(0.80, 219.98, 0.7);

    // Use AudioStream::add to insert the precise delay.
    y.add(play(0.16, 195.99, 0.6), 1.20f);

    y << play(0.14, 174.61, 0.5)
      << play(0.12, 164.80, 0.4)
      << play(0.12, 146.83, 0.3)
      << play(0.56, 138.58, 0.4)
      << play(1.20, 146.83, 0.5);

    // i4  29.8   .12  1600   6.09  .5
    // i4  +      .1   .      6.07  .5
    // i4  .      .8   .      6.09  .5
    // i4  31     .3   .      6.05  .5
    // i4  +      .3   .      6.07  .5
    // i4  .      .3   .      6.01  .5
    // i4  .     1.2   .      6.02  .5

    y.add(play(0.12, 109.99), 2.80f);

    y << play(0.10,  97.99)
      << play(0.80, 109.99);

    y.add(play(0.30,  87.30), 5.00f);

    y << play(0.30,  97.99)
      << play(0.30,  69.29)
      << play(1.20,  73.41);

    // i4  33.2   .12  3000   5.09  .5
    // i4  +      .1   .      5.07  .5
    // i4  .      .8   .      5.09  .5
    // i4  34.4   .16  .      5.07  .5
    // i4  +      .14  .      5.05  .5
    // i4  .      .12  .      5.04  .5
    // i4  .      .12  .      5.02  .5
    // i4  .      .56  .      5.01  .5
    // i4  .     1.2   .      5.02  .5

    y.add(play(0.12, 55.00), 7.20f);

    y << play(0.10, 49.00)
      << play(0.80, 55.00);

    y.add(play(0.16, 49.00), 8.40f);

    y << play(0.14, 43.65)
      << play(0.12, 41.20)
      << play(0.12, 36.71)
      << play(0.56, 34.65)
      << play(1.20, 36.71);

    // i4  36.5  2.0   .      5.01  .5
    // i4  36.7  1.8   .      5.04  .5
    // i4  36.9  1.6   .      5.07  .5
    // i4  37.1  1.4   .      5.10  .5
    // i4  37.3  1.2   .      6.01  .5
    // i4  38.7  3.2   .      5.02  .5
    // i4  38.7  3.2   .      6.02  .5
    // i4  38.7  0.8   .      5.07  .5
    // i4  +     0.8   .      5.09  .5
    // i4  .     1.6   .      5.06  .5

    y.add(play(2.00, 34.65), 10.5f);
    y.add(play(1.80, 41.20), 10.7f);
    y.add(play(1.60, 49.00), 10.9f);
    y.add(play(1.40, 58.27), 11.1f);
    y.add(play(1.20, 69.29), 11.3f);
    y.add(play(3.20, 36.71), 12.7f);
    y.add(play(3.20, 73.41), 12.7f);
    y.add(play(0.08, 49.00), 12.7f);

    for(uint32 i = 0 ; i < sample_rate_ * 0.3; ++i) y << 0.0;

    y.normalize();

    return y;
}

AudioStream
OrganPipe::
play(
    const float64 & duration,
    const float64 & frequency)
{
    return play(duration, frequency, 0.5);
}


AudioStream
OrganPipe::
play(
    const float64 & duration,
    const float64 & frequency,
    const float64 & pan)
{
    M_ASSERT_VALUE(duration, >, 0);

    // The three phases of software development:
    //    1. Make it work.
    //    2. Make it work well.
    //    3. Make it work well for others.

    float64 pan_left = pan;
    float64 pan_right = 1.0 - pan_left;

    float64 op1f =  1.00 * frequency;
    float64 op2f =  2.01 * frequency;
    float64 op3f =  3.99 * frequency;
    float64 op4f =  8.00 * frequency;
    float64 op5f =  0.50 * frequency;
    float64 op7f = 16.00 * frequency;

    Sine sin(sample_rate_);

    Buffer dclick;
    Buffer amp1;
    Buffer amp2;
    Buffer amp3;
    Buffer amp4;

    if(duration > 0.002)
    {
        dclick
            << sin.drawLine(0.001, 0.0, 1.0)
            << sin.drawLine(duration - 0.002, 1.0, 1.0)
            << sin.drawLine(0.001, 1.0, 0.0);
    }
    else
    {
        dclick << sin.drawLine(duration, 1.0, 1.0);
    }

    if(duration > 0.02)
    {
        amp1
            << sin.drawLine(0.01, 0.0, 1.0)
            << sin.drawLine(duration - 0.02, 1.0, 1.0)
            << sin.drawLine(0.01, 1.0, 0.0);
    }
    else
    {
        amp1 = sin.drawLine(duration, 1.0, 1.0);
    }

    if(duration > 0.16)
    {
        amp2
            << sin.drawLine(0.05, 0.0, 1.0)
            << sin.drawLine(0.1, 1.0, 0.7)
            << sin.drawLine(duration - 0.16, 0.7, 0.7)
            << sin.drawLine(0.01, 0.7, 0.0);
    }
    else
    {
        amp2 = sin.drawLine(duration, 1.0, 1.0);
    }

    if(duration > 0.08)
    {
        amp3
            << sin.drawLine(0.03, 0.0, 0.8)
            << sin.drawLine(0.05, 0.8, 0.0)
            << sin.drawLine(duration - 0.08, 0.0, 0.0);
    }
    else
    {
        amp3 = sin.drawLine(duration, 1.0, 1.0);
    }

    if(duration > 0.21)
    {
        amp4
            << sin.drawLine(0.1, 0.0, 0.3)
            << sin.drawLine(0.1, 0.3, 0.05)
            << sin.drawLine(duration -0.21, 0.05, 0.1)
            << sin.drawLine(0.01, 0.1, 0.0);
    }
    else
    {
        amp4 = sin.drawLine(duration, 1.0, 1.0);
    }

    Buffer op8 = amp4 * sin.generate(duration, op5f);

    Buffer p1 = op8 + 1.0;

    Buffer out = amp1 * sin.generate(duration, p1 * op1f);
    out += amp2 * sin.generate(duration, p1 * op2f);
    out += amp2 * sin.generate(duration, op3f);
    out += amp2 * sin.generate(duration, op4f);
    out += amp3 * sin.generate(duration, 5.0 * op5f);
    out += amp2 * sin.generate(duration, op7f);

    AudioStream y(sample_rate_, 2);

    y[0] = out * dclick * pan_left;
    y[1] = out * dclick * pan_right;

    return y;
}

// :mode=c++: jEdit modeline
