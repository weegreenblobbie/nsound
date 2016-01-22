//-----------------------------------------------------------------------------
//
//  $Id: test_iir_ga.cc 874 2014-09-08 02:21:29Z weegreenblobbie $
//
//  Copyright (c) 2009 to Present Nick Hilton
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

#include <cmath>
#include <iostream>
#include <sstream>

using namespace Nsound;

using std::cerr;
using std::cout;
using std::endl;
using std::flush;

int main(int argc, char ** argv)
{
    Plotter pylab;

    float64 sr = 44100.0;

    FilterIIR f(sr, 15);

    Generator g(256);

    Buffer ref_response;

    // custom response
    ref_response << g.drawLine(0.25, 0.0, 0.8)
                 << g.drawLine(0.25, 0.8, 0.8)
                 << g.drawLine(0.15, 0.8, 1.4)
                 << g.drawLine(0.15, 1.4, 1.4)
                 << g.drawLine(0.20, 1.4, 0.0);

    // Some round off error will cause f_response
    while( ref_response.getLength() < 256 ) ref_response << 0.0;

    Buffer ref_freq_axis = g.drawLine(1.0, 0.0, sr/2.0);

//~    // Band Pass
//~    f_response << g.drawLine(0.2, 0.0, 0.0)
//~               << g.drawLine(0.2, 1.0, 1.0)
//~               << g.drawLine(0.6, 0.0, 0.0);

    pylab.plot(ref_freq_axis, ref_response);
    pylab.title("desired frequency response");

    Buffer evolution = f.designFrequencyResponse(ref_response, 0.001, 10000);

    cout << "f = " << endl << f << endl;

    pylab.figure();
    pylab.plot(evolution);
//~     pylab.ylim(0.0, 1.0);

    pylab.xlabel("Generation");
    pylab.ylabel("Error");

    int32 n_generations = evolution.getLength();
    float64 error = evolution[n_generations - 1];

    char buffer[1024];
    sprintf(buffer, "%5.2f RMS error in %d generations",
        error,
        n_generations);

    pylab.title(std::string(buffer));

    Buffer fr = f.getFrequencyResponse();
    Buffer faxis = f.getFrequencyAxis();

    pylab.figure();
    pylab.plot(ref_freq_axis, ref_response);
    pylab.plot(faxis, fr);
    pylab.xlabel("Frequency Hz");
    pylab.ylabel("Frequency Response");

    pylab.ylim(0.0, ref_response.getMax() * 1.10);

    sprintf(buffer, "Generation %06d", n_generations);

    pylab.title(std::string(buffer));

    pylab.xlim(0.0, faxis[fr.getLength() - 1]);

    sprintf(buffer, "Error = %.3f RMS", error);

    pylab.text(
        faxis[static_cast<uint32>(faxis.getLength() * 0.5)],
        0.25,
        std::string(buffer));

    f.plot();

    Plotter::show();


    return 0;
}


