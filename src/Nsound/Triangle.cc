//-----------------------------------------------------------------------------
//
//  $Id: Triangle.cc 874 2014-09-08 02:21:29Z weegreenblobbie $
//
//  Copyright (c) 2006 Nick Hilton
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
#include <Nsound/Generator.h>
#include <Nsound/Triangle.h>

#include <iostream>

using namespace Nsound;

using std::cout;
using std::endl;

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

    if(attack_time > 0)
    {
        waveform <<  drawLine(attack_time, 0, 1.0);
    }

    waveform << drawLine(1.0 - attack_time - release_time, 1.0, -1.0);

    if(release_time > 0)
    {
        waveform << drawLine(release_time, -1.0, 0.0);
    }

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
    waveform << line.drawLine(percent_lambda_1,
                                0.0,
                                0.0);
    // Line 2
    waveform << line.drawLine(percent_lambda_2 - percent_lambda_1,
                                0.0,
                                amplitude_2);
    // Line 3
    waveform << line.drawLine(percent_lambda_3 - percent_lambda_2,
                                amplitude_2,
                                0.0);
    // Line 4
    waveform << line.drawLine(percent_lambda_4 - percent_lambda_3,
                                0.0,
                                0.0);
    // Line 5
    waveform << line.drawLine(percent_lambda_5 - percent_lambda_4,
                                0.0,
                                amplitude_5);
    // Line 6
    waveform << line.drawLine(percent_lambda_6 - percent_lambda_5,
                                amplitude_5,
                                0.0);
    // Line 7
    waveform << line.drawLine(1.0 - percent_lambda_6,
                                0.0,
                                0.0);

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
