//-----------------------------------------------------------------------------
//
//  $Id: BufferWindowSearch.cc 874 2014-09-08 02:21:29Z weegreenblobbie $
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

#include <Nsound/Buffer.h>
#include <Nsound/BufferWindowSearch.h>

#include <Nsound/Plotter.h>

#include <cmath>
#include <iostream>

using namespace Nsound;

using std::cerr;
using std::endl;

//-----------------------------------------------------------------------------
BufferWindowSearch::
BufferWindowSearch(const Buffer & target, uint32 window_size)
    :
    target_buffer_(&target),
    window_size_(window_size),
    position_(0)
{
    M_ASSERT_VALUE(window_size_, >=, 2);
};

//-----------------------------------------------------------------------------
BufferWindowSearch::
BufferWindowSearch(const BufferWindowSearch & copy)
    :
    target_buffer_(copy.target_buffer_),
    window_size_(copy.window_size_),
    position_(copy.position_)
{
};

//-----------------------------------------------------------------------------
BufferWindowSearch &
BufferWindowSearch::
operator=(const BufferWindowSearch & rhs)
{
    if(this == &rhs)
    {
        return *this;
    }

    target_buffer_ = rhs.target_buffer_;
    window_size_   = rhs.window_size_;
    position_      = rhs.position_;

    return *this;
};

Buffer
BufferWindowSearch::
getNextWindow()
{
    enum State
    {
        INIT,
        POS,
        NEG,
        END,
        DONE
    };

    //~    std::string STRINGS[5] = { "init", "pos", "neg", "end", "done" };

    State s = INIT;

    uint32 next_pos = position_ + window_size_;

    //~    Plotter pylab;
    //~
    //~    boolean debug_plot = false;
    //~
    //~    if(next_pos == 0)
    //~    {
    //~        Buffer x_axis;
    //~        for(uint32 i = next_pos; i < next_pos + 100; ++i)
    //~        {
    //~            x_axis << i;
    //~        }
    //~
    //~        pylab.plot(x_axis, target_buffer_->subbuffer(next_pos, 100));
    //~
    //~        pylab.title("window 26303");
    //~
    //~        pylab.hold(true);
    //~
    //~        debug_plot = true;
    //~    }

    //~    cerr << "position_ = " << position_
    //~         << "    start = " << next_pos;

    uint32 end = target_buffer_->getLength();

    // Are we off the end?
    if(next_pos >= end)
    {
        Buffer w = target_buffer_->subbuffer(position_);
        position_ = end;
        return w;
    }

    // DEBUG
    Buffer x(1);
    Buffer y(1);
    x << next_pos;
    y << (*target_buffer_)[next_pos];


    // Initalize the state.
    if((*target_buffer_)[next_pos] > 0.0)
    {
        s = POS;
        //~        if(debug_plot) pylab.plot(x,y, "g+");
    }
    else if((*target_buffer_)[next_pos] < 0.0)
    {
        s = NEG;
        //~        if(debug_plot) pylab.plot(x,y, "r+");
    }
    else
    {
        s = DONE;
    }

    //~    cerr << "    state = " << STRINGS[s];

    State last_s = s;

    while(last_s == s)
    {
        last_s = s;

        ++next_pos;

        //~        if(next_pos > position_ + window_size_ + 200 ) debug_plot = false;
        //~
        //~        // DEBUG
        //~        x = Buffer(1);
        //~        y = Buffer(1);
        //~        x << next_pos;
        //~        y << (*target_buffer_)[next_pos];

        // Are we off the end?
        if(next_pos >= end)
        {
            Buffer w = target_buffer_->subbuffer(position_);
            position_ = end;
            return w;
        }

        // Update state.
        if((*target_buffer_)[next_pos] > 0.0)
        {
            s = POS;
            //~            if(debug_plot) pylab.plot(x,y, "g+");
        }
        else if((*target_buffer_)[next_pos] < 0.0)
        {
            s = NEG;
            //~            if(debug_plot) pylab.plot(x,y, "r+");
        }
        else
        {
            break;
        }
    }

    // Pick the sample that is closest to zero.
    if( std::fabs((*target_buffer_)[next_pos-1]) <
        std::fabs((*target_buffer_)[next_pos]))
    {
        --next_pos;
    }

    //~    cerr << "    state = " << STRINGS[s];
    //~    cerr << "    n_samples_ = " << next_pos - position_ << endl;

    Buffer w = target_buffer_->subbuffer(position_, next_pos - position_);

    position_ = next_pos;

    return w;
}

uint32
BufferWindowSearch::
getSamplesLeft() const
{
    return target_buffer_->getLength() - position_;
}
