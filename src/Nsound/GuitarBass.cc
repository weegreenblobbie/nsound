//-----------------------------------------------------------------------------
//
//  $Id: GuitarBass.cc 874 2014-09-08 02:21:29Z weegreenblobbie $
//
//  Copyright (c) 2004-Present Nick Hilton
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
#include <Nsound/FilterBandPassIIR.h>
#include <Nsound/FilterTone.h>
#include <Nsound/GuitarBass.h>
#include <Nsound/Plotter.h>
#include <Nsound/Triangle.h>

using namespace Nsound;

//-----------------------------------------------------------------------------
GuitarBass::
GuitarBass(const float64 & sample_rate)
    :
    Instrument(sample_rate)
{
}

//-----------------------------------------------------------------------------
GuitarBass::
~GuitarBass()
{
}

AudioStream
GuitarBass::
play()
{
    AudioStream y(sample_rate_, 1);

    //              duration,   freq
    y <<        play(0.450,    61.734)
      << 0.90 * play(0.300,   123.467)
      << 0.95 * play(0.150,    92.491)
      << 0.80 * play(0.450,    61.734)
      << 0.80 * play(0.450,    46.245)
      << 0.80 * play(0.450,    55.000)
      << 0.80 * play(0.450,    61.734)
      <<        play(0.450,    73.414)
      <<        play(0.650,    61.734);

    return y;
}

AudioStream
GuitarBass::
play(const float64 & duration, const float64 & frequency)
{
    Triangle tri(sample_rate_);

    Buffer outenv =  tri.drawLine(0.01, 0.0, 1.0)
                  << tri.drawLine(duration - 0.11, 1.0, 1.0)
                  << tri.drawLine(0.11, 1.0, 0.0);

    // draw a tirangle envlope
    float64 triangle_duration = 1.0 / (2.0 * frequency);
    Buffer triangle =  -1.0 * tri.generate(triangle_duration, 2.0 * frequency)
                    << tri.drawLine(duration - triangle_duration, 0.0,0.0);

    // this envlope is for the body resonance
    Buffer envres =  tri.drawLine(0.1, 0.0, 1.0)
                  << tri.drawLine(duration - 0.1, 1.0, 1.0);

    FilterTone tone(sample_rate_, frequency * frequency / 10.0);

    DelayLine delay(sample_rate_, 1.0 / frequency);

    uint32 n_samples = static_cast<uint32>(duration * sample_rate_);

    float64 filter_out = 0.0;

    AudioStream y(sample_rate_, 1);

    Buffer::circular_iterator tri_iter = triangle.cbegin();

    filter_out = tone.filter(0);
    for(uint32 n = 0; n < n_samples; ++n, ++tri_iter)
    {
        delay.write(filter_out + *tri_iter);
        filter_out = tone.filter(delay.read());
        y << filter_out;
    }

    // Add some resonance from the body

    FilterBandPassIIR body1(sample_rate_, 2, 120.0, 200.0, 0.01);
    FilterBandPassIIR body2(sample_rate_, 2,  60.0, 100.0, 0.01);

    y += 0.0001  * body1.filter(y)
       + 0.00004 * body2.filter(y);

    y *= outenv;

    return y;
}


// :mode=c++: jEdit modeline
