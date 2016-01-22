//-----------------------------------------------------------------------------
//
//  $Id: Generator_UnitTest.cc 874 2014-09-08 02:21:29Z weegreenblobbie $
//
//  Copyright (c) 2008 Nick Hilton
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

// The __FILE__ macro includes the path, I don't want the whole path.
static const char * THIS_FILE = "Generator_UnitTest.cc";

static const float64 GAMMA = 8e-7;

void Generator_UnitTest()
{
    cout << endl << THIS_FILE;

    Wavefile::setDefaultSampleSize(32);
    Wavefile::setIEEEFloat(true);

    Generator gen(100);
    Cosine cos(100);

    // Test 7 Hz wave
    cout << TEST_HEADER << "Testing Generator::drawLine(1.0, 1.0, 0.0) ...";

    Buffer data = gen.drawLine(1.0, 1.0, 0.0);

    // Create the gold file
//~    data >> "gold/Generator_line1.wav";

    Buffer gold;

    gold << "gold/Generator_line1.wav";

    Buffer diff = data - gold;

    Buffer abs_diff(diff);
    abs_diff.abs();

    if(abs_diff.getMax() > GAMMA)
    {
        cerr << TEST_ERROR_HEADER
             << "Output did not match gold file!"
             << endl;

        data.plot("data");
        gold.plot("gold");
        diff.plot("data - gold");
        Plotter::show();

        exit(1);
    }

    cout << SUCCESS;

    // Test 3.5 Hz wave
    cout << TEST_HEADER << "Testing Generator::drawLine(1.26, -0.26, 0.26) ...";

    data = gen.drawLine(1.26, -0.26, 0.26);

    // Create the gold file
//~    data >> "gold/Generator_line2.wav";

    gold = Buffer();

    gold << "gold/Generator_line2.wav";

    diff = data - gold;

    abs_diff = diff;
    abs_diff.abs();

    if(abs_diff.getMax() > GAMMA)
    {
        cerr << TEST_ERROR_HEADER
             << "Output did not match gold file!"
             << endl;

        data.plot("data");
        gold.plot("gold");
        diff.plot("data - gold");
        Plotter::show();

        exit(1);
    }

    cout << SUCCESS;


    // Test 3.5 Hz wave
    cout << TEST_HEADER << "Testing Generator::buzz(1.0, 4.0, 3, 0.0) ...";

    data = cos.buzz(1.0, 4.0, 3, 0.0);

    // Create the gold file
//~    data >> "gold/Generator_buzz1.wav";

    gold = Buffer();

    gold << "gold/Generator_buzz1.wav";

    diff = data - gold;

    abs_diff = diff;
    abs_diff.abs();

    if(abs_diff.getMax() > GAMMA)
    {
        cerr << TEST_ERROR_HEADER
             << "Output did not match gold file!"
             << endl;

        data.plot("data");
        gold.plot("gold");
        diff.plot("data - gold");
        Plotter::show();

        exit(1);
    }

    cout << SUCCESS;


    // Test 3.5 Hz wave
    cout << TEST_HEADER << "Testing Generator::buzz(1.0, 4.0, 3, 0.5) ...";

    data = cos.buzz(1.0, 4.0, 3, 0.5);

    // Create the gold file
//~    data >> "gold/Generator_buzz2.wav";

    gold = Buffer();

    gold << "gold/Generator_buzz2.wav";

    diff = data - gold;

    abs_diff = diff;
    abs_diff.abs();

    if(abs_diff.getMax() > GAMMA)
    {
        cerr << TEST_ERROR_HEADER
             << "Output did not match gold file!"
             << endl;

        data.plot("data");
        gold.plot("gold");
        diff.plot("data - gold");
        Plotter::show();

        exit(1);
    }

    cout << SUCCESS;

    // Test 3.5 Hz wave
    cout << TEST_HEADER << "Testing Generator::buzz(1.0, 4.0, 4, 0.5) ...";

    data = cos.buzz(1.0, 4.0, 4, 0.5);

    // Create the gold file
//~    data >> "gold/Generator_buzz3.wav";

    gold = Buffer();

    gold << "gold/Generator_buzz3.wav";

    diff = data - gold;

    abs_diff = diff;
    abs_diff.abs();

    if(abs_diff.getMax() > GAMMA)
    {
        cerr << TEST_ERROR_HEADER
             << "Output did not match gold file!"
             << endl;

        data.plot("data");
        gold.plot("gold");
        diff.plot("data - gold");
        Plotter::show();

        exit(1);
    }

    cout << SUCCESS;

    // Test drawSine
    cout << TEST_HEADER << "Testing Generator::drawSine(1.0, 10.0) ...";

    gen = Generator(600.0);

    data = Buffer();
    data << gen.drawSine(1.0, 10.0);

    // Create the gold file
//~    data >> "gold/Generator_sine1.wav";

    gold = Buffer("gold/Generator_sine1.wav");

    diff = data - gold;

    abs_diff = diff;
    abs_diff.abs();

    if(abs_diff.getMax() > GAMMA)
    {
        cerr << TEST_ERROR_HEADER
             << "Output did not match gold file!"
             << endl;

        data.plot("data");
        gold.plot("gold");
        diff.plot("data - gold");
        Plotter::show();

        exit(1);
    }

    cout << SUCCESS;

    // Test drawSine
    cout << TEST_HEADER << "Testing Generator::drawSine(1.0, freqs) ...";

    data = Buffer();

    Buffer freqs(gen.drawLine(1.0, 1.0, 10.0));

    data << gen.drawSine(1.0, freqs);

    // Create the gold file
//~    data >> "gold/Generator_sine2.wav";

    gold = Buffer("gold/Generator_sine2.wav");

    diff = data - gold;

    abs_diff = diff;
    abs_diff.abs();

    if(abs_diff.getMax() > GAMMA)
    {
        cerr << TEST_ERROR_HEADER
             << "Output did not match gold file!"
             << endl;

        data.plot("data");
        gold.plot("gold");
        diff.plot("data - gold");
        Plotter::show();

        exit(1);
    }

    cout << SUCCESS;

    // Test drawSine
    cout << TEST_HEADER << "Testing Generator::drawSine2(1.0, 3.0, 0.5) ...";

    data = Buffer();

    data << gen.drawSine2(1.0, 3.0, 0.5);

    // Create the gold file
//~    data >> "gold/Generator_sine3.wav";

    gold = Buffer("gold/Generator_sine3.wav");

    diff = data - gold;

    abs_diff = diff;
    abs_diff.abs();

    if(abs_diff.getMax() > GAMMA)
    {
        cerr << TEST_ERROR_HEADER
             << "Output did not match gold file!"
             << endl;

        data.plot("data");
        gold.plot("gold");
        diff.plot("data - gold");
        Plotter::show();

        exit(1);
    }

    cout << SUCCESS;

    // Test drawSine
    cout << TEST_HEADER << "Testing Generator::drawSine2(1.0, 3.0, phase) ...";

    data = Buffer();

    Buffer phase;
    phase << gen.drawLine(1.0, 0.0, 1.0);

    data << gen.drawSine2(1.0, 3.0, phase);

    // Create the gold file
//~    data >> "gold/Generator_sine4.wav";

    gold = Buffer("gold/Generator_sine4.wav");

    diff = data - gold;

    abs_diff = diff;
    abs_diff.abs();

    if(abs_diff.getMax() > GAMMA)
    {
        cerr << TEST_ERROR_HEADER
             << "Output did not match gold file!"
             << endl;

        data.plot("data");
        gold.plot("gold");
        diff.plot("data - gold");
        Plotter::show();

        exit(1);
    }

    cout << SUCCESS;

    // Test drawSine
    cout << TEST_HEADER << "Testing Generator::drawSine2(1.0, freqs, 0.5) ...";

    data = Buffer();
    freqs = Buffer();

    freqs << gen.drawLine(1.0, 0.0, 10.0);

    data << gen.drawSine2(1.0, freqs, 0.5);

    // Create the gold file
//~    data >> "gold/Generator_sine5.wav";

    gold = Buffer("gold/Generator_sine5.wav");

    diff = data - gold;

    abs_diff = diff;
    abs_diff.abs();

    if(abs_diff.getMax() > GAMMA)
    {
        cerr << TEST_ERROR_HEADER
             << "Output did not match gold file!"
             << endl;

        data.plot("data");
        gold.plot("gold");
        diff.plot("data - gold");
        Plotter::show();

        exit(1);
    }

    cout << SUCCESS;

    // Test drawSine
    cout << TEST_HEADER << "Testing Generator::drawSine2(1.0, freqs, phase) ...";

    data = Buffer();

    data << gen.drawSine2(1.0, freqs, phase);

    // Create the gold file
//~    data >> "gold/Generator_sine6.wav";

    gold = Buffer("gold/Generator_sine6.wav");

    diff = data - gold;

    abs_diff = diff;
    abs_diff.abs();

    if(abs_diff.getMax() > GAMMA)
    {
        cerr << TEST_ERROR_HEADER
             << "Output did not match gold file!"
             << endl;

        data.plot("data");
        gold.plot("gold");
        diff.plot("data - gold");
        Plotter::show();

        exit(1);
    }

    cout << SUCCESS;

    // Test chorus
    cout << TEST_HEADER << "Testing Generator::generate(1.0, freq) with chorus ...";

    data = Buffer();

    cos.setSeed(457745);

    cos.setChorus(2, 0.50);

    data << cos.generate(1.0, 5);

    // Create the gold file
//~    data >> "gold/Generator_chorus1.wav";

//~    // Force a failure
//~    data[data.getLength() / 2] += data.getMax();

    gold = Buffer("gold/Generator_chorus1.wav");

    diff = data - gold;

    abs_diff = diff;
    abs_diff.abs();

    if(abs_diff.getMax() > GAMMA)
    {
        cerr << TEST_ERROR_HEADER
             << "Output did not match gold file!"
             << endl;

        Plotter pylab;
        pylab.plot(data, "r", "label='data'");
        pylab.plot(gold, "b", "label='gold', lw=2");
        pylab.grid(true);
        pylab.legend();

        pylab.show();

        exit(1);
    }

    cout << SUCCESS;


    // Finish
    cout << endl;

    Plotter::show();
}


