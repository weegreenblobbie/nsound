//-----------------------------------------------------------------------------
//
//  $Id: test_filters.cc 874 2014-09-08 02:21:29Z weegreenblobbie $
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

#include <Nsound/NsoundAll.h>

#include "Test.h"

#include <iostream>

using namespace Nsound;

using std::cout;
using std::endl;
using std::flush;

static const char * THIS_FILE = "test_filters.cc";

int main(int argc, char ** argv)
{
    Sine sin(44100);

    Buffer frequency_sweep = sin.drawLine(5.0, 0.0, 1720.0);

    AudioStream output(44100, 1);
    AudioStream input(44100, 1);

    input = sin.generate(5.0, frequency_sweep);

    input >> "filters_input.wav";

    Buffer low_freqs = 200.0 * sin.generate(5.0, 1.0) + frequency_sweep;

    Buffer high_freqs = 200.0 + low_freqs;

    cout << TEST_HEADER
         << "testing FilterLowPassFIR ... " << flush;

    FilterLowPassFIR f1(44100, 256, 880.0);

    Tic();

    output = f1.filter(input);

    cout << Toc() << " seconds"
         << endl;

    output >> "filters_fir_low_pass.wav";

    cout << TEST_HEADER
         << "testing FilterLowPassIIR ... " << flush;

    Tic();

    FilterLowPassIIR f2(44100, 4, 880.0, 0.01);

    output = f2.filter(input);

    cout << Toc() << " seconds"
         << endl;

    output >> "filters_iir_low_pass.wav";

    cout << TEST_HEADER
         << "testing FilterLowPassFIR dynamic filtering... " << flush;

    Tic();

    output = f1.filter(input, low_freqs);

    cout << Toc() << " seconds"
         << endl;

    output >> "filters_fir_low_pass_dynamic.wav";

    cout << TEST_HEADER
         << "testing FilterLowPassIIR dynamic filtering... " << flush;

    Tic();

    output = f2.filter(input, low_freqs);

    cout << Toc() << " seconds"
         << endl;

    output >> "filters_iir_low_pass_dynamic.wav";

    cout << TEST_HEADER
        << "testing FilterHighPassFIR ... " << flush;

    FilterHighPassFIR f3(44100, 256, 880.0);

    Tic();
    output = f3.filter(input);

    cout << Toc() << " seconds"
         << endl;

    output >> "filters_fir_high_pass.wav";

    cout << TEST_HEADER
        << "testing FilterHighPassIIR ... " << flush;

    FilterHighPassIIR f4(44100, 4, 880.0, 0.01);

    Tic();
    output = f4.filter(input);

    cout << Toc() << " seconds"
         << endl;

    output >> "filters_iir_high_pass.wav";

    cout << TEST_HEADER
         << "testing FilterHighPassFIR dynamic filtering... " << flush;

    Tic();
    output = f3.filter(input, high_freqs);

    cout << Toc() << " seconds"
         << endl;

    output >> "filters_fir_high_pass_dynamic.wav";

    cout << TEST_HEADER
         << "testing FilterHighPassIIR dynamic filtering... " << flush;

    Tic();
    output = f4.filter(input, high_freqs);

    cout << Toc() << " seconds"
         << endl;

    output >> "filters_iir_high_pass_dynamic.wav";

    cout << TEST_HEADER
        << "testing FilterBandRejectFIR ... " << flush;

    FilterBandRejectFIR f5(44100, 512, 450.0, 870.0);

    Tic();
    output = f5.filter(input);

    cout << Toc() << " seconds"
         << endl;

    output >> "filters_fir_band_reject.wav";

    cout << TEST_HEADER
        << "testing FilterBandRejectIIR ... " << flush;

    FilterBandRejectIIR f6(44100, 4, 440.0, 880.0, 0.01);

    Tic();
    output = f6.filter(input);

    cout << Toc() << " seconds"
         << endl;

    output >> "filters_iir_band_reject.wav";

    cout << TEST_HEADER
         << "testing FilterBandRejectFIR dynamic filtering... " << flush;

    Tic();

    output = f5.filter(input, low_freqs, high_freqs);

    cout << Toc() << " seconds"
         << endl;

    output >> "filters_fir_band_reject_dynamic.wav";

    cout << TEST_HEADER
         << "testing FilterBandRejectIIR dynamic filtering... " << flush;

    Tic();

    output = f6.filter(input, low_freqs, high_freqs);

    cout << Toc() << " seconds"
         << endl;

    output >> "filters_iir_band_reject_dynamic.wav";

    cout << TEST_HEADER
         << "testing FilterBandPassFIR ... " << flush;

    FilterBandPassFIR f7(44100, 512, 440, 880);

    Tic();
    output = f7.filter(input);

    cout << Toc() << " seconds"
         << endl;

    output >> "filters_fir_band_pass.wav";

    cout << TEST_HEADER
        << "testing FilterBandPassIIR ... " << flush;

    FilterBandPassIIR f8(44100, 4, 440.0, 880.0, 0.01);

    Tic();
    output = f8.filter(input);

    cout << Toc() << " seconds"
         << endl;

    output >> "filters_iir_band_pass.wav";

    cout << TEST_HEADER
         << "testing FilterBandPassFIR dynamic filtering... " << flush;

    Tic();
    output = f7.filter(input, low_freqs, high_freqs);

    cout << Toc() << " seconds"
         << endl;

    output >> "filters_fir_band_pass_dynamic.wav";

    cout << TEST_HEADER
         << "testing FilterBandPassIIR dynamic filtering... " << flush;

    Tic();
    output = f8.filter(input, low_freqs, high_freqs);

    cout << Toc() << " seconds"
         << endl;

    output >> "filters_iir_band_pass_dynamic.wav";

    return 0;
}
