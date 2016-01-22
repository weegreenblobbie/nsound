//-----------------------------------------------------------------------------
//
//  $Id: Triangle_UnitTest.cc 874 2014-09-08 02:21:29Z weegreenblobbie $
//
//  Copyright (c) 2006-Present Nick Hilton
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
#include <Nsound/Plotter.h>
#include <Nsound/Triangle.h>
#include <Nsound/Wavefile.h>

#include "UnitTest.h"

#include <cmath>
#include <stdlib.h>
#include <iostream>

using namespace Nsound;

using std::cerr;
using std::cout;
using std::endl;

// The __FILE__ macro includes the path, I don't want the whole path.
static const char * THIS_FILE = "Triangle_UnitTest.cc";

static const float64 GAMMA = 1.5e-14;

void Triangle_UnitTest()
{
    cout << endl << THIS_FILE;

    Wavefile::setDefaultSampleSize(64);
    Wavefile::setIEEEFloat(true);

    Triangle tri(100);

    Buffer data;
    Buffer gold;
    Buffer diff;

    // Test 7 Hz wave
    cout << TEST_HEADER << "Testing Triangle::generate(1.01, 2.0) ...";

    data = tri.generate(1.01, 2.0);

    // Create the gold file
//~    data >> "gold/Triangle_2Hz.wav";

    gold = Buffer("gold/Triangle_2Hz.wav");

    diff = data - gold;

    diff.abs();

    if(diff.getMax() > GAMMA)
    {
        cerr << TEST_ERROR_HEADER
             << "Output did not match gold file!"
             << endl;

        diff.plot("data - gold");
        data.plot("data");
        gold.plot("gold");

        Plotter::show();

        exit(1);
    }

    cout << SUCCESS;

    // Test 3.5 Hz wave
    cout << TEST_HEADER << "Testing Triangle::generate(1.01, 1.0) ...";

    data = tri.generate(1.01, 1.0);

    // Create the gold file
//~    data >> "gold/Triangle_1.0Hz.wav";

    gold = Buffer("gold/Triangle_1.0Hz.wav");

    diff = data - gold;

    diff.abs();

    if(diff.getMax() > GAMMA)
    {
        cerr << TEST_ERROR_HEADER
             << "Output did not match gold file!"
             << endl;

        diff.plot("data - gold");
        data.plot("data");
        gold.plot("gold");

        Plotter::show();

        exit(1);
    }

    cout << SUCCESS;

    // Test dynamic
    cout << TEST_HEADER << "Testing Triangle::generate(1.01, frequencies) ...";

    Buffer freqs = tri.drawLine(1.01, 1.0, 5.0);

    data = tri.generate(1.01, freqs);

    // Create the gold file
//~    data >> "gold/Triangle_1_to_5Hz.wav";

    gold = Buffer("gold/Triangle_1_to_5Hz.wav");

    diff = data - gold;

    diff.abs();

    if(diff.getMax() > GAMMA)
    {
        cerr << TEST_ERROR_HEADER
             << "Output did not match gold file!"
             << endl;

        diff.plot("data - gold");
        data.plot("data");
        gold.plot("gold");

        Plotter::show();

        exit(1);
    }

    Plotter::show();

    cout << SUCCESS << endl;
}


