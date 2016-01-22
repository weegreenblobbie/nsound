//-----------------------------------------------------------------------------
//
//  $Id: test_plotter.cc 874 2014-09-08 02:21:29Z weegreenblobbie $
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

#include <Nsound/AudioStream.h>
#include <Nsound/Buffer.h>
#include <Nsound/Nsound.h>
#include <Nsound/Generator.h>
#include <Nsound/Plotter.h>
#include <Nsound/Sine.h>

#include <cmath>
#include <iostream>

using namespace Nsound;

using std::cerr;
using std::cout;
using std::endl;

void testBuffer();

int main(int argc, char ** argv)
{
    Generator generator(5000);
    Sine sine(5000);

    AudioStream as(5000, 2);

    // Draw some signals.
    as[0] = sine.drawGaussian(2.0, 1.0, 0.10);
    as[1] = sine.drawFatGaussian(2.0, 0.3);

    Plotter pylab;

    // Plot a buffer in blue.
    pylab.plot(as[0],"b");

    // Plot a buffer in red.
    pylab.plot(as[1],"r");

    // Set the title.
    pylab.title("blue = default gaussian, red = gaussian with 30% pass band");

    // Set the axis labels.
    pylab.xlabel("sample");
    pylab.ylabel("amplitude");

    // Creating a plot with two subplots
    Buffer x_axis = sine.drawLine(2.0, 0.0, 2.0);

    pylab.figure();
    pylab.subplot(2,1,1);
    pylab.title("default gaussian");
    pylab.plot(x_axis, as[0], "b");
    pylab.ylabel("amplitude");

    pylab.subplot(2,1,2);
    pylab.title("gaussian with 30% pass band");
    pylab.plot(x_axis, as[1], "r");
    pylab.xlabel("seconds");
    pylab.ylabel("amplitude");

    // simple plot of AudioStream
    as.plot("quick plot from AudioStream");

    // simple plots of Buffers
    as[1].plot("quick plot, gaussian with 30% pass band");
    as[0].plot("quick plot, default gaussian");

    Plotter::show();

    return 0;
}


