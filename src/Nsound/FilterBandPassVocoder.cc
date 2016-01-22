//-----------------------------------------------------------------------------
//
//  $Id: FilterBandPassVocoder.cc 878 2014-11-23 04:51:23Z weegreenblobbie $
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
#include <Nsound/FilterBandPassVocoder.h>
#include <Nsound/Plotter.h>

#include <cmath>
#include <cstdio>
#include <iostream>

using namespace Nsound;

using std::cerr;
using std::endl;

#define CERR_HEADER __FILE__ << ":" << __LINE__ << ": "

//-----------------------------------------------------------------------------
FilterBandPassVocoder::
FilterBandPassVocoder(
    const float64 & sample_rate,
    const float64 & frequency_low,
    const float64 & frequency_high)
    :
    Filter(sample_rate),
    frequency_low_(frequency_low),
    frequency_high_(frequency_high),
    omega2_(0.0),
    f_(0.0),
    att_(0.0),
    low1_(0.0),
    low2_(0.0),
    mid1_(0.0),
    mid2_(0.0),
    high1_(0.0),
    high2_(0.0)
{
    reset();
}

//-----------------------------------------------------------------------------
FilterBandPassVocoder::
~FilterBandPassVocoder()
{
}

AudioStream
FilterBandPassVocoder::
filter(const AudioStream & x)
{
    return Filter::filter(x);
}

AudioStream
FilterBandPassVocoder::
filter(const AudioStream & x, const Buffer & frequencies)
{
    return Filter::filter(x);
}

AudioStream
FilterBandPassVocoder::
filter(
    const AudioStream & x,
    const Buffer & frequencies_Hz_low,
    const Buffer & frequencies_Hz_high)
{
    uint32 n_channels = x.getNChannels();

    AudioStream y(x.getSampleRate(), n_channels);

    for(uint32 channel = 0; channel < n_channels; ++channel)
    {
        y[channel] = FilterBandPassVocoder::filter(
            x[channel],
            frequencies_Hz_low,
            frequencies_Hz_high);
    }

    return y;
}

Buffer
FilterBandPassVocoder::
filter(const Buffer & x)
{
    return Filter::filter(x);
}

Buffer
FilterBandPassVocoder::
filter(const Buffer & x, const Buffer & frequencies)
{
    return Filter::filter(x);
}

Buffer
FilterBandPassVocoder::
filter(
    const Buffer & x,
    const Buffer & frequencies_Hz_low,
    const Buffer & frequencies_Hz_high)
{
    Buffer::const_circular_iterator low  = frequencies_Hz_low.cbegin();
    Buffer::const_circular_iterator high = frequencies_Hz_high.cbegin();

    Buffer y;

    for(uint32 i = 0; i < x.getLength(); ++i, ++low, ++high)
    {
        y << FilterBandPassVocoder::filter(x[i], *low, *high);
    }

    return y;
}

float64
FilterBandPassVocoder::
filter(const float64 & x)
{
    high1_ = x - f_ * mid1_ - low1_;
    mid1_ += high1_ * omega2_;
    low1_ += mid1_;

    high2_ = low1_ - f_ * mid2_ - low2_;
    mid2_ += high2_ * omega2_;
    low2_ += mid2_;

    return high2_ * att_;
};

float64
FilterBandPassVocoder::
filter(const float64 & x, const float64 & frequency)
{
    return FilterBandPassVocoder::filter(x);
}

float64
FilterBandPassVocoder::
filter(
    const float64 & x,
    const float64 & frequencies_Hz_low,
    const float64 & frequencies_Hz_high)
{
    makeKernel(frequencies_Hz_low, frequencies_Hz_high);
    return FilterBandPassVocoder::filter(x);
}

void
FilterBandPassVocoder::
plot(boolean show_fc, boolean show_phase)
{
    char title[256];
    sprintf(title,
        "Band Pass IIR Frequency Response\n"
        "fl = %0.1f Hz, fh = %01f Hz, omega2 = %0.1f Hz, f_ = %0.1f Hz, att_ = %0.1f",
        frequency_low_,
        frequency_high_,
        omega2_,
        f_,
        att_);

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

        pylab.axvline(frequency_low_,"color='red'");
        pylab.axvline(frequency_high_,"color='red'");

        pylab.title(title);
    }
}

void
FilterBandPassVocoder::
reset()
{
    low1_ =
    low2_ =
    mid1_ =
    mid2_ =
    high1_ =
    high2_ = 0.0;

    makeKernel(frequency_low_, frequency_high_);
}

void
FilterBandPassVocoder::
makeKernel(const float64 & f_low, const float64 & f_high)
{
    float64 fc = f_high - 0.5 * (f_high - f_low);

    float64 omega = fc * 2.0 * M_PI / sample_rate_;

    omega2_ = omega * omega;

    f_ = 0.4 / omega;

    att_ = 1.0 / (6.0 + ((std::exp(fc / sample_rate_) - 1.0) * 10.0));
}
