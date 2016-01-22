//-----------------------------------------------------------------------------
//
//  $Id: Sine.cc 874 2014-09-08 02:21:29Z weegreenblobbie $
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
#include <Nsound/Sine.h>

#include <cmath>

using namespace Nsound;

//-----------------------------------------------------------------------------
Sine::
Sine(const float64 & sample_rate)
    : Generator(sample_rate)
{
//~    float64 two_pi = 2.0 * M_PI;

//~    float64 w = two_pi / sample_rate_;

//~    uint64 n_samples = static_cast<uint64>(sample_rate_);

//~    for(uint64 i = 0; i < n_samples; ++i)
//~    {
//~        waveform_[i] = std::sin(i * w);
//~    }

    Buffer waveform = drawSine(1.0, 1.0);
    ctor(sample_rate, waveform);
}
