//-----------------------------------------------------------------------------
//
//  $Id: FFTChunk.h 874 2014-09-08 02:21:29Z weegreenblobbie $
//
//  Nsound is a C++ library and Python module for audio synthesis featuring
//  dynamic digital filters. Nsound lets you easily shape waveforms and write
//  to disk or plot them. Nsound aims to be as powerful as Csound but easy to
//  use.
//
//  Copyright (c) 2008-Present Nick Hilton
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
#ifndef _NSOUND_FFT_CHUNK_H_
#define _NSOUND_FFT_CHUNK_H_

#include <Nsound/Nsound.h>

#include <cmath>
#include <iostream>
#include <vector>

namespace Nsound
{

// forward declare
class Buffer;

//! Results of performing an FFT are stored in this class.
class FFTChunk
{
    public:

    FFTChunk(uint32 size=32, uint32 sample_rate=44100, uint32 original_size = 0);

    FFTChunk(const FFTChunk & copy);

    ~FFTChunk();

    Buffer
    getFrequencyAxis() const;

    Buffer
    getReal() const;

    Buffer
    getImaginary() const;

    Buffer
    getMagnitude() const;

    uint32
    getOriginalSize() const { return original_size_; };

    Buffer
    getPhase() const;

    boolean
    isPolar() const { return is_polar_; };

    FFTChunk &
    operator=(const FFTChunk & rhs);

    void
    plot(
        const std::string & title = "",
        boolean dB = true,
        boolean show_phase = false) const;

    //! Sets up an FFTChunk to use the provided real & imaginary.
    void
    setCartesian(const Buffer & real, const Buffer & imaginary);

    //! Sets up an FFTChunk to use the provided magnitude & phase.
    void
    setPolar(const Buffer & magnitude, const Buffer & phase);

    //! convertes the real & imaginary unit to plor form: magnitude & phase
    void
    toPolar();

    //! convertes the magnitude & phase to cartesian form: real & imaginary
    void
    toCartesian();

    Buffer * real_;
    Buffer * imag_;

    uint32 getSampleRate() const {return sample_rate_;};

    protected:

    uint32 sample_rate_;
    uint32 original_size_;
    boolean is_polar_;


}; // class FFTChunk

typedef std::vector< FFTChunk > FFTChunkVector;

} // namespace Nsound

// :mode=c++: jEdit modeline
#endif
