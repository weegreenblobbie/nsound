//-----------------------------------------------------------------------------
//
//  $Id: Pluck.cc 874 2014-09-08 02:21:29Z weegreenblobbie $
//
//  Copyright (c) 2004-2007 Nick Hilton
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
#include <Nsound/FilterDC.h>
#include <Nsound/FilterTone.h>
#include <Nsound/Pluck.h>

#include <cmath>

using namespace Nsound;

//-----------------------------------------------------------------------------
Nsound::
Pluck::
Pluck(const float64 & sample_rate, uint32 n_smooth_samples)
    :
    Generator(sample_rate),
    n_smooth_samples_(n_smooth_samples)
{
}

//-----------------------------------------------------------------------------
Nsound::
Pluck::
~Pluck()
{
}

Buffer
Pluck::
generate(const float64 & duration, const float64 & frequency)
{
    DelayLine delay(sample_rate_, 1.0 / frequency);
    FilterDC dc_filter(0.99);
    FilterTone tone(sample_rate_, 14 * frequency * duration);

    Buffer noise_env =  drawLine(1.0 / frequency, 1.0, 1.0)
                     << drawLine(duration - (1.0 / frequency), 0.0, 0.0)
                     << 0.0;

    Buffer x = whiteNoise(duration) * noise_env * 2.0;

    x.smooth(1, n_smooth_samples_);
    x.normalize();

    uint32 n_samples = static_cast<int32>(duration * sample_rate_);

    Buffer y;

    delay.write(x[0]);
    for(uint32 n = 0; n < n_samples; ++n)
    {

        y << dc_filter.filter(tone.filter(delay.read()));

        delay.write(x[n] + y[n]);
    }
    y.normalize();

    return y;
}

