//-----------------------------------------------------------------------------
//
//  $Id: Spectrogram.h 878 2014-11-23 04:51:23Z weegreenblobbie $
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
#ifndef _NSOUND_SPECTROGRAM_H_
#define _NSOUND_SPECTROGRAM_H_

#include <Nsound/WindowType.h>

#include <string>

namespace Nsound
{

class AudioStream;
class Buffer;

//! The result from an STFT
class Spectrogram
{
    public:

    Spectrogram(
        const Buffer &     x,
        const float64 &    sample_rate,
        const float64 &    time_window,
        const float64 &    time_step,
        const WindowType & type);

    Spectrogram(const Spectrogram & copy);

    ~Spectrogram();

    Buffer
    getFrequencyAxis() const;

    AudioStream
    getMagnitude() const;

    Buffer
    getTimeAxis() const;

    Spectrogram &
    operator=(const Spectrogram & rhs);

    void
    plot(
        const std::string & title = "",
        const boolean & use_dB = true,
        const float64 & squash = 0.5) const;

    // For use in a real-time loop, computes one-time slice from the last
    // n samples and returns the frequency magnitude.
    Buffer computeMagnitude(const Buffer & x);

    protected:

    float64 sample_rate_;

    Buffer * frequency_axis_;
    Buffer * time_axis_;

    AudioStream * real_;  // Using an AudioStream as a 2D matrix
    AudioStream * imag_;  // Using an AudioStream as a 2D matrix

    Buffer *      fft_window_;
    uint32        nfft_;
    uint32        n_window_samples_;
    FFTransform * fft_;

}; // class Spectrogram

} // namespace Nsound

// :mode=c++: jEdit modeline
#endif
