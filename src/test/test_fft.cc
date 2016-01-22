//-----------------------------------------------------------------------------
//
//  $Id: test_fft.cc 874 2014-09-08 02:21:29Z weegreenblobbie $
//
//  Copyright (c) 2008-Present Nick Hilton
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

#include <Nsound/NsoundAll.h>

#include "Test.h"

#include <stdlib.h>
#include <cmath>
#include <iostream>
#include <sstream>

using namespace Nsound;

using std::cerr;
using std::cout;
using std::endl;
using std::flush;

static const char * THIS_FILE = "test_fft.cc";

int main(int argc, char ** argv)
{
    uint32 SAMPLE_RATE = 44100;

    Sine sine(SAMPLE_RATE);

    Buffer input = sine.generate(2.0,  4000.0)
                 + sine.generate(2.0,  8000.0)
                 + sine.generate(2.0, 12000.0);

    FFTransform fft(SAMPLE_RATE);

    Tic();
    cout << TEST_HEADER
         << "FFTransform::fft(), FFTransform::ifft() ... " << flush;

    FFTChunkVector vec = fft.fft(input, 2048);

    // Call the inverse fft.
    Buffer ifft = fft.ifft(vec);

    Buffer diff = input - ifft;

    diff.abs();

    if(diff.getMax() > 0.00005)
    {
        cerr << TEST_ERROR_HEADER
             << "fft() - ifft() > 0.00005, max(diff) = "
             << diff.getMax()
             << endl;
        exit(1);
    }

    cout << Toc() << " seconds: SUCCESS" << endl;

    float64 fl = 440;
    float64 fh = fl * 2;

    FilterLowPassFIR    f1(SAMPLE_RATE, 512, fl);
    FilterHighPassFIR   f2(SAMPLE_RATE, 512, fl);
    FilterBandPassFIR   f3(SAMPLE_RATE, 512, fl, fh);
    FilterBandRejectFIR f4(SAMPLE_RATE, 512, fl, fh);

    FilterLowPassIIR    f11(SAMPLE_RATE, 6, fl, 0.01);
    FilterHighPassIIR   f22(SAMPLE_RATE, 6, fl, 0.01);
    FilterBandPassIIR   f33(SAMPLE_RATE, 6, fl, fh, 0.01);
    FilterBandRejectIIR f44(SAMPLE_RATE, 6, fl, fh, 0.01);

    Tic();
    cout << TEST_HEADER
         << "Plotting Frequecy Responses of Filters ... " << flush;

    Plotter pylab;

    f4.plot();
    pylab.xlim(0, fh * 2);
    pylab.ylim(-40, 5);

    f44.plot();
    pylab.xlim(0, fh * 2);
    pylab.ylim(-40, 5);

    f3.plot();
    pylab.xlim(0,fh * 2);
    pylab.ylim(-40,5);

    f33.plot();
    pylab.xlim(0,fh * 2);
    pylab.ylim(-40,5);

    f2.plot();
    pylab.xlim(0,fh * 2);
    pylab.ylim(-40,5);

    f22.plot();
    pylab.xlim(0,fh * 2);
    pylab.ylim(-40,5);

    f1.plot();
    pylab.xlim(0,fh * 2);
    pylab.ylim(-40,5);

    f11.plot();
    pylab.xlim(0,fh * 2);
    pylab.ylim(-40,5);

    cout << Toc() << " seconds" << endl << flush;

    Plotter::show();

    return 0;
}


