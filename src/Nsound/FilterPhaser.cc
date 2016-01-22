//-----------------------------------------------------------------------------
//
//  $Id: FilterPhaser.cc 874 2014-09-08 02:21:29Z weegreenblobbie $
//
//  Copyright (c) 2010 to Present Nick Hilton
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
#include <Nsound/FilterAllPass.h>
#include <Nsound/FilterPhaser.h>
#include <Nsound/Plotter.h>
#include <Nsound/Sine.h>

#include <stdio.h>
#include <iostream>

using namespace Nsound;

using std::cerr;
using std::cout;
using std::endl;

//-----------------------------------------------------------------------------
FilterPhaser::
FilterPhaser(
    const float64 & sample_rate,
    const uint32 n_stages,
    const float64 & frequency,
    const float64 & frequency_step_per_stage,
    const float64 & max_delay_time_seconds)
    :
    Filter(sample_rate),
    n_stages_(n_stages),
    max_delay_(max_delay_time_seconds),
    filters_(NULL),
    frequencies_(),
    waveform_(NULL),
    waveform_position_()
{
    M_ASSERT_VALUE(n_stages_, >, 0);
    M_ASSERT_VALUE(frequency, >, 0.0);
    M_ASSERT_VALUE(frequency_step_per_stage, >, 0.0);
    M_ASSERT_VALUE(max_delay_, >, 0.0);

    float64 f = frequency;

    if(f <= 0.0) f = 1.0;

    float64 fstep = frequency_step_per_stage;

    if(fstep <= 0.0) fstep = 1.0 / static_cast<float64>(n_stages_);

    if(max_delay_ <= 0.0) max_delay_ = 0.1;

    filters_= new FilterAllPass * [n_stages_];

    waveform_ = new Buffer(static_cast<uint32>(sample_rate_));

    Sine sin(sample_rate_);

    *waveform_ = (1.0 + sin.generate(1.0, 1.0)) / 2.0;

    for(uint32 i = 0; i < n_stages_; ++i)
    {
        filters_[i] = new FilterAllPass(sample_rate_, max_delay_, 0.5);
        waveform_position_.push_back(0.0);
        frequencies_.push_back(f + fstep * static_cast<float64>(i));
    }
}

//-----------------------------------------------------------------------------
FilterPhaser::
FilterPhaser(const FilterPhaser & copy)
    :
    Filter(copy.sample_rate_),
    n_stages_(copy.n_stages_),
    max_delay_(copy.max_delay_),
    filters_(NULL),
    frequencies_(),
    waveform_(NULL),
    waveform_position_(copy.waveform_position_)
{
    filters_= new FilterAllPass * [n_stages_];

    waveform_ = new Buffer(static_cast<uint32>(sample_rate_));

    for(uint32 i = 0; i < n_stages_; ++i)
    {
        filters_[i] = new FilterAllPass(sample_rate_, max_delay_, 0.5);
        waveform_position_.push_back(0.0);
    }

    *this = copy;
}

//-----------------------------------------------------------------------------
FilterPhaser::
~FilterPhaser()
{
    for(uint32 i = 0; i < n_stages_; ++i)
    {
        delete filters_[i];
    }

    delete [] filters_;

    delete waveform_;
}

AudioStream
FilterPhaser::
filter(const AudioStream & x)
{
    return Filter::filter(x);
}

Buffer
FilterPhaser::
filter(const Buffer & x)
{
    return Filter::filter(x);
}

float64
FilterPhaser::
filter(const float64 & x)
{
    float64 y = 0.0;

    for(uint32 i = 0; i < n_stages_; ++i)
    {
        float64 pos = waveform_position_[i]
                    + frequencies_[i];

        if(pos >= sample_rate_) pos -= sample_rate_;

        waveform_position_[i] = pos;

        float64 f = (*waveform_)[static_cast<uint32>(pos)];

        y += filters_[i]->filter(x, f * max_delay_);
    }

    y /= static_cast<float64>(n_stages_);

    return (y+x) / 2.0;
}

///////////////////////////////////////////////////////////////////////////
FilterPhaser &
FilterPhaser::
operator=(const FilterPhaser & rhs)
{
    if(this == &rhs)
    {
        return *this;
    }

    sample_rate_       = rhs.sample_rate_;
    max_delay_         = rhs.max_delay_;
    *waveform_         = *rhs.waveform_;
    frequencies_       = rhs.frequencies_;
    waveform_position_ = rhs.waveform_position_;

    if(n_stages_ != rhs.n_stages_)
    {
        for(uint32 i = 0; i < n_stages_; ++i)
        {
            delete filters_[i];
        }

        delete [] filters_;

        n_stages_ = rhs.n_stages_;

        filters_ = new FilterAllPass * [n_stages_];

        for(uint32 i = 0; i < n_stages_; ++i)
        {
            filters_[i] = new FilterAllPass(sample_rate_, max_delay_, 0.5);
        }
    }

    for(uint32 i = 0; i < n_stages_; ++i)
    {
        *filters_[i] = *rhs.filters_[i];
    }

    return *this;
}

void
FilterPhaser::
plot(boolean show_fc, boolean show_phase)
{
    char title[128];
    sprintf(title,
        "Phaser Frequency Response\n"
        "sr = %0.1f Hz, f = %0.1f Hz, delay = %0.3f ms",
        sample_rate_,
        frequencies_[0],
        max_delay_ * 1000.0);

    Filter::plot(show_phase);

    Plotter pylab;

    uint32 n_rows = 1;

    if(show_phase)
    {
        n_rows = 2;
    }

    if(show_fc)
    {
        pylab.subplot(n_rows, 1, 1);
    }

    pylab.title(title);
}

void
FilterPhaser::
reset()
{
    for(uint32 i = 0; i < n_stages_; ++i)
    {
        filters_[i]->reset();
        waveform_position_[i] = 0.0;
    }
}
