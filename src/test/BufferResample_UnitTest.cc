//-----------------------------------------------------------------------------
//
//  $Id: BufferResample_UnitTest.cc 874 2014-09-08 02:21:29Z weegreenblobbie $
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
#include <Nsound/Plotter.h>
#include <Nsound/Sine.h>
#include <Nsound/Wavefile.h>

#include "UnitTest.h"

#include <cmath>
#include <stdlib.h>

#include <iostream>
#include <sstream>

using namespace Nsound;

using std::cerr;
using std::cout;
using std::endl;

// The __FILE__ macro includes the path, I don't want the whole path.
#define THIS_FILE "BufferResample_UnitTest.cc"
#define GAMMA 1.5e-14

void BufferResample_UnitTest()
{
    cout << endl << THIS_FILE;

    Wavefile::setDefaultSampleSize(64);
    Wavefile::setIEEEFloat(true);

    static const uint32 LM[4] = { 2,3,5,7 };

    Sine   sin(150);

    Buffer input = sin.generate(1.0, 3.0);

    cout << TEST_HEADER << "Testing Buffer:getResample(L,M) ...";

    Buffer data;
    Buffer gold;
    Buffer diff;

    std::stringstream ss;

    for(uint32 i  = 1; i <= 4; ++i)
    {

        std::string gold_filename;

        ss.str("");
        ss << "gold/BufferResample_out_" << i << "_" << LM[i-1] << ".wav";

        data = input.getResample(i, LM[i-1]);

        // Create gold file
//~        data >> ss.str().c_str();

        gold = Buffer(ss.str().c_str());

        diff = data - gold;

        if(gold.getLength() != data.getLength() ||
           diff.getAbs().getMax() > GAMMA)
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

        // Reverse L & M
        ss.str("");
        ss << "gold/BufferResample_out_" << LM[i-1] << "_" << i << ".wav";

        data = input.getResample(LM[i-1], i);

        // Create gold file
//~        data >> ss.str().c_str();

        gold = Buffer(ss.str().c_str());

        diff = data - gold;

        if(gold.getLength() != data.getLength() ||
           diff.getAbs().getMax() > GAMMA)
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

    }

    cout << SUCCESS << endl;
}


