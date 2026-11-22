//-----------------------------------------------------------------------------
//
//  $Id: FilterFlanger.h 874 2014-09-08 02:21:29Z weegreenblobbie $
//
//  Nsound is a C++ library and Python module for audio synthesis featuring
//  dynamic digital filters. Nsound lets you easily shape waveforms and write
//  to disk or plot them. Nsound aims to be as powerful as Csound but easy to
//  use.
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
#pragma once

#include <Nsound/Filter.h>
#include <Nsound/FilterDelay.h>
#include <Nsound/Generator.h>

namespace Nsound
{

// Forward class declarations
class AudioStream;
class Buffer;

//-----------------------------------------------------------------------------
//! A class for filtering audio in the frequecy domain.
class FilterFlanger : public Filter
{
    public:

    FilterFlanger(
        const float64 & sample_rate,
        const float64 & frequency,
        const float64 & max_delay_time_seconds);

    AudioStream
    filter(const AudioStream & x);

    AudioStream
    filter(
        const AudioStream & x,
        const float64 & frequency);

    AudioStream
    filter(
        const AudioStream & x,
        const float64 & frequency,
        const float64 & delay);

    AudioStream
    filter(
        const AudioStream & x,
        const Buffer & frequency,
        const Buffer & delay);

    Buffer
    filter(const Buffer & x);

    Buffer
    filter(
        const Buffer & x,
        const float64 & frequency);

    Buffer
    filter(
        const Buffer & x,
        const float64 & frequency,
        const float64 & delay);

    Buffer
    filter(
        const Buffer & x,
        const Buffer & frequency,
        const Buffer & delay);

    float64
    filter(const float64 & x) override;

    float64
    filter(
        const float64 & x,
        const float64 & frequency) override;

    float64
    filter(
        const float64 & x,
        const float64 & frequency,
        const float64 & delay);

    void
    plot(boolean show_fc=false, boolean show_phase=false);

    void
    reset() override;

    protected:

    float64       frequency_;
    float64       max_delay_;
    FilterDelay   delay_;
    Generator     gen_;

};

};


// :mode=c++:  jEdit modeline
