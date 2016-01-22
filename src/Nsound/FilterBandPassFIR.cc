//-----------------------------------------------------------------------------
//
//  $Id: FilterBandPassFIR.cc 874 2014-09-08 02:21:29Z weegreenblobbie $
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

#include <Nsound/AudioStream.h>
#include <Nsound/Buffer.h>
#include <Nsound/FFTransform.h>
#include <Nsound/FilterBandPassFIR.h>
#include <Nsound/FilterLowPassFIR.h>
#include <Nsound/FilterHighPassFIR.h>
#include <Nsound/Plotter.h>

#include <cmath>
#include <cstdio>
#include <iostream>

using namespace Nsound;

using std::cerr;
using std::endl;

#define CERR_HEADER __FILE__ << ":" << __LINE__ << ": "

//-----------------------------------------------------------------------------
FilterBandPassFIR::
FilterBandPassFIR(
    const float64 & sample_rate,
    uint32 kernel_size,
    const float64 & frequency_Hz_low,
    const float64 & frequency_Hz_high)
    :
    Filter(sample_rate),
    low_(new FilterLowPassFIR(sample_rate, kernel_size, frequency_Hz_high)),
    high_(new FilterHighPassFIR(sample_rate, kernel_size, frequency_Hz_low))
{
    kernel_size_ = kernel_size * 2;
}

//-----------------------------------------------------------------------------
FilterBandPassFIR::
~FilterBandPassFIR()
{
    delete low_;
    delete high_;
}

float64
FilterBandPassFIR::
getFrequencyLow() const
{
    return high_->getFrequency();
}

float64
FilterBandPassFIR::
getFrequencyHigh() const
{
    return low_->getFrequency();
}

AudioStream
FilterBandPassFIR::
filter(const AudioStream & x)
{
    return Filter::filter(x);
}

AudioStream
FilterBandPassFIR::
filter(const AudioStream & x, const float64 & f_low, const float64 & f_high)
{
    return low_->filter(high_->filter(x, f_low), f_high);
}

AudioStream
FilterBandPassFIR::
filter(
    const AudioStream & x,
    const Buffer & f_low,
    const Buffer & f_high)
{
    return low_->filter(high_->filter(x,f_low), f_high);
}

Buffer
FilterBandPassFIR::
filter(const Buffer & x)
{
    return Filter::filter(x);
}

Buffer
FilterBandPassFIR::
filter(const Buffer & x, const float64 & f_low, const float64 & f_high)
{
    return low_->filter(high_->filter(x,f_low), f_high);
}

Buffer
FilterBandPassFIR::
filter(
    const Buffer & x,
    const Buffer & f_low,
    const Buffer & f_high)
{
    return low_->filter(high_->filter(x, f_low), f_high);
}

float64
FilterBandPassFIR::
filter(const float64 & x)
{
    return low_->filter(high_->filter(x));
};

float64
FilterBandPassFIR::
filter(const float64 & x, const float64 & frequency)
{
    return low_->filter(high_->filter(x));
}

float64
FilterBandPassFIR::
filter(
    const float64 & x,
    const float64 & f_low,
    const float64 & f_high)
{
    return low_->filter(high_->filter(x, f_low), f_high);
}

void
FilterBandPassFIR::
plot(boolean show_fc,boolean show_phase)
{
    char title[128];
    sprintf(title,
        "Band Pass FIR Frequency Response\n"
        "order = %d, fl = %0.1f Hz, fl = %0.1f Hz, sr = %0.1f Hz",
        low_->getKernelSize() - 1,
        high_->getFrequency(),
        low_->getFrequency(),
        sample_rate_);

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

        pylab.axvline(low_->getFrequency(),  "color='red'");
        pylab.axvline(high_->getFrequency(), "color='red'");

        pylab.title(title);
    }
}

void
FilterBandPassFIR::
reset()
{
    low_->reset();
    high_->reset();
}

