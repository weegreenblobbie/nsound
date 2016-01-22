//-----------------------------------------------------------------------------
//
//  $Id: Main.cc 874 2014-09-08 02:21:29Z weegreenblobbie $
//
//  Copyright (c) 2005-2006 Nick Hilton
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

#include <Nsound/Plotter.h>

#include "UnitTest.h"

#include <iostream>

using std::cerr;
using std::cout;
using std::endl;
using std::flush;

int main(int argc, char ** argv)
{
    cout << "////////////////////////////////////////////////////////////"
         << endl
         << "// Testing the Nsound release ..."
         << endl
         << flush;

    Wavefile_UnitTest();

    Buffer_UnitTest();
    BufferResample_UnitTest();

    DelayLine_UnitTest();

    FilterDelay_UnitTest();

    FilterCombLowPassFeedback_UnitTest();

    FilterLeastSquaresFIR_UnitTest();

    FilterParametricEqualizer_UnitTest();

    Generator_UnitTest();
    Sine_UnitTest();
    Triangle_UnitTest();

    FFTransform_UnitTest();

    Nsound::Plotter::show();

    cout << endl
         << "All Unit Tests Passed!"
         << endl;

    return 0;
}
