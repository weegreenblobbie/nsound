//-----------------------------------------------------------------------------
//
//  $Id: Hat.cc 874 2014-09-08 02:21:29Z weegreenblobbie $
//
//  Nsound is a C++ library and Python module for audio synthesis featuring
//  dynamic digital filters. Nsound lets you easily shape waveforms and write
//  to disk or plot them. Nsound aims to be as powerful as Csound but easy to
//  use.
//
//  Copyright (c) 2009-Present Nick Hilton
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
#include <Nsound/Hat.h>
#include <Nsound/FilterHighPassIIR.h>
#include <Nsound/Square.h>

using namespace Nsound;
using std::cout;
using std::endl;

//-----------------------------------------------------------------------------
Hat::
Hat(const float64 & sample_rate)
    :
    Instrument(sample_rate),
    square_(NULL),
    hpf_(NULL)
{
    square_ = new Square(sample_rate_);
    hpf_ = new FilterHighPassIIR(sample_rate_, 2, 4872.0, 0.05);
}

//-----------------------------------------------------------------------------
Hat::
Hat(const Hat & rhs)
    :
    Instrument(rhs.sample_rate_),
    square_(NULL),
    hpf_(NULL)
{
    square_ = new Square(*rhs.square_);
    hpf_ = new FilterHighPassIIR(*rhs.hpf_);
}

//-----------------------------------------------------------------------------
Hat::
~Hat()
{
    delete square_;
    delete hpf_;
}

//-----------------------------------------------------------------------------
Hat &
Hat::
operator=(const Hat & rhs)
{
    delete square_;
    delete hpf_;

    sample_rate_ = rhs.sample_rate_;
    square_ = new Square(*rhs.square_);
    hpf_ = new FilterHighPassIIR(*rhs.hpf_);

    return *this;
}

AudioStream
Hat::
play()
{
    AudioStream y(sample_rate_, 1);

    float64 offset = 0.124;

    y << play(1.0, 1.0, true);

    //        duration, tune, is_open
//~    y.add(play(1.0,      1.0,  true),  0 * offset);
    y.add(play(1.0,      1.0,  true),  1 * offset);
    y.add(play(1.0,      1.0,  false), 2 * offset);
    y.add(play(1.0,      1.0,  true) , 3 * offset);

    y.add(play(1.0,      1.0,  true) , 4 * offset);
    y.add(play(1.0,      1.0,  true) , 5 * offset);
    y.add(play(1.0,      1.0,  true) , 6  * offset);
    y.add(play(1.0,      1.0,  false), 7  * offset);

    y.add(play(1.0,      1.0,  true) , 8  * offset);
    y.add(play(1.0,      1.0,  true) , 9  * offset);
    y.add(play(1.0,      1.0,  false), 10 * offset);
    y.add(play(1.0,      1.0,  true) , 11 * offset);

    y.add(play(1.0,      1.0,  true) , 12 * offset);
    y.add(play(1.0,      1.0,  true) , 13 * offset);
    y.add(play(1.0,      1.0,  true) , 14 * offset);
    y.add(play(1.0,      1.0,  false), 15 * offset);

    y.add(play(1.0,      1.0,  true) , 16 * offset);
    y.add(play(1.0,      1.0,  true) , 17 * offset);
    y.add(play(1.0,      1.0,  false), 18 * offset);
    y.add(play(1.0,      1.0,  true) , 19 * offset);

    y.add(play(1.0,      1.0,  true) , 20 * offset);
    y.add(play(1.0,      1.0,  true) , 21 * offset);
    y.add(play(1.0,      1.0,  true) , 22 * offset);
    y.add(play(1.0,      1.0,  false), 23 * offset);

    y.add(play(1.0,      0.9,  true) , 24 * offset);
    y.add(play(1.0,      0.8,  true) , 25 * offset);
    y.add(play(1.0,      0.7,  true) , 26 * offset);
    y.add(play(1.0,      0.6,  true) , 27 * offset);

    y.add(play(1.0,      0.5,  true) , 28 * offset);
    y.add(play(1.0,      0.4,  true) , 29 * offset);
    y.add(play(1.0,      0.3,  true) , 30 * offset);
    y.add(play(1.0,      0.2,  true) , 31 * offset);

    return y;
}

#include <Nsound/Plotter.h>

AudioStream
Hat::
play(
    const float64 & duration,
    const float64 & tune,
    boolean is_closed)
{
    float64 play_duration = duration;
    float64 silence_duration = 0.0;

    float64 env_offset = 0.55 * play_duration;

    if(is_closed)
    {
        play_duration *= 0.06;
        silence_duration = duration - play_duration;
        env_offset = 0.0;
    }

    float64 frequency = tune * 530.0;

    Buffer env = square_->drawLine(0.0005, 0.1, 1.0)
              << square_->drawDecay(play_duration-0.0005 - env_offset, 7.0)
              << square_->silence(silence_duration + env_offset);

    AudioStream out(sample_rate_, 1);

    out << square_->generate(play_duration, frequency);

    out += square_->generate(play_duration, frequency * 1.5038);
    out += square_->generate(play_duration, frequency * 1.6132);
    out += square_->generate(play_duration, frequency * 1.9642);
    out += square_->generate(play_duration, frequency * 2.5321);
    out += square_->generate(play_duration, frequency * 2.7547);

    out /= 2.0;

    out = hpf_->filter(out);
    out = hpf_->filter(out);

    return out * env;
}

// :mode=c++: jEdit modeline
