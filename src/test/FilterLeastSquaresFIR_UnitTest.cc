//-----------------------------------------------------------------------------
//
//  $Id: FilterLeastSquaresFIR_UnitTest.cc 874 2014-09-08 02:21:29Z weegreenblobbie $
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
#include <Nsound/FilterLeastSquaresFIR.h>
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
using std::flush;

// The __FILE__ macro includes the path, I don't want the whole path.
#define THIS_FILE "FilterLeastSquaresFIR_UnitTest.cc"

#define GAMMA 1.5e-14

void FilterLeastSquaresFIR_UnitTest()
{
    cout << endl << THIS_FILE;

    Wavefile::setDefaultSampleSize(64);
    Wavefile::setIEEEFloat(true);

    float64 sr = 1000.0;

    // Create a low pass least squares FIR filter

    Buffer freqs;
    Buffer amplitude;

    freqs     << 0.0 << 100.0 << 100.0 << 500.0;
    amplitude << 1.0 << 1.0   << 0.0   << 0.0;

    FilterLeastSquaresFIR f(sr, 1025, freqs, amplitude);

    Buffer noise("gold/Filter_noise.wav");

    cout << TEST_HEADER << "Testing FilterLeastSquaresFIR::filter(input) ...";

    Buffer data;

    data  = f.filter(noise);

    // Create the gold file
    //~    data >> "gold/FilterLeastSquaresFIR_out1.wav";

    Buffer gold("gold/FilterLeastSquaresFIR_out1.wav");

    Buffer diff = data - gold;

    diff.abs();

    if(diff.getMax() > GAMMA)
    {
        cerr << TEST_ERROR_HEADER
             << "Output did not match gold file!"
             << endl
             << flush;

        diff.plot("data - gold");
        data.plot("data");
        gold.plot("gold");

        Plotter::show();

        exit(1);
    }

    cout << SUCCESS;

    // Repeated to test that reset() is being called.
    cout << TEST_HEADER << "Testing FilterLeastSquaresFIR::filter(input) ..."
         << flush;

    data = f.filter(noise);

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

    // Create a high pass least squares FIR filter

    freqs = Buffer(4);
    amplitude = Buffer(4);

    freqs     << 0.0 << 100.0 << 100.0 << 500.0;
    amplitude << 0.0 << 0.0   << 1.0   << 1.0;

    f.makeKernel(freqs, amplitude);

    cout << TEST_HEADER << "Testing FilterLeastSquaresFIR::filter(input) ...";

    data = f.filter(0.9 * noise);

    // Create the gold file
    //~    data >> "gold/FilterLeastSquaresFIR_out2.wav";

    gold = Buffer("gold/FilterLeastSquaresFIR_out2.wav");

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

    // Repeated to test that reset() is being called.
    cout << TEST_HEADER << "Testing FilterLeastSquaresFIR::filter(input) ...";

    data = f.filter(0.9 * noise);

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

    // Repeated to test that reset() is being called.
    cout << TEST_HEADER << "Testing FilterLeastSquaresFIR::setWindow() ...";

    freqs     = Buffer(4);
    amplitude = Buffer(4);

    freqs     << 0.0 << 30.0 << 30.0 << 500.0;
    amplitude << 1.0 << 1.0   << 0.0   << 0.0;

    f = FilterLeastSquaresFIR(sr, 32, freqs, amplitude);

    f.setWindow(RECTANGULAR);

    data = f.getFrequencyResponse().getdB();

//~    // Create gold file
//~    cout << "SAVING GOLD FILE: gold/FilterLeastSquaresFIR_out3.wav" << endl;
//~    data >> "gold/FilterLeastSquaresFIR_out3.wav";

    gold = Buffer("gold/FilterLeastSquaresFIR_out3.wav");

    diff = data - gold;

    float64 max_diff = diff.getAbs().getMax();

    if(max_diff > 1e-9) // gamma is changed here
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
