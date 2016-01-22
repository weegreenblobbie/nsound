//-----------------------------------------------------------------------------
//
//  $Id: DelayLine_UnitTest.cc 874 2014-09-08 02:21:29Z weegreenblobbie $
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
#include <Nsound/DelayLine.h>
#include <Nsound/Plotter.h>
#include <Nsound/Sine.h>
#include <Nsound/Wavefile.h>

#include "UnitTest.h"

#include <stdlib.h>
#include <math.h>
#include <iostream>

using namespace Nsound;

using std::cout;
using std::cerr;
using std::endl;
using std::flush;

static const char * THIS_FILE = "DelayLine_UnitTest.cc";

static const float64 GAMMA = 1.5e-14;

void
DelayLine_UnitTest()
{
    cout << endl << THIS_FILE;

    Wavefile::setDefaultSampleSize(64);
    Wavefile::setIEEEFloat(true);

    DelayLine dl(1.0, 10.0);
    Sine sine(100.0);

    Buffer input;
    Buffer data;
    Buffer gold;
    Buffer diff;

    cout << TEST_HEADER << "Testing DelayLine::write() & read() ...";

    input = sine.generate(1.0, 2.0) * sine.drawLine(1.0, 1.0, 0.0);

    for(uint32 i = 0; i < 100; ++i)
    {
        dl.write(input[i]);
        data << dl.read();
    }

    // Create Gold files
//~    data >> "gold/DelayLine_out1.wav";

    gold = Buffer("gold/DelayLine_out1.wav");

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

    cout << SUCCESS << endl;
}
