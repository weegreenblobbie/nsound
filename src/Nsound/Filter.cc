//-----------------------------------------------------------------------------
//
//  $Id: Filter.cc 916 2015-08-22 16:31:39Z weegreenblobbie $
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

#include <Nsound/AudioStream.h>
#include <Nsound/Buffer.h>
#include <Nsound/FFTransform.h>
#include <Nsound/Filter.h>
#include <Nsound/Plotter.h>

#include <cmath>
#include <iostream>

using namespace Nsound;

Filter::
Filter(const float64 & sample_rate)
    :
    sample_rate_(sample_rate),
    two_pi_over_sample_rate_(2.0 * M_PI / sample_rate),
    sample_time_(1.0 / sample_rate),
    kernel_size_(0),
    is_realtime_(false)
{
}

AudioStream
Filter::
filter(const AudioStream & x)
{
    if(!is_realtime_) reset();

    uint32 n_channels = x.getNChannels();

    if(is_realtime_ && n_channels > 1)
    {
        M_THROW("In real-time mode, a filter per audio channel must be used!");
    }

    AudioStream y(x.getSampleRate(), n_channels);

    for(uint32 channel = 0; channel < n_channels; ++channel)
    {
        y[channel] = filter(x[channel]);
    }

    return y;
}

AudioStream
Filter::
filter(const AudioStream & x, const float64 & frequency)
{
    if(!is_realtime_) reset();

    uint32 n_channels = x.getNChannels();

    if(is_realtime_ && n_channels > 1)
    {
        M_THROW("In real-time mode, a filter per audio channel must be used!");
    }

    AudioStream y(x.getSampleRate(), n_channels);

    for(uint32 channel = 0; channel < n_channels; ++channel)
    {
        y[channel] = filter(x[channel], frequency);
    }

    return y;
}

AudioStream
Filter::
filter(const AudioStream & x, const Buffer & frequency)
{
    if(!is_realtime_) reset();

    uint32 n_channels = x.getNChannels();

    if(is_realtime_ && n_channels > 1)
    {
        M_THROW("In real-time mode, a filter per audio channel must be used!");
    }

    AudioStream y(x.getSampleRate(), n_channels);

    for(uint32 channel = 0; channel < n_channels; ++channel)
    {
        y[channel] = filter(x[channel], frequency);
    }

    return y;
}

Buffer
Filter::
filter(const Buffer & x)
{
    if(!is_realtime_) reset();

    Buffer y(x.getLength());

    uint32 x_samples = x.getLength();

    for(uint32 i = 0; i < x_samples; ++i)
    {
        y << filter(x[i]);
    }

    return y;
}

Buffer
Filter::
filter(const Buffer & x, const float64 & frequency)
{
    if(!is_realtime_) reset();

    Buffer::const_iterator itor = x.begin();
    Buffer::const_iterator end = x.end();

    Buffer y(x.getLength());

    while(itor != end)
    {
        y << filter(*itor, frequency);

        ++itor;
    }

    return y;
}

Buffer
Filter::
filter(const Buffer & x, const Buffer & frequencies)
{
    if(!is_realtime_) reset();

    Buffer::const_circular_iterator freq = frequencies.cbegin();

    Buffer::const_iterator itor = x.begin();
    Buffer::const_iterator end = x.end();

    Buffer y(x.getLength());

    while(itor != end)
    {
        y << filter(*itor, *freq);

        ++itor;
        ++freq;
    }

    return y;
}

Buffer
Filter::
getFrequencyAxis(const uint32 n_fft)
{
    uint32 fft_chunk_size = FFTransform::roundUp2(
        static_cast<int32>(n_fft));

    uint32 n_samples = fft_chunk_size / 2 + 1;

    float64 f_step = (1.0 / (static_cast<float64>(fft_chunk_size) / 2.0))
                   * (sample_rate_ / 2.0);

    Buffer f_axis;

    float64 f = 0.0;

    for(uint32 i = 0; i < n_samples; ++i)
    {
        f_axis << f;
        f += f_step;
    }

    return f_axis;
};

Buffer
Filter::
getFrequencyResponse(const uint32 n_fft)
{
    FFTransform fft(sample_rate_);

//~    fft.setWindow(HANNING);

    FFTChunkVector vec;

    vec = fft.fft(getImpulseResponse(), n_fft);

    return vec[0].getMagnitude();
}

Buffer
Filter::
getImpulseResponse(const uint32 n_samples)
{
    if(!is_realtime_) reset();

    Buffer response(n_samples);

    response << filter(1.0);

    for(uint32 i = 1; i < n_samples; ++i)
    {
        response << filter(0.0);
    }

    if(!is_realtime_) reset();

    return response;
}

Buffer
Filter::
getPhaseResponse()
{
    uint32 n_samples = static_cast<uint32>(sample_rate_ * 2);

    FFTransform fft(n_samples);

    FFTChunkVector vec;

    vec = fft.fft(getImpulseResponse(), n_samples);

    Buffer phase = vec[0].getPhase();

    return phase.subbuffer(0, phase.getLength() / 2 + 1);
}

void
Filter::
plot(boolean show_phase)
{
    Buffer x = getFrequencyAxis();
    Buffer fr = getFrequencyResponse().getdB();

    Plotter pylab;

    pylab.figure();

    uint32 n_rows = 1;
    uint32 n_cols = 1;

    if(show_phase)
    {
        n_rows = 2;
    }

    pylab.subplot(n_rows, n_cols, 1);

    // Frequency response
    pylab.plot(x,fr, "blue");

    pylab.xlabel("Frequency (Hz)");
    pylab.ylabel("Frequency Response (dB)");

    // Phase response
    if(show_phase)
    {
        pylab.subplot(n_rows, n_cols, 2);

        Buffer pr = getPhaseResponse().getdB();

        pylab.plot(x,pr);

        pylab.xlabel("Frequency (Hz)");
        pylab.ylabel("Phase Response (dB)");
    }

    float64 ymax = fr.getMax();
    float64 height = ymax - -60.0;

    pylab.ylim(-60.0, ymax + 0.05 * height);
}
