//-----------------------------------------------------------------------------
//
//  $Id: Filter.h 911 2015-07-10 03:04:24Z weegreenblobbie $
//
//  Nsound is a C++ library and Python module for audio synthesis featuring
//  dynamic digital filters. Nsound lets you easily shape waveforms and write
//  to disk or plot them. Nsound aims to be as powerful as Csound but easy to
//  use.
//
//  Copyright (c) 2007 Nick Hilton
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

#ifndef _NSOUND_FILTER_H_
#define _NSOUND_FILTER_H_

#include <Nsound/Nsound.h>

#include <string>

namespace Nsound
{

class AudioStream;
class Buffer;

//-----------------------------------------------------------------------------
//! Base class for IIR Filters, defines the interface.
class Filter
{
    public:

    Filter(const float64 & sample_rate);

    virtual ~Filter() {};

    void setRealtime(bool flag) {is_realtime_ = flag;}

    AudioStream
    filter(const AudioStream & x);

    AudioStream
    filter(const AudioStream & x, const float64 & frequency);

    AudioStream
    filter(const AudioStream & x, const Buffer & frequencies);

    Buffer
    filter(const Buffer & x);

    Buffer
    filter(const Buffer & x, const float64 & frequency);

    Buffer
    filter(const Buffer & x, const Buffer & frequencies);

    virtual
    float64
    filter(const float64 & x) = 0;

    virtual
    float64
    filter(const float64 & x, const float64 & frequency) = 0;

    Buffer
    getFrequencyAxis(const uint32 n_fft = 8192);

    Buffer
    getFrequencyResponse(const uint32 n_fft = 8192);

    Buffer
    getImpulseResponse(const uint32 n_samples = 8192);

    virtual
    uint32
    getKernelSize() const {return kernel_size_;};

    Buffer
    getPhaseResponse();

    float64
    getSampleRate() const { return sample_rate_; };

    void
    plot(boolean show_phase = false);

    virtual
    void
    reset() = 0;

    protected:

    float64 sample_rate_;
    float64 two_pi_over_sample_rate_;
    float64 sample_time_; // 1.0 / sample_rate_
    uint32 kernel_size_;

    bool is_realtime_;

}; // class Filter

} // namespace

// :mode=c++: jEdit modeline
#endif
