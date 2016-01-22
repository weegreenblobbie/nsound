//-----------------------------------------------------------------------------
//
//  $Id: test_generators.cc 874 2014-09-08 02:21:29Z weegreenblobbie $
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

#include <Nsound/NsoundAll.h>

#include <cmath>
#include <iostream>
#include <sstream>

using namespace Nsound;

using std::cerr;
using std::cout;
using std::endl;

void testBuffer();

int main(int argc, char ** argv)
{
//~    Generator generator(44100);
//~    Sine sine(44100);
//~
//~    Buffer exp = sine.drawExponential(1.0);
//~
//~    exp.plot("exp");
//~
//~    Buffer decay = sine.drawDecay(1.0);
//~
//~    decay.plot("decay");
//~
//~    Buffer g2 = sine.drawFatGaussian(2.0, 0.3);
//~
//~    g2.plot("Gaussian 30% pass band time");
//~
//~    Buffer gaussian = sine.drawGaussian(1.0);
//~
//~    gaussian.plot("Gaussian");
//~
//~    Triangle triangle2(
//~        44100,
//~        1.0*0.1428,
//~        2.0*0.1428, 1.0,
//~        3.0*0.1428,
//~        4.0*0.1428,
//~        5.0*0.1428, -1.0,
//~        6.0*0.1428);
//~
//~    Buffer p = generator.drawParabola(1.0,1.0,0.5,0.75,1.0);
//~    p << generator.drawParabola(1.0, 1.0, 0.5, 0.50, 1.0);
//~    p << generator.drawParabola(1.0, 1.0, 0.5, 0.25, 1.0);
//~    p.plot("parabola");
//~
//~    Buffer triangle_buffer = triangle2.generate(1.0,1.0);
//~    triangle_buffer.plot("triangle2(100,1*0.1666,2*0.1666, 1.0,3*0.1666,4*0.1666,5*0.1666, -1.0,6*0.1666)");
//~
//~    Triangle triangle1(44100);
//~
//~    triangle_buffer = triangle1.generate(1.0,1.0);
//~    triangle_buffer.plot("triangle");
//~
//~    Square square1(
//~        44100,
//~        0.2, 1.0,
//~        0.4,
//~        0.6, -1.0,
//~        0.8);
//~    Buffer square_buffer = 0.5 * square1.generate(1.0,1.0);
//~    square_buffer.plot("square2 (44100, 0.2, 1.0,0.4, 0.6, -1.0, 0.8) (1/2 amplitude)");
//~
//~    Square square2(44100);
//~    square_buffer = 0.5 * square2.generate(1.0,1.0);
//~    square_buffer.plot("square (1/2 amplitude)");
//~
//~    Buffer sin = sine.generate(1.0,1.0);
//~    sin.plot("sine");
//~
//~    Buffer line;
//~    line = generator.drawLine(1.0,1.0,-1.0);
//~    line.plot("line (0,1) -> (1,-1)");
//~
//~    line = generator.drawLine(1.0,0.0,-1.0);
//~    line.plot("line (0,0) -> (1,-1)");
//~
//~    line = generator.drawLine(1.0,-1.0,1.0);
//~    line.plot("line (0,-1) -> (1,1)");
//~
//~    line = generator.drawLine(1.0,0.0,1.0);
//~    line.plot("line (0,0) -> (1,1)");



    float64 sr = 44100.0;

//~    FFTransform ftr(sr);
//~
//~    ftr.setWindow(HANNING);
//~
//~//~    // Six Track
//~
//~    // Reference
//~    Buffer six_trak("../examples/six trak patch.wav");
//~
//~    FFTChunkVector vec = ftr.fft(six_trak, 4096*2*2*2);
//~
//~    Buffer six_mag = vec[0].getMagnitude().getdB();
//~    Buffer six_x   = vec[0].getFrequencyAxis();
//~
//~    Plotter pylab;
//~
//~    pylab.plot(six_x, six_mag, "r");
//~    pylab.title("six trak");
//~
//~    pylab.hold();
//~
//~//~    // Imatation
//~
//~    float64 pw = 0.0044;
//~
//~    Pulse p(sr, pw);
//~
//~    p.setChorus(5, 0.010);
//~
//~    Buffer b = p.generate(4.0, 133.750); // + std::pow(1.05946, 4.0));
//~
//~    b -= 0.5;
//~
//~    FilterLowPassIIR lpf(sr, 2, 3000.0, 0.01);
//~
//~    b = lpf.filter(b);
//~
//~    b.normalize();
//~
//~    b *= 0.66;
//~
//~    vec = ftr.fft(b.subbuffer(10000), 4096*2*2*2);
//~
//~    pylab.plot(six_x, vec[0].getMagnitude().getdB(), "b");
//~
//~    pylab.axisX(0, 6000);

//~    b >> "imitation.wav";


//~    ////////////////////////////////////////////////////////////////////
//~    // Bass Synth
//~
//~    // Reference
//~    Buffer six_trak("../examples/bass.wav");
//~
//~    FFTChunkVector vec = ftr.fft(six_trak, 4096*2);
//~
//~    Buffer six_mag = vec[0].getMagnitude().getdB();
//~    Buffer six_x   = vec[0].getFrequencyAxis();
//~
//~    Plotter pylab;
//~
//~    pylab.plot(six_x, six_mag, "r");
//~    pylab.title("six trak");
//~
//~    pylab.axisX(-10, 4000);
//~
//~    pylab.hold();
//~
//~//~    // Imatation
//~
//~    Sawtooth saw1(sr);
//~    Sawtooth saw2(sr);
//~
//~    saw1.setChorus(6, 0.01);
//~    saw2.setChorus(6, 0.01);
//~
//~    saw1.generate(1.0, 110.5);
//~    saw2.generate(1.0, 221.0);
//~
//~    Buffer b = saw1.generate(0.24, 110.5, false)
//~             + saw2.generate(0.24, 221.0, false);
//~
//~    FilterLowPassIIR lpf2(sr, 2, 4000.0, 0.01);
//~
//~    b = lpf2.filter(b);
//~
//~    b.normalize();
//~
//~    // Apply envelope.
//~    AdsrEnvelope env(sr, 0.00001, 1.25, 0.25, 0.75, 0.50, 0.0001);
//~
//~    b = env.shape(b);
//~
//~    b *= 0.66;
//~
//~    vec = ftr.fft(b, 4096*2);
//~
//~    pylab.plot(six_x, vec[0].getMagnitude().getdB(), "b");
//~
//~    pylab.axisX(-10, 4000);
//~
//~    b >> "imitation2.wav";


//~    Wavefile::setDefaultSampleRate(48000);
//~
//~    FilterFlanger flanger(48000, 2.0, 0.002);
//~    FilterPhaser  phaser(48000, 2, 2.5, 0.002, 0.0025);

//~    Buffer c("../examples/california.wav");
//~    Buffer w("../examples/walle.wav");
//~
//~    Buffer b;
//~    b << c << w;
//~
//~    Buffer out = flanger.filter(b);
//~
//~    out >> "flanger.wav";
//~
//~    out = phaser.filter(b);
//~
//~    out >> "phaser.wav";


//~    Buffer c("../examples/california.wav");
//~    Buffer walle("../examples/walle.wav");
//~
//~    BufferWindowSearch search(walle, 24030);
//~
//~    Plotter pylab;
//~
//~    walle.plot("walle.wav");
//~
//~    uint32 pos = 0;
//~
//~    std::vector<Buffer> windows;
//~
//~    while(search.getSamplesLeft() > 0)
//~    {
//~        pylab.axvline((float64)pos, "color='red'");
//~
//~        Buffer w = search.nextWindow();
//~
//~        pos += w.getLength();
//~
//~        windows.push_back(w);
//~    }
//~
//~    pylab.figure();
//~    pylab.hold(true);
//~
//~    pos = 0;
//~
//~    Buffer y;
//~
//~    for(uint32 i = 0; i < windows.size(); ++i)
//~    {
//~        pylab.axvline((float64)pos, "color='red'");
//~
//~        y << windows[i];
//~
//~        pos += windows[i].getLength();
//~    }
//~
//~    y.plot("reconstructed");
//~
//~    Buffer diff = walle - y;
//~
//~    cout << "len(walle) = " << walle.getLength() << endl;
//~    cout << "len(y)     = " << y.getLength() << endl;
//~
//~    cout << diff.getAbs().getMax() << endl;

    Wavefile::setDefaultSampleRate(48000);
    Wavefile::setIEEEFloat(true);
    Wavefile::setDefaultSampleSize(64);

    Buffer input = Buffer("../examples/california.wav")
                << Buffer("../examples/walle.wav");

    input >> "input.wav";

    Buffer data(input);

    data.resample(1.013);

//~    // Create "gold file"
//~    data >> "gold_1.013.wav";

    data >> "data_1.013.wav";

    Buffer gold("gold_1.013.wav");

    Buffer diff = gold - data;

    cout << "len(orig) = " << input.getLength() << endl
         << "len(data) = " << data.getLength() << endl
         << "len(gold) = " << gold.getLength() << endl
         << "Ratio     = " <<
            ( static_cast<float64>(data.getLength())
            / static_cast<float64>(input.getLength())) << endl

         << "Max error = " << diff.getAbs().getMax() << endl;

    diff.plot("gold - data");
    gold.plot("gold");
    input.plot("data");

    Plotter::show();

    return 0;
}


