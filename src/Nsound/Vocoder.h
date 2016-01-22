//-----------------------------------------------------------------------------
//
//  $Id: Vocoder.h 874 2014-09-08 02:21:29Z weegreenblobbie $
//
//  Copyright (c) 2008 to Present Nick Hilton
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
#ifndef _NSOUND_VOCODER_H_
#define _NSOUND_VOCODER_H_

#include <Nsound/Buffer.h>

#include <vector>

namespace Nsound
{

class AudioStream;
class Buffer;
class Filter;
class FilterMovingAverage;

//-----------------------------------------------------------------------------
class Vocoder
{
    public:

    //! Default Constructor
    //
    //! sample_rate:   the sample rate
    //! window_length: the length of a window to average for the voice envelope
    //! n_bands:       the number of band pass filters to use
    //! freq_max:      the maximum frequency, band pass filters divide up the
    //!                frequency range from 0 to freq_max
    Vocoder(
        const float64 & sample_rate,
        const float64 & window_length,
        const uint32  & n_bands,
        const float64 & freq_max = 4000.0,
        const bool use_mel_scale = true,
        const bool plot_filter_bank = false);

    virtual ~Vocoder();

    Buffer
    filter(const Buffer & voice, const Buffer & carrier);

    float64
    filter(const float64 & voice, const float64 & carrier);

    float64
    getSampleRate() const { return sample_rate_; };

    void
    reset();

    protected:

    float64 sample_rate_;
    uint32  window_size_;
    uint32  n_bands_;
    float64 freq_max_;

    typedef std::vector< Filter * > FilterVector;

    FilterVector filters_input_;
    FilterVector filters_output_;
    FilterVector moving_average_;

};

} // namespace

// :mode=c++: jEdit modeline
#endif
