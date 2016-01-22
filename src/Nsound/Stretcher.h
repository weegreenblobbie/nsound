//-----------------------------------------------------------------------------
//
//  $Id: Stretcher.h 874 2014-09-08 02:21:29Z weegreenblobbie $
//
//  Nsound is a C++ library and Python module for audio synthesis featuring
//  dynamic digital filters. Nsound lets you easily shape waveforms and write
//  to disk or plot them. Nsound aims to be as powerful as Csound but easy to
//  use.
//
//  Copyright (c) 2009 to Present Nick Hilton
//
//  weegreenblobbie_yahoo_com (replace '_' with '@' and '.')
//
//  Based on WSOLA.
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
#ifndef _NSOUND_STRETCHER_H_
#define _NSOUND_STRETCHER_H_

#include <Nsound/Nsound.h>

namespace Nsound
{

class AudioStream;
class Buffer;
class FFTChunk;

//-----------------------------------------------------------------------------
//! WSOLA
class Stretcher
{
    public:

    //! Default Constructor
    //
    //! sample_rate:      the sample rate
    //! window_size:      default window size in seconds
    //! max_delta_window: percent of window size to search for best fit
    Stretcher(
        const float64 & sample_rate,
        const float64 & window_size_seconds = 0.08,
        const float64 & max_delta_window = 0.25);

    //! Copy constructor.
    Stretcher(const Stretcher & copy);

    //! Destructor.
    virtual ~Stretcher();

    //! Assignment.
    Stretcher &
    operator=(const Stretcher & rhs);

    AudioStream
    pitchShift(const AudioStream & x, const float64 & factor);

    AudioStream
    pitchShift(const AudioStream & x, const Buffer & factor);

    Buffer
    pitchShift(const Buffer & x, const float64 & factor);

    Buffer
    pitchShift(const Buffer & x, const Buffer & factor);

    void
    showProgress(boolean flag){show_progress_ = flag;};

    AudioStream
    timeShift(const AudioStream & x, const float64 & factor);

    AudioStream
    timeShift(const AudioStream & x, const Buffer & factor);

    Buffer
    timeShift(const Buffer & x, const float64 & factor);

    Buffer
    timeShift(const Buffer & x, const Buffer & factor);

    protected:

    void
    analyize(const Buffer & input, const float64 & factor);

    void
    analyize(const Buffer & input, const Buffer & factor);

    uint32
    searchForBestMatch(
        const Buffer & input,
        uint32 source_index,
        uint32 search_index) const;

    Buffer
    overlapAdd(const Buffer & input) const;

    Buffer * frames_;
    float64  sample_rate_;
    Buffer * window_;
    uint32   window_length_;
    uint32   max_delta_;
    boolean  show_progress_;

};

} // namespace

#endif
// :mode=c++: jEdit modeline

