//-----------------------------------------------------------------------------
//
//  $Id: Sine_UnitTest.cc 874 2014-09-08 02:21:29Z weegreenblobbie $
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
#include <Nsound/Sine.h>
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
static const char * THIS_FILE = "Sine_UnitTest.cc";

static const float64 GAMMA = 1.5e-14;

void Sine_UnitTest()
{
    cout << endl << THIS_FILE;

    Wavefile::setDefaultSampleSize(64);
    Wavefile::setIEEEFloat(true);

    Sine sine(600);

    Buffer freqs;
	Buffer phase;

	freqs << sine.drawLine(1.0, 0.0, 10.0);
	phase << sine.drawLine(1.0, 0.0, 1.0);

    cout << TEST_HEADER << "Testing Sine::generate(1.0, 3.0) ...";

    Buffer data = sine.generate(1.0, 3.0);

    // Create the gold file
//~    data >> "gold/Sine_1.wav";

    Buffer gold;

    gold << "gold/Sine_1.wav";

    Buffer diff = data - gold;

    Buffer abs_diff(diff);
    abs_diff.abs();

    if(abs_diff.getMax() > GAMMA)
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
    cout << TEST_HEADER << "Testing Sine::generate(1.0, freqs) ...";

    data = sine.generate(1.0, freqs);

    // Create the gold file
//~    data >> "gold/Sine_2.wav";

    gold = Buffer();

    gold << "gold/Sine_2.wav";

    diff = data - gold;

    abs_diff = diff;
    abs_diff.abs();

    if(abs_diff.getMax() > GAMMA)
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
    cout << TEST_HEADER << "Testing Sine::generate2(1.0, 3.0, 0.5) ...";

    data = sine.generate2(1.0, 3.0, 0.5);

    // Create the gold file
//~    data >> "gold/Sine_3.wav";

    gold = Buffer();

    gold << "gold/Sine_3.wav";

    diff = data - gold;

    abs_diff = diff;
    abs_diff.abs();

    if(abs_diff.getMax() > GAMMA)
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
    cout << TEST_HEADER << "Testing Sine::generate2(1.0, 3.0, phase) ...";

    data = sine.generate2(1.0, 3.0, phase);

    // Create the gold file
//~    data >> "gold/Sine_4.wav";

    gold = Buffer();

    gold << "gold/Sine_4.wav";

    diff = data - gold;

    abs_diff = diff;
    abs_diff.abs();

    if(abs_diff.getMax() > GAMMA)
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
    cout << TEST_HEADER << "Testing Sine::generate2(1.0, freqs, 0.5) ...";

    data = sine.generate2(1.0, freqs, 0.5);

    // Create the gold file
//~    data >> "gold/Sine_5.wav";

    gold = Buffer();

    gold << "gold/Sine_5.wav";

    diff = data - gold;

    abs_diff = diff;
    abs_diff.abs();

    if(abs_diff.getMax() > GAMMA)
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
    cout << TEST_HEADER << "Testing Sine::generate2(1.0, freqs, phase) ...";

    data = sine.generate2(1.0, freqs, phase);

    // Create the gold file
//~    data >> "gold/Sine_6.wav";

    gold = Buffer();

    gold << "gold/Sine_6.wav";

    diff = data - gold;

    abs_diff = diff;
    abs_diff.abs();

    if(abs_diff.getMax() > GAMMA)
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
    cout << TEST_HEADER << "Testing Sine::setChorus(5,0.10) ...";

    sine.setSeed(6846513); // Some random seed I typed in.

    sine.setChorus(5, 0.10);

    data = sine.generate2(1.0, 2*freqs, phase);

    // Create the gold file
//~    data >> "gold/Sine_7.wav";

    gold = Buffer();

    gold << "gold/Sine_7.wav";

    diff = data - gold;

    abs_diff = diff;
    abs_diff.abs();

    if(abs_diff.getMax() > GAMMA)
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

    cout << SUCCESS << endl;
}


