//-----------------------------------------------------------------------------
//
//  $Id: FilterCombLowPassFeedback_UnitTest.cc 874 2014-09-08 02:21:29Z weegreenblobbie $
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

#include <Nsound/Buffer.h>

#include <Nsound/FilterCombLowPassFeedback.h>
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
static const char * THIS_FILE = "FilterCombLowPassFeedback_UnitTest.cc";

static const float64 GAMMA = 1.5e-14;

void FilterCombLowPassFeedback_UnitTest()
{
    cout << endl << THIS_FILE;

    Wavefile::setDefaultSampleSize(48);

    FilterCombLowPassFeedback f(100.0, 0.1, 0.66, 12.0);

    // Create some noise!
    Sine sine(100);

    Buffer noise;

    noise << "gold/Filter_noise.wav";

    cout << TEST_HEADER << "Testing FilterCombLowPassFeedback::filter(input) ...";

    Buffer data;

    data = f.filter(noise);
    data.normalize();

    // Create the gold file
//~    data >> "gold/FilterCombLowPassFeedback_out1.wav";

    Buffer gold;

    gold << "gold/FilterCombLowPassFeedback_out1.wav";

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

    cout << TEST_HEADER << "Testing FilterCombLowPassFeedback::filter(input, freqs) ...";

    Buffer freqs = sine.drawLine(1.0, 6.0, 50.0);

    data = f.filter(noise, freqs);
    data.normalize();

    // Create the gold file
//~    data >> "gold/FilterCombLowPassFeedback_out2.wav";

    gold = Buffer();

    gold << "gold/FilterCombLowPassFeedback_out2.wav";

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

//~    cout << SUCCESS;


    cout << SUCCESS << endl;

}


