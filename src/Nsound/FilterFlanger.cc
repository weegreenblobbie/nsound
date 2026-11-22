//-----------------------------------------------------------------------------
//
//  $Id: FilterFlanger.cc 878 2014-11-23 04:51:23Z weegreenblobbie $
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
#include <Nsound/FilterDelay.h>
#include <Nsound/FilterFlanger.h>
#include <Nsound/Generator.h>
#include <Nsound/Plotter.h>
#include <Nsound/Sine.h>
#include <Nsound/Triangle.h>


#include <stdio.h>
#include <iostream>
//~#include <string.h>

using namespace Nsound;

using std::cerr;
using std::cout;
using std::endl;

//-----------------------------------------------------------------------------
//! The sweep, a second of a sine from 0 to 1.
static
Buffer
sweep_waveform(const float64 & sample_rate)
{
    Sine sin(sample_rate);

    return (1.0 + sin.generate(1.0, 1.0)) / 2.0;
}

//-----------------------------------------------------------------------------
FilterFlanger::
FilterFlanger(
    const float64 & sample_rate,
    const float64 & frequency,
    const float64 & max_delay_time_seconds)
    :
    Filter(sample_rate),
    frequency_(frequency),
    max_delay_(max_delay_time_seconds),
    delay_(sample_rate, max_delay_time_seconds),
    gen_(sample_rate, sweep_waveform(sample_rate))
{
    M_ASSERT_VALUE(frequency_, >, 0.0);
    M_ASSERT_VALUE(max_delay_, >, 0.0);
}

AudioStream
FilterFlanger::
filter(const AudioStream & x)
{
    return Filter::filter(x);
}

AudioStream
FilterFlanger::
filter(const AudioStream & x, const float64 & frequency)
{
    return Filter::filter(x, frequency);
}

AudioStream
FilterFlanger::
filter(
    const AudioStream & x,
    const float64 & frequency,
    const float64 & delay)
{
    uint32 n_channels = x.getNChannels();

    if(is_realtime_ && n_channels > 1)
    {
        M_THROW("In real-time mode, a filter per audio channel must be used!");
    }

    AudioStream y(x.getSampleRate(), n_channels);

    for(uint32 channel = 0; channel < n_channels; ++channel)
    {
        y[channel] = filter(x[channel], frequency, delay);
    }

    return y;
}

AudioStream
FilterFlanger::
filter(
    const AudioStream & x,
    const Buffer & frequency,
    const Buffer & delay)
{
    uint32 n_channels = x.getNChannels();

    if(is_realtime_ && n_channels > 1)
    {
        M_THROW("In real-time mode, a filter per audio channel must be used!");
    }

    AudioStream y(x.getSampleRate(), n_channels);

    for(uint32 channel = 0; channel < n_channels; ++channel)
    {
        y[channel] = filter(x[channel], frequency, delay);
    }

    return y;
}

Buffer
FilterFlanger::
filter(const Buffer & x)
{
    if(!is_realtime_) reset();

    Buffer y(x.getLength());

    uint32 x_samples = x.getLength();

    for(uint32 i = 0; i < x_samples; ++i)
    {
        y << filter(x[i], frequency_, max_delay_);
    }

    return y;
}

Buffer
FilterFlanger::
filter(const Buffer & x, const float64 & frequency)
{
    if(!is_realtime_) reset();

    Buffer y(x.getLength());

    uint32 x_samples = x.getLength();

    for(uint32 i = 0; i < x_samples; ++i)
    {
        y << filter(x[i], frequency, max_delay_);
    }

    return y;
}

Buffer
FilterFlanger::
filter(const Buffer & x, const float64 & frequency, const float64 & delay)
{
    if(!is_realtime_) reset();

    Buffer y(x.getLength());

    uint32 x_samples = x.getLength();

    for(uint32 i = 0; i < x_samples; ++i)
    {
        y << filter(x[i], frequency, delay);
    }

    return y;
}

Buffer
FilterFlanger::
filter(const Buffer & x, const Buffer & frequency, const Buffer & delay)
{
    if(!is_realtime_) reset();

    Buffer y(x.getLength());

    uint32 x_samples = x.getLength();

    Buffer::const_circular_iterator f = frequency.cbegin();
    Buffer::const_circular_iterator d = delay.cbegin();

    for(uint32 i = 0; i < x_samples; ++i)
    {
        y << filter(x[i], *f, *d);
        ++f;
        ++d;
    }

    return y;
}

float64
FilterFlanger::
filter(const float64 & x)
{
    return filter(x, frequency_, max_delay_);

}

float64
FilterFlanger::
filter(const float64 & x, const float64 & frequency)
{
    return filter(x, frequency, max_delay_);

}

float64
FilterFlanger::
filter(const float64 & x, const float64 & frequency, const float64 & delay)
{
    float64 f = gen_.generate(frequency);

    float64 y = delay_.filter(x, f*delay);

    return (y+x) / 2.0;
}

void
FilterFlanger::
plot(boolean show_fc, boolean show_phase)
{
    char title[128];
    sprintf(title,
        "Flanger Frequency Response\n"
        "sr = %0.1f Hz, f = %0.1f Hz, delay = %0.3f ms",
        sample_rate_,
        frequency_,
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
FilterFlanger::
reset()
{
    delay_.reset();
    gen_.reset();
}
