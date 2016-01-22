//-----------------------------------------------------------------------------
//
//  $Id: Wavefile_UnitTest.cc 874 2014-09-08 02:21:29Z weegreenblobbie $
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

#include <Nsound/Nsound.h>

#include <Nsound/AudioStream.h>
#include <Nsound/Buffer.h>
#include <Nsound/Plotter.h>
#include <Nsound/Sine.h>
#include <Nsound/Wavefile.h>

#include "UnitTest.h"

#include <stdlib.h>
#include <iostream>

using namespace Nsound;

using std::cout;
using std::cerr;
using std::endl;
using std::flush;

static const char * THIS_FILE = "Wavefile_UnitTest.cc";

static const float64 GAMMA = 1.5e-14;

void
Wavefile_UnitTest()
{
    cout << endl << THIS_FILE;

    Wavefile::setDefaultSampleSize(48);

    Sine sin(100);

    AudioStream data1(100, 3);

    data1 << sin.generate(1.0, 13.0);

    data1[1] *= 0.666666666666666;
    data1[2] *= 0.333333333333333;

    // Write out 10 channel 13 Hz wave.
    cout << TEST_HEADER << "Testing Wavefile::write(48-bit), read(48-bit) ..." << flush;

    data1 >> "test_wavefile.wav";

    AudioStream data(100, 3);

    data << "test_wavefile.wav";

    AudioStream gold(100, 3);

    // Create gold file
//~    data >> "gold/Wavefile_out1.wav";

    gold << "gold/Wavefile_out1.wav";

    AudioStream diff = data - gold;

    AudioStream abs_diff(diff);
    abs_diff.abs();

    for(int32 i = 0; i < 3; ++i)
    {
        if(abs_diff[i].getMax() > GAMMA)
        {
            cerr << TEST_ERROR_HEADER
                 << "Output did not match gold file!"
                 << endl;

            diff[i].plot("data - gold");
            data[i].plot("data");
            gold[i].plot("gold");

            Plotter::show();

            exit(1);
        }
    }

    cout << SUCCESS;

    // Write out 10 channel 13 Hz wave again but with 24 bits per sample
    cout << TEST_HEADER << "Testing Wavefile::write(24-bit), read(24-bit) ..." << flush;

    Wavefile::setDefaultSampleSize(24);

    data1 >> "test_wavefile2.wav";

    data = AudioStream(100, 3);
    data << "test_wavefile2.wav";

    // Create gold file
//~    data >> "gold/Wavefile_out2.wav";

    gold = AudioStream(100, 3);
    gold << "gold/Wavefile_out2.wav";

    diff = data - gold;

    abs_diff = diff;
    abs_diff.abs();

    for(int32 i = 0; i < 1; ++i)
    {
        if(abs_diff[i].getMax() > 0.5 * GAMMA)
        {
            cerr << TEST_ERROR_HEADER
                 << "Output did not match gold file!"
                 << endl;

            diff[i].plot("data - gold");
            data[i].plot("data");
            gold[i].plot("gold");

            Plotter::show();

            exit(1);
        }
    }

    cout << SUCCESS << endl;
}
