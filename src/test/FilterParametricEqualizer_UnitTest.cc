//-----------------------------------------------------------------------------
//
//  $Id: FilterParametricEqualizer_UnitTest.cc 874 2014-09-08 02:21:29Z weegreenblobbie $
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
#include <Nsound/FilterParametricEqualizer.h>

#include "UnitTest.h"

#include <cmath>
#include <stdlib.h>

#include <iostream>

using namespace Nsound;

using std::cerr;
using std::cout;
using std::endl;

// The __FILE__ macro includes the path, I don't want the whole path.
#define THIS_FILE "FilterParametricEqualizer_UnitTest.cc"
#define GAMMA 1.5e-14

void FilterParametricEqualizer_UnitTest()
{
    cout << endl << THIS_FILE;

    Wavefile::setDefaultSampleSize(64);
    Wavefile::setIEEEFloat(true);

    Sine sine(100);

    FilterParametricEqualizer peaking(
        FilterParametricEqualizer::PEAKING,
        100,
        12,
        0.2,
        4.0);

    FilterParametricEqualizer low(
        FilterParametricEqualizer::LOW_SHELF,
        100,
        12,
        0.2,
        4.0);

    FilterParametricEqualizer high(
        FilterParametricEqualizer::HIGH_SHELF,
        100,
        12,
        0.2,
        4.0);

    Buffer freqs = sine.drawLine(5.0, 24, 6);

    static const std::string titles[3] =
    {
        "Testing Peaking filter ...",
        "Testing Low Shelf filter ...",
        "Testing High Shelf filter ..."
    };

    static const std::string types[3] =
    {
        "peaking",
        "low",
        "high"
    };

    static const std::string gold_filenames[3] =
    {
        "gold/FilterParametricEqualizer_out_peaking.wav",
        "gold/FilterParametricEqualizer_out_low.wav",
        "gold/FilterParametricEqualizer_out_high.wav"
    };

    FilterParametricEqualizer * filters[3] =
    {
        &peaking,
        &low,
        &high
    };

    Buffer noise;
    Buffer data;
    Buffer gold;
    Buffer diff;

    noise << "gold/Filter_noise.wav";
    noise << "gold/Filter_noise.wav";

    for(int i = 0; i < 3; ++i)
    {
        cout << TEST_HEADER << titles[i];

        data = filters[i]->filter(noise, freqs);

        // Create gold file
//~        data >> gold_filenames[i].c_str();

        gold = Buffer(gold_filenames[i]);

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

    }

    cout << endl;
}


