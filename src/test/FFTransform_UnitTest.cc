//-----------------------------------------------------------------------------
//
//  $Id: FFTransform_UnitTest.cc 874 2014-09-08 02:21:29Z weegreenblobbie $
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
#include <Nsound/Cosine.h>
#include <Nsound/FFTransform.h>
#include <Nsound/Plotter.h>
#include <Nsound/Sine.h>
#include <Nsound/Wavefile.h>

#include "UnitTest.h"

#include <stdlib.h>
#include <iostream>

using namespace Nsound;

using std::cout;
using std::cerr;
using std::endl;
using std::flush;

static const char * THIS_FILE = "FFTransform_UnitTest.cc";

static const float64 GAMMA = 1.0e-11;

void
FFTransform_UnitTest()
{
    cout << endl << THIS_FILE;

    Wavefile::setDefaultSampleSize(64);
    Wavefile::setIEEEFloat(true);

    Cosine cos(512);

    Buffer input = cos.buzz(1.0, 8.0, 2.0, 0.0);

    FFTransform fft(512);

    FFTChunkVector vec;

    Buffer gold;

    cout << TEST_HEADER << "Testing FFTransform::fft(), ifft() ..." << flush;

    vec = fft.fft(input, 128, 0);

    if(vec.size() != 512 / 128)
    {
        cerr << TEST_ERROR_HEADER
             << "Output did not match!"
             << endl
             << "vec.size() = " << vec.size() << " != " << 512 / 128
             << endl;

        exit(1);
    }

    Buffer data = fft.ifft(vec);

//~    input.plot("input");
//~    data.plot("fft,ifft");

    if(data.getLength() != input.getLength())
    {
        cerr << TEST_ERROR_HEADER
             << "Output did not match!"
             << endl
             << "result.getLength() = " << data.getLength() << " != "
             << input.getLength()
             << endl;

        exit(1);
    }

    // Create gold file
//~    data >> "gold/FFTransform_out1.wav";

    gold = Buffer("gold/FFTransform_out1.wav");

    Buffer diff = data - gold;

    if(diff.getAbs().getMax() > GAMMA)
    {
        cerr << TEST_ERROR_HEADER
             << "Output did not match gold file!"
             << endl;

        diff.plot("input - gold");
        data.plot("data");
        gold.plot("gold");

        Plotter::show();
        exit(1);
    }

    cout << SUCCESS;

    cout << TEST_HEADER << "Testing FFTChunk::getReal(), getImaginary() ..." << flush;

    vec = fft.fft(input, 128, 0);

    Buffer real = vec[0].getReal();
    Buffer img  = vec[0].getImaginary();

    // Create gold files.
//~    real >> "gold/FFTransform_out2.wav";
//~    img  >> "gold/FFTransform_out3.wav";

    gold = Buffer("gold/FFTransform_out2.wav");

    diff = real - gold;
    diff.abs();
    if(diff.getMax() > GAMMA)
    {
        cerr << TEST_ERROR_HEADER
             << "Output did not match gold file!"
             << endl;

        diff.plot("real - gold");
        real.plot("real");
        gold.plot("gold");

        Plotter::show();
        exit(1);
    }

    gold = Buffer("gold/FFTransform_out3.wav");

    diff = img - gold;
    diff.abs();

    if(diff.getMax() > GAMMA)
    {
        cerr << TEST_ERROR_HEADER
             << "Output did not match gold file!"
             << endl;

        diff.plot("img - gold");
        img.plot("img");
        gold.plot("gold");

        Plotter::show();
        exit(1);
    }

    cout << SUCCESS;

    cout << TEST_HEADER << "Testing FFTChunk::getMagnitude(), getPhase() ..." << flush;

    vec = fft.fft(input, 128, 0);

    Buffer mag   = vec[0].getMagnitude();
    Buffer phase = vec[0].getPhase();

    // Create gold files.
//~    mag >> "gold/FFTransform_out4.wav";
//~    phase >> "gold/FFTransform_out5.wav";

    gold = Buffer("gold/FFTransform_out4.wav");

    diff = mag - gold;

    if(diff.getAbs().getMax() > GAMMA)
    {
        cerr << TEST_ERROR_HEADER
             << "Output did not match gold file!"
             << endl;

        diff.plot("data - gold");
        mag.plot("data");
        gold.plot("gold");

        Plotter::show();
        exit(1);
    }

    gold = Buffer("gold/FFTransform_out5.wav");

    diff = phase - gold;

    diff(diff > 2.0 * M_PI - 0.01) = 0.0;
    diff(diff < 2.0 * M_PI - 0.01) = 0.0;

    if(diff.getAbs().getMax() > GAMMA)
    {
        cerr << TEST_ERROR_HEADER
             << "Output did not match gold file!"
             << endl;

        diff.plot("phase - gold");
        phase.plot("phase");
        gold.plot("gold");

        Plotter::show();
        exit(1);
    }

    cout << SUCCESS;

    cout << TEST_HEADER << "Testing FFTChunk::toCartesian(), toPolar() ..." << flush;

    vec[0].toPolar();

    mag   = vec[0].getMagnitude();
    phase = vec[0].getPhase();

    gold = Buffer("gold/FFTransform_out4.wav");

    diff = mag - gold;

    if(diff.getAbs().getMax() > GAMMA)
    {
        cerr << TEST_ERROR_HEADER
             << "Output did not match gold file!"
             << endl;

        diff.plot("data - gold");
        mag.plot("data");
        gold.plot("gold");

        Plotter::show();
        exit(1);
    }

    gold = Buffer("gold/FFTransform_out5.wav");

    diff = phase - gold;
    diff.abs();
    if(diff.getMax() > 2*M_PI)
    {
        cerr << TEST_ERROR_HEADER
             << "Output did not match gold file!"
             << endl;

        diff.plot("phase - gold");
        phase.plot("phase");
        gold.plot("gold");

        Plotter::show();
        exit(1);
    }

    cout << SUCCESS;

    cout << TEST_HEADER << "Testing FFTChunk::setCartesian(), setPolar() ..." << flush;

    vec = fft.fft(input, 512, 0);

    mag = vec[0].getMagnitude();
    phase = vec[0].getPhase();

    FFTChunk orig(vec[0]);
    FFTChunk chunk(512, 512);

    chunk.setPolar(mag, phase);

    vec[0] = chunk;

    data = fft.ifft(vec);

    diff = data - input;

    if(diff.getAbs().getMax() > 0.005)
    {
        cerr << TEST_ERROR_HEADER
             << "Output did not match gold file!"
             << endl;

        diff.plot("data - gold");
        data.plot("data");
        input.plot("gold");

        Plotter::show();
        exit(1);
    }

    chunk.setCartesian(orig.getReal(), orig.getImaginary());

    vec[0] = chunk;

    data = fft.ifft(vec);

    diff = data - input;

    if(diff.getAbs().getMax() > 2*GAMMA)
    {
        cerr << TEST_ERROR_HEADER
             << "Output did not match gold file!"
             << endl;

        diff.plot("data - gold");
        data.plot("data");
        input.plot("gold");

        Plotter::show();
        exit(1);
    }

    cout << SUCCESS;

    cout << TEST_HEADER << "Testing FFTChunk::getFrequencyAxis() ..." << flush;

    data = vec[0].getFrequencyAxis();

//~    // Create gold files.
//~    data >> "gold/FFTransform_out6.wav";

    gold = Buffer("gold/FFTransform_out6.wav");

    diff = data - gold;

    if(diff.getAbs().getMax() > 1e-16)
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
