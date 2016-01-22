//-----------------------------------------------------------------------------
//
//  $Id: test_pluck.cc 874 2014-09-08 02:21:29Z weegreenblobbie $
//
//  Copyright (c) 2005-2007 Nick Hilton
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

#include <math.h>
#include <iostream>

using namespace Nsound;

using std::cerr;
using std::cout;
using std::endl;
using std::flush;

#define THIS_FILE "test_pluck.cc"

int main(int argc, char ** argv)
{
    Pluck pluck(44100, 1024);

    AudioStream output(44100, 1);

    cout << TEST_HEADER
         << "pluck.generate() ..." << flush;

    Tic();

    Buffer temp = pluck.generate(2.0, 246.94);

    cout << " " << Toc() << " seconds" << endl << flush;

    output << temp;
    output << temp.getReverse() << pluck.silence(1.0);

    cout << TEST_HEADER
         << "pluck.generate(): plucking 6 strings: guitar chord E ..."
         << flush;

    Tic();

    Buffer string6 = pluck.generate(2.0, 82.41);  // E2         C4 = middle C
    Buffer string5 = pluck.generate(2.0, 123.47); // B2
    Buffer string4 = pluck.generate(2.0, 164.81); // E3
    Buffer string3 = pluck.generate(2.0, 207.65); // G3#
    Buffer string2 = pluck.generate(2.0, 246.94); // B3
    Buffer string1 = pluck.generate(2.0, 329.63); // E4

    // add in each string with a little delay

    uint32 offset = static_cast<uint32>(0.150 * 44100.0);

    Buffer final(string6);

    final.add(string5, offset    );
    final.add(string4, offset * 2);
    final.add(string3, offset * 3);
    final.add(string2, offset * 4);
    final.add(string1, offset * 5);

    final.normalize();

    cout << " " << Toc() << " seconds" << endl << flush;

    output << final;
    output << final.getReverse()
           << pluck.silence(0.5);

    output >> "test_pluck.wav";

    return 0;
}
