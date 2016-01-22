//-----------------------------------------------------------------------------
//
//  $Id: FFTransform.h 874 2014-09-08 02:21:29Z weegreenblobbie $
//
//  Nsound is a C++ library and Python module for audio synthesis featuring
//  dynamic digital filters. Nsound lets you easily shape waveforms and write
//  to disk or plot them. Nsound aims to be as powerful as Csound but easy to
//  use.
//
//  Copyright (c) 2007-Present Nick Hilton
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
#ifndef _NSOUND_FFTRANSFORM_H_
#define _NSOUND_FFTRANSFORM_H_

#include <Nsound/Nsound.h>
#include <Nsound/FFTChunk.h>
#include <Nsound/WindowType.h>

namespace Nsound
{

class Buffer;
class FFTChunk;

//-----------------------------------------------------------------------------
//
//! A Class that performes the Fast Fouier Transfrom on a Buffer.
//
//! Implementing the fft algorithm on page 235 of the book:
//! "Digital Signal Processing: A Practical Guide for Engineers and
//! Scientists"
//!
//! ISBN-13: 978-0-7506-7444-7
//!
//! ISBN-10: 0-7506-7444-X
class FFTransform
{
    public:

    //! Creates an FFTTransform instance.  The sample rate here is only used to
    //! tell the FFTChunk objects how to plot the spectrum, otherwise it does
    //! play a role.
    //!
    //! \par Example:
    //! \code
    //! // C++
    //! FFTransform t(44100.0);
    //!
    //! // Python
    //! t = FFTransform(44100.0)
    //! \endcode
    FFTransform(const float64 & sample_rate);


    //! Destructor
    ~FFTransform(){};

    //! Transforms the time_domain signal and calculates the FFT.
    //
    //! The size of the FFT is determined to be a power of 2 greater than or
    //! equal to the length of time_domain.  If time_domain is less than a
    //! power of 2, the Buffer is padded with zeros until it is exactly a
    //! power of 2.
    //!
    //! \par Example:
    //! \code
    //! // C++
    //! FFTransform t(44100.0);
    //! Buffer b("california.wav");
    //! Buffer fdomain;
    //! fdomain = t.fft(b);
    //!
    //! // Python
    //! t = FFTransform(44100.0)
    //! b = Buffer("california.wav")
    //! fdomain = t.fft(b)
    //! \endcode
    Buffer
    fft(const Buffer & time_domain) const;

    //! Performs the FFT of size N on the input Buffer of overlaping frames.
    //
    //! The size of the FFT is specifed by n_order.  The input Buffer is broken
    //! up into frames of size n_order, the returned FFTChunkVector is the result
    //! for each frame.  If n_overlap is > 0, the frames will overlap by that
    //! number of samples.
    //!
    //! \par Example 1:
    //!
    //! Let n_order = 16 and n_overlap = 0, this how the input is split into
    //! frames.
    //! \code
    //! input = [xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx] // 42 samples
    //!
    //!         |xxxxxxxxxxxxxxx|xxxxxxxxxxxxxxx|xxxxxxxxxx00000| // frames
    //!             16 samples     16 samples     16 samples + pad
    //! \endcode
    //! The returned FFTChunkVector will have 3 FFTChunk objects that represent
    //! the FFT for each of the frames above, note that the last frame will be
    //! padded out to compile a 16-point FFT.
    //!
    //! \par Example 2:
    //!
    //! Let n_order = 16 and n_overlap = 4, this how the input is split into
    //! frames.
    //! \code
    //! input = [xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx] // 42 samples
    //!
    //!         |xxxxxxxxxxxxxxx|
    //!         16 samples   |xxxxxxxxxxxxxxx|
    //!                      16 samples   |xxxxxxxxxxxxxxx|
    //!                                   16 samples   |xxx000000000000|
    //!                                                16 samples
    //! \endcode
    //! The returned FFTChunkVector will have 4 FFTChunk objects that represent
    //! the FFT for each of the frames above, note that the these frames contain
    //! overlapping samples as specified.  Also note that the last frame in
    //! this case has a large number of padded zeros.
    //!
    //! \par Code Example:
    //! \code
    //! // C++
    //! FFTransform t(44100.0);
    //! Buffer b("california.wav");
    //! FFTChunkVector vec;
    //! vec = t.fft(b, 16, 4);
    //!
    //! // Python
    //! t = FFTransform(44100.0)
    //! b = Buffer("california.wav")
    //! vec = t.fft(b, 16, 4)
    //! \endcode
    FFTChunkVector
    fft(const Buffer & input, int32 n_order, int32 n_overlap = 0) const;

    //! Peforms an inverse FFT on each FFTChunk and concatenates the output.
    //
    //! This transforms the frequency domain signals held in the FFTChunkVector
    //! back to the time domain.  If the FFTChunkVector was created with
    //! non-overlapping frames, the resulting output Buffer will be nearly
    //! identical to the original input (there will be some small round-off
    //! error).
    //!
    //! \par Exmample:
    //! \code
    //! // C++
    //! FFTransform t(44100.0);
    //! Buffer b("california.wav");
    //! FFTChunkVector vec;
    //! vec = t.fft(b, 16);
    //! Buffer b2;
    //! b2 = t.ifft(vec);
    //!
    //! // Python
    //! t = FFTransform(44100.0)
    //! b = Buffer("california.wav")
    //! vec = t.fft(b, 16)
    //! b2 = t.ifft(vec)
    //! \endcode
    Buffer
    ifft(const FFTChunkVector & input) const;

        //! Peforms an inverse FFT on the input Buffer.
    //
    //! This transforms the frequency domain signal held in the input Buffer
    //! back to the time domain.  The input signal will get padded so its
    //! length is exactly a power of 2.
    //!
    //! \par Exmample:
    //! \code
    //! // C++
    //! FFTransform t(44100.0);
    //! Buffer b("california.wav");
    //! fdomain = t.fft(b);
    //! Buffer tdomain;
    //! tdomain = t.ifft(fdomain);
    //!
    //! // Python
    //! t = FFTransform(44100.0)
    //! b = Buffer("california.wav")
    //! fdomain = t.fft(b, 16)
    //! tdomain = t.ifft(fdomain)
    //! \endcode
    Buffer
    ifft(const Buffer & frequency_domain) const;

    //! Returns nearest power of 2 >= raw.
    static
    int32
    roundUp2(int32 raw);

    //! A window is multiplied by the input prior to performing the transform, this help reduce artifacts near the edges.
    void
    setWindow(WindowType type);

    protected:

    //! Samples per second.
    uint32 sample_rate_;

    private:

    //! Peforms an inplace, nth order Fast Fouier Transform on the Buffers.
    void
    fft(Buffer & real, Buffer & img, int32 n_order) const;

    WindowType type_;

}; // class BufferChunk

} // namespace Nsound

#endif

// :mode=c++: jEdit modeline
