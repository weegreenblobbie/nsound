//-----------------------------------------------------------------------------
//
//  $Id: DrumKickBass.cc 874 2014-09-08 02:21:29Z weegreenblobbie $
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
#include <Nsound/DrumKickBass.h>
#include <Nsound/FilterParametricEqualizer.h>
#include <Nsound/Square.h>

// DEBUG
#include <Nsound/Plotter.h>

using namespace Nsound;

//-----------------------------------------------------------------------------
DrumKickBass::
DrumKickBass(
    const float64 & sample_rate,
    const float64 & high_frequency,
    const float64 &  low_frequency)
    :
    Instrument(sample_rate),
    hi_f_(high_frequency),
    lo_f_( low_frequency){}

//-----------------------------------------------------------------------------
DrumKickBass::
~DrumKickBass(){}

AudioStream
DrumKickBass::
play()
{
    AudioStream y(sample_rate_, 1);

    y << play(0.6, 120.0, 51.0)
      << play(0.6, 120.0, 51.0)
      << play(0.6, 120.0, 51.0)
      << play(0.6, 120.0, 51.0);

    return y;
}

AudioStream
DrumKickBass::
play(
    const float64 & duration,
    const float64 & frequency)
{
    return play(duration, frequency, lo_f_);
}


AudioStream
DrumKickBass::
play(
    const float64 & duration,
    const float64 & hi_freq,
    const float64 & low_freq)
{
    Square square(sample_rate_);

    Buffer signal_freqs = 2.333 * hi_freq * square.drawDecay(0.08, 7.0)
                        + low_freq
                       << square.drawLine(duration - 0.08, low_freq, low_freq);

    Buffer signal = -0.5 * square.generate(duration, signal_freqs) << 0.0;


    Buffer filter_freqs = 0.8*square.drawDecay(duration * 0.5, 7.0) + 0.2
                       << square.drawLine(duration * 0.5, 0.2, 0.0);

    filter_freqs *= 2.0 * signal_freqs;

    FilterParametricEqualizer f1(
        FilterParametricEqualizer::HIGH_SHELF,
        sample_rate_, filter_freqs[0], 0.7071, 0.0);

    FilterParametricEqualizer f2(
        FilterParametricEqualizer::HIGH_SHELF,
        sample_rate_, signal_freqs[0] * 3.0, 1.0, 2.0);

    FilterParametricEqualizer f3(
        FilterParametricEqualizer::HIGH_SHELF,
        sample_rate_, low_freq * 1.25, 1.0, 2.5);

    AudioStream y(sample_rate_, 1);

    y = f1.filter(signal, filter_freqs);

    for(uint32 i = 0; i < y.getLength(); ++i)
    {
        y[0][1] = std::atan(y[0][i] * 20.0);
    }

    y = f2.filter(y, signal_freqs * 3.0);

    y = f3.filter(y) * 0.56;

    return y;
}

AudioStream
DrumKickBass::
play(
    const float64 & duration,
    const Buffer  & high_frequency,
    const Buffer  &  low_frequency)
{

    // FIXME FIXME FIXME FIXME, this is not dynamic!
    return play(duration, high_frequency[0], low_frequency[0]);

}


// :mode=c++: jEdit modeline
