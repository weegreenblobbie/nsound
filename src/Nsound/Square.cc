//-----------------------------------------------------------------------------
//
//  $Id: Square.cc 874 2014-09-08 02:21:29Z weegreenblobbie $
//
//  Copyright (c) 2006 Nick Hilton
//
//  weegreenblobbie2_gmail_com (replace '_' with '@' and '.')
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
#include <Nsound/Generator.h>
#include <Nsound/Square.h>

#include <cmath>
#include <cstring>
#include <cstdlib>

using namespace Nsound;

//-----------------------------------------------------------------------------
//! Appends a line from y1 to y2 between the fractions of a wavelength from
//! and to.
//
//! Both ends are rounded to whole samples, so lines that meet add up to
//! exactly one wavelength.  A line with no samples adds nothing.
static
void
draw_segment(
    Buffer & waveform,
    const Generator & line,
    const float64 & sample_rate,
    const float64 & from,
    const float64 & to,
    const float64 & y1,
    const float64 & y2)
{
    float64 n = std::round(to * sample_rate) - std::round(from * sample_rate);

    if(n == 0.0) return;

    waveform << line.drawLine(n / sample_rate, y1, y2);
}

//-----------------------------------------------------------------------------
Buffer
drawSquare(
    const float64 & sample_rate,
    const float64 & percent_lambda_1,
    const float64 & amplitude_1,
    const float64 & percent_lambda_2,
    const float64 & percent_lambda_3,
    const float64 & amplitude_3,
    const float64 & percent_lambda_4)
{
    Buffer waveform;

    //    1----2
    //    |    |
    //    |    |
    //    |    |
    //  --1    2--3    4--5
    //            |    |
    //            |    |
    //            |    |
    //            3----4
    //
    //  |-----lambda------|

    // Draw the 5 lines
    Generator line(sample_rate);

    // Line 1
    draw_segment(waveform, line, sample_rate, 0.0, percent_lambda_1, 0.0, 0.0);
    // Line 2
    draw_segment(waveform, line, sample_rate, percent_lambda_1, percent_lambda_2, amplitude_1, amplitude_1);
    // Line 3
    draw_segment(waveform, line, sample_rate, percent_lambda_2, percent_lambda_3, 0.0, 0.0);
    // Line 4
    draw_segment(waveform, line, sample_rate, percent_lambda_3, percent_lambda_4, amplitude_3, amplitude_3);
    // Line 5
    draw_segment(waveform, line, sample_rate, percent_lambda_4, 1.0, 0.0, 0.0);

    return waveform;
}

//-----------------------------------------------------------------------------
Square::
Square(const float64 & sample_rate) : Generator(sample_rate)
{
    // Half a wavelength at 1, half at -1.
    ctor(sample_rate, drawSquare(sample_rate, 0.0, 1.0, 0.5, 0.5, -1.0, 1.0));
}

//-----------------------------------------------------------------------------
Square::
Square(
    const float64 & sample_rate,
    const int32 n_harmonics)
    :
    Generator(sample_rate)
{
    // From wikipedia's definition of a square wave.

    float64 Nf = static_cast<float64>(std::abs(n_harmonics));

    if(Nf < 1.0) Nf = 1.0;

    Buffer waveform = Buffer::zeros(static_cast<uint32>(sample_rate_));

    for(float64 k = 1.0; k <= Nf; k += 1.0)
    {
        float64 kk = 2.0 * k - 1.0;

        waveform += drawSine(1.0, kk) / kk;
    }

    waveform *= 4.0 / M_PI;

    ctor(sample_rate, waveform);
}


//-----------------------------------------------------------------------------
Square::
Square(
    const float64 & sample_rate,
    const float64 & percent_lambda_1,
    const float64 & amplitude_1,
    const float64 & percent_lambda_2,
    const float64 & percent_lambda_3,
    const float64 & amplitude_3,
    const float64 & percent_lambda_4) : Generator(sample_rate)
{
    ctor(sample_rate,
        drawSquare(
            sample_rate,
            percent_lambda_1,
            amplitude_1,
            percent_lambda_2,
            percent_lambda_3,
            amplitude_3,
            percent_lambda_4));
}
