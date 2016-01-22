//-----------------------------------------------------------------------------
//
//  $Id: test_granulator.cc 874 2014-09-08 02:21:29Z weegreenblobbie $
//
//  Copyright (c) 2002-2007 Nick Hilton
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
#include <Nsound/Granulator.h>
#include <Nsound/Sine.h>
#include <Nsound/TicToc.h>
#include <Nsound/Wavefile.h>

#include <cmath>
#include <iostream>

using namespace Nsound;

using std::cerr;
using std::cout;
using std::endl;
using std::flush;

int
main(int argc, char ** argv)
{
    Wavefile::setDefaultSampleRate(44100);
    Wavefile::setDefaultSampleSize(16);

    Granulator grains(44100, Granulator::GAUSSIAN_30);
    Sine sin(44100);

    Buffer grain_freqs;
    grain_freqs << 180;

    Buffer waves_per_grain;
    waves_per_grain << 2.0;

    Buffer grains_per_second = 1200 * sin.drawDecay(5.0);

    Tic();
    cout << "gran.generate() ... " << flush;

    Buffer output(4096);

    output << 0.6 * grains.generate(5, grain_freqs, waves_per_grain, grains_per_second);

    cout << Toc() << " seconds" << endl << flush;

    output >> "grains.wav";

    return 0;
}
