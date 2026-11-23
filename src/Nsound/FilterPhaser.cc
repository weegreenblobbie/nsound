//-----------------------------------------------------------------------------
//
//  $Id: FilterPhaser.cc 874 2014-09-08 02:21:29Z weegreenblobbie $
//
//  Copyright (c) 2010 to Present Nick Hilton
//
//  weegreenblobbie2_gmail_com (replace '_' with '@' and '.')
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

#include <cmath>
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
    max_delay_(max_delay_time_seconds)
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

    Sine sin(sample_rate_);

    waveform_ = (1.0 + sin.generate(1.0, 1.0)) / 2.0;

    for(uint32 i = 0; i < n_stages_; ++i)
    {
        filters_.emplace_back(sample_rate_, max_delay_, 0.5);
        waveform_position_.push_back(0.0);
        frequencies_.push_back(f + fstep * static_cast<float64>(i));
    }
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

        // Wrap around the waveform however far a step goes, so the index
        // stays inside it.
        pos = std::fmod(pos, static_cast<float64>(waveform_.getLength()));

        waveform_position_[i] = pos;

        float64 f = waveform_[static_cast<uint32>(pos)];

        y += filters_[i].filter(x, f * max_delay_);
    }

    y /= static_cast<float64>(n_stages_);

    return (y+x) / 2.0;
}

void
FilterPhaser::
plot(boolean show_fc, boolean show_phase)
{
    char title[128];
    snprintf(title, sizeof(title),
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
        filters_[i].reset();
        waveform_position_[i] = 0.0;
    }
}
