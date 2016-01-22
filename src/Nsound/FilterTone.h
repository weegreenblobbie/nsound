//-----------------------------------------------------------------------------
//
//  $Id: FilterTone.h 874 2014-09-08 02:21:29Z weegreenblobbie $
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
#ifndef _NSOUND_FILTER_TONE_H_
#define _NSOUND_FILTER_TONE_H_

#include <Nsound/Filter.h>

#include <set>

namespace Nsound
{

class AudioStream;
class Buffer;

//-----------------------------------------------------------------------------
class FilterTone : public Filter
{
    public:

    FilterTone(
        const float64 & sample_rate,
        const float64 & half_power_frequency);

    virtual ~FilterTone();

    AudioStream
    filter(const AudioStream & x);

    AudioStream
    filter(const AudioStream & x, const Buffer & hp_frequencies);

    Buffer
    filter(const Buffer & x);

    Buffer
    filter(const Buffer & x, const Buffer & hp_frequencies);

    float64
    filter(const float64 & x);

    float64
    filter(const float64 & x, const float64 & hp_frequency);

    float64
    getFrequency() const {return hp_frequency_;};

    uint32
    getKernelSize() const { return 2; };

    void
    plot(boolean show_fc = true, boolean show_phase = false);

    //! Resets interal history buffer and sets the cut off frequency to the one
    //! used at declaration.
    void
    reset();

    void
    makeKernel(const float64 & half_power_frequency);

    protected:

    float64 hp_frequency_;
    float64 b_;
    float64 a_;
    float64 last_output_;

    //! A class to store calculated kernels.
    struct Kernel
    {
        Kernel(const uint32 & frequency);

        bool operator<(const Kernel & rhs) const;

        float64 b_;
        float64 a_;

        uint32 frequency_;
    };

    typedef std::set<Kernel> KernelCache;

    KernelCache kernel_cache_;
};

}; // Nsound

// :mode=c++: jEdit modeline

#endif
