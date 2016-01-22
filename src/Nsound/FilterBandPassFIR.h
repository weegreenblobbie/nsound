//-----------------------------------------------------------------------------
//
//  $Id: FilterBandPassFIR.h 874 2014-09-08 02:21:29Z weegreenblobbie $
//
//  Nsound is a C++ library and Python module for audio synthesis featuring
//  dynamic digital filters. Nsound lets you easily shape waveforms and write
//  to disk or plot them. Nsound aims to be as powerful as Csound but easy to
//  use.
//
//  Copyright (c) 2006 - Present Nick Hilton
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
#ifndef _NSOUND_FILTER_BAND_PASS_FIR_H_
#define _NSOUND_FILTER_BAND_PASS_FIR_H_

#include <Nsound/Filter.h>

#include <set>

namespace Nsound
{

class FilterLowPassFIR;
class FilterHighPassFIR;

//-----------------------------------------------------------------------------
class FilterBandPassFIR : public Filter
{
    public:

    FilterBandPassFIR(
        const float64 & sample_rate,
        uint32 kernel_size,
        const float64 & frequency_Hz_low,
        const float64 & frequency_Hz_high);

    virtual ~FilterBandPassFIR();

    float64
    getFrequencyLow() const;

    float64
    getFrequencyHigh() const;

    AudioStream
    filter(const AudioStream & x);

//~//~    AudioStream
//~    filter(const AudioStream & x, const Buffer & frequencies);

    AudioStream
    filter(
        const AudioStream & x,
        const float64 & low_frequencies,
        const float64 & high_frequencies);

    AudioStream
    filter(
        const AudioStream & x,
        const Buffer & low_frequencies,
        const Buffer & high_frequencies);

    Buffer
    filter(const Buffer & x);

//~//~    Buffer
//~    filter(const Buffer & x, const Buffer & frequencies);

    Buffer
    filter(
        const Buffer & x,
        const float64 & frequencies_Hz_low,
        const float64 & frequencies_Hz_high);

    Buffer
    filter(
        const Buffer & x,
        const Buffer & frequencies_Hz_low,
        const Buffer & frequencies_Hz_high);

    virtual
    float64
    filter(const float64 & x);

    virtual
    float64
    filter(const float64 & x, const float64 & frequency_Hz);

    virtual
    float64
    filter(
        const float64 & x,
        const float64 & frequency_Hz_low,
        const float64 & frequency_Hz_high);


    void
    plot(boolean show_fc = true, boolean show_phase = false);

    //! Resets interal history buffer and sets the cut off frequency to the one
    //! used at declaration.
    void
    reset();

    protected:

    // Band Pass must cascade two stages in series.
    FilterLowPassFIR  * low_;
    FilterHighPassFIR * high_;

    private:

    FilterBandPassFIR(const FilterBandPassFIR & copy)
        :
        Filter(copy.sample_rate_),
        low_(NULL),
        high_(NULL)
    {
    };

    FilterBandPassFIR &
    operator=(const FilterBandPassFIR & rhs)
    {
        return *this;
    };

}; // class FilterBandPassFIR

} // namespace Nsound

// :mode=c++: jEdit modeline
#endif
