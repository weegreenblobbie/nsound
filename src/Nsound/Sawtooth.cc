//-----------------------------------------------------------------------------
//
//  $Id: Sawtooth.cc 874 2014-09-08 02:21:29Z weegreenblobbie $
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
#include <Nsound/Sawtooth.h>
#include <Nsound/Sine.h>

#include <cmath>
#include <cstring>
#include <cstdlib>

using namespace Nsound;

//-----------------------------------------------------------------------------
Sawtooth::
Sawtooth(const float64 & sample_rate, const int32 n_harmonics)
    : Generator(sample_rate)
{

    // From wikipedia's definition of a sawtooth wave.

    float64 Nf = static_cast<float64>(std::abs(n_harmonics));

    if(Nf < 1.0) Nf = 1.0;

    Buffer waveform = Buffer::zeros(static_cast<uint32>(sample_rate_));

    for(float64 k = 1.0; k <= Nf; k += 1.0)
    {
        waveform += drawSine(1.0, k) / k;
    }

    waveform *= 2.0 / M_PI;

    ctor(sample_rate, waveform);
}
