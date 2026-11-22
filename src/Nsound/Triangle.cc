//-----------------------------------------------------------------------------
//
//  $Id: Triangle.cc 874 2014-09-08 02:21:29Z weegreenblobbie $
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
#include <Nsound/Triangle.h>

#include <cmath>
#include <iostream>

using namespace Nsound;

using std::cout;
using std::endl;

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

Triangle::
Triangle(const float64 & sample_rate) : Generator(sample_rate)
{
    float64 third = 1.0/3.0;

    Buffer waveform;

    waveform << drawLine(third, 0.0, 1.0);
    waveform << drawLine(third, 1.0, -1.0);
    waveform << drawLine(third, -1.0, 0.0);

    uint32 n = static_cast<uint32>(sample_rate);

    // Pad if necessary.
    while(waveform.getLength() < n) waveform << 0.0;

    // Throw away if necessary.
    if(waveform.getLength() > n)
    {
        waveform = waveform.subbuffer(0, n);
    }

    ctor(sample_rate, waveform);
}

Triangle::
Triangle(
    const float64 & sample_rate,
    const float64 & attack_time,
    const float64 & release_time) : Generator(sample_rate)
{
    M_ASSERT_VALUE(attack_time, >=, 0);
    M_ASSERT_VALUE(release_time, >=, 0);
    M_ASSERT_VALUE(1.0 - attack_time - release_time, >, 0);

    Buffer waveform;

    float64 release_start = 1.0 - release_time;

    draw_segment(waveform, *this, sample_rate, 0.0, attack_time, 0.0, 1.0);
    draw_segment(waveform, *this, sample_rate, attack_time, release_start, 1.0, -1.0);
    draw_segment(waveform, *this, sample_rate, release_start, 1.0, -1.0, 0.0);

    ctor(sample_rate, waveform);
}


//-----------------------------------------------------------------------------
// drawTriangle
//
//         2                                     //
//        /\                                     //
//       /  \                                    //
//      /    \                                   //
//     /      \                                  //
//  --1        3--4          6--                 //
//                 \        /                    //
//                  \      /                     //
//                   \    /                      //
//                    \  /                       //
//                     \/                        //
//                      5                        //
//                                               //
//  |----------lambda----------|                 //
//
//-----------------------------------------------------------------------------
Buffer
drawTriangle(
    const float64 & sample_rate,
    const float64 & percent_lambda_1,
    const float64 & percent_lambda_2,
    const float64 & amplitude_2,
    const float64 & percent_lambda_3,
    const float64 & percent_lambda_4,
    const float64 & percent_lambda_5,
    const float64 & amplitude_5,
    const float64 & percent_lambda_6)
{
    Buffer waveform;

    // Draw the 7 lines
    Generator line(sample_rate);

    // Line 1
    draw_segment(waveform, line, sample_rate, 0.0, percent_lambda_1, 0.0, 0.0);
    // Line 2
    draw_segment(waveform, line, sample_rate, percent_lambda_1, percent_lambda_2, 0.0, amplitude_2);
    // Line 3
    draw_segment(waveform, line, sample_rate, percent_lambda_2, percent_lambda_3, amplitude_2, 0.0);
    // Line 4
    draw_segment(waveform, line, sample_rate, percent_lambda_3, percent_lambda_4, 0.0, 0.0);
    // Line 5
    draw_segment(waveform, line, sample_rate, percent_lambda_4, percent_lambda_5, 0.0, amplitude_5);
    // Line 6
    draw_segment(waveform, line, sample_rate, percent_lambda_5, percent_lambda_6, amplitude_5, 0.0);
    // Line 7
    draw_segment(waveform, line, sample_rate, percent_lambda_6, 1.0, 0.0, 0.0);

    return waveform;
}

Triangle::
Triangle(
    const float64 & sample_rate,
    const float64 & percent_lambda_1,
    const float64 & percent_lambda_2,
    const float64 & amplitude_2,
    const float64 & percent_lambda_3,
    const float64 & percent_lambda_4,
    const float64 & percent_lambda_5,
    const float64 & amplitude_5,
    const float64 & percent_lambda_6) : Generator(sample_rate)
{
    //         2                        //
    //        /\                        //
    //       /  \                       //
    //      /    \                      //
    //     /      \                     //
    //  --1        3--4          6--    //
    //                 \        /       //
    //                  \      /        //
    //                   \    /         //
    //                    \  /          //
    //                     \/           //
    //                      5           //
    //                                  //
    //  |----------lambda----------|    //

    Buffer waveform = drawTriangle(
        sample_rate,
        percent_lambda_1,
        percent_lambda_2,
        amplitude_2,
        percent_lambda_3,
        percent_lambda_4,
        percent_lambda_5,
        amplitude_5,
        percent_lambda_6);

    ctor(sample_rate, waveform);
}
