//-----------------------------------------------------------------------------
//
//  $Id: FilterDelay_UnitTest.cc 874 2014-09-08 02:21:29Z weegreenblobbie $
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

#include <Nsound/AudioStream.h>
#include <Nsound/Buffer.h>
#include <Nsound/FilterDelay.h>
#include <Nsound/Plotter.h>
#include <Nsound/Sine.h>
#include <Nsound/Wavefile.h>

#include "UnitTest.h"

#include <stdlib.h>
#include <iostream>

using namespace Nsound;

using std::cerr;
using std::cout;
using std::endl;

// The __FILE__ macro includes the path, I don't want the whole path.
static const char * THIS_FILE = "FilterDelay_UnitTest.cc";

static const float64 GAMMA = 1.5e-14;

void FilterDelay_UnitTest()
{
    cout << endl << THIS_FILE;

    Wavefile::setDefaultSampleSize(48);

    FilterDelay f(100.0, 3.0);

    Sine sine(100);

    AudioStream input(100, 1);

    input << sine.generate(0.25, 4.0)
          << sine.silence(0.75);

    cout << TEST_HEADER << "Testing FilterDelay::filter(input) ...";

    AudioStream data = f.filter(input, 0.333);

    // Create the gold file
//~    data >> "gold/FilterDelay_out1.wav";

    AudioStream gold;

    gold << "gold/FilterDelay_out1.wav";

    AudioStream diff = data - gold;

    if(diff.getAbs().getMax() > GAMMA)
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


    data = f.filter(input, 0.666);

    // Create the gold file
//~    data >> "gold/FilterDelay_out2.wav";

    gold = AudioStream("gold/FilterDelay_out2.wav");

    diff = data - gold;

    if(diff.getAbs().getMax() > GAMMA)
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


    FilterDelay f2(1.0, 5.0);

    input = AudioStream(1,1);

    input << 1.0 << 0.0 << 0.0 << 0.0 << 0.0
          << 0.0 << 0.0 << 0.0 << 0.0 << 0.0;

    data = f2.filter(input, 60.0);

    // Create the gold file
//~    data >> "gold/FilterDelay_out3.wav";

    gold = AudioStream("gold/FilterDelay_out3.wav");

    diff = data - gold;

    if(diff.getAbs().getMax() > GAMMA)
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


