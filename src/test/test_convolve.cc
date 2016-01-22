//-----------------------------------------------------------------------------
//
//  $Id: test_convolve.cc 874 2014-09-08 02:21:29Z weegreenblobbie $
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

#include <cmath>
#include <iostream>
#include <sstream>

using namespace Nsound;

using std::cerr;
using std::cout;
using std::endl;
using std::flush;

static const char * THIS_FILE = "test_convolve.cc";

int main(int argc, char ** argv)
{
    AudioStream x("../examples/california.wav");
    AudioStream h("../examples/walle.wav");

    x.resample2(8000);
    h.resample2(8000);

    cout << TEST_HEADER
         << "AudioStream::getConvolve(x) ... " << flush;

    Tic();

    AudioStream y = h.getConvolve(x[0]);

    float64 delta_t = Toc();

    y.normalize();
    y *= 0.6;

    cout << endl << "writing convolved2.wav ..." << flush;

    y >> "convolved2.wav";

    cout << " done " << delta_t << " seconds" << endl;

    return 0;
}


