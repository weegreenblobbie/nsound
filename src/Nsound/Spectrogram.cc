//-----------------------------------------------------------------------------
//
//  $Id: Spectrogram.cc 878 2014-11-23 04:51:23Z weegreenblobbie $
//
//  Copyright (c) 2008 Nick Hilton
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

#include <Nsound/AudioStream.h>
#include <Nsound/Buffer.h>
#include <Nsound/FFTransform.h>
#include <Nsound/Generator.h>
#include <Nsound/Spectrogram.h>
#include <Nsound/Plotter.h>

#include <algorithm>
#include <iostream>

using namespace Nsound;

using std::cerr;
using std::endl;

Spectrogram::
Spectrogram(
    const Buffer & x,
    const float64 & sample_rate,
    const float64 & time_window,
    const float64 & time_step,
    const WindowType & type)
    :
    sample_rate_(sample_rate),
    fft_(sample_rate)
{
    // Checked before anything is allocated.
    M_ASSERT_VALUE(sample_rate, >, 0.0);
    M_ASSERT_VALUE(time_window, >, 0.0);
    M_ASSERT_VALUE(time_step, >, 0.0);

    float64 sr = sample_rate_;


    n_window_samples_  = static_cast<int32>(time_window * sr + 0.5);
    uint32 window_step = static_cast<int32>(time_step * sr + 0.5);

    // Calculate the fft size.
    nfft_ = FFTransform::roundUp2(n_window_samples_);

    Generator gen(1);

    fft_window_ = gen.drawWindow(n_window_samples_, type);

    int32 n_samples = x.getLength();
    int32 h_window_samples = n_window_samples_ / 2;

    int32 i = - h_window_samples;

    // Count how many windows we'll need.
    uint32 k = 0;

    while(i < n_samples)
    {
        ++k;
        i += window_step;
    }

    // Allocate matricies.

    time_axis_ = Buffer(k);

    // Using AudioStreams for a matrix-like container where channels are rows
    // and Buffer length is columns.  Here I'm setting samplerate to 1 (doesn't
    // really matter) and pre allocating the buffers to length 1, since they
    // get assigned to later.
    real_ = AudioStream(1, k, 1);
    imag_ = AudioStream(1, k, 1);

    boolean once = true;
    k = 0;
    i = - h_window_samples;
    while(i < n_samples)
    {
        // The window is centered on frame k's time, its step rounded to whole
        // samples.
        time_axis_ << k * window_step / sr;

        // The window's samples of x, zeros where it is past either end, all
        // windowed alike.
        Buffer sub(n_window_samples_);

        if(i < 0)
        {
            sub << Buffer::zeros(-i);
        }

        sub << x.subbuffer(
            static_cast<uint32>(std::max(i, 0)),
            n_window_samples_ - sub.getLength());

        if(sub.getLength() < n_window_samples_)
        {
            sub << Buffer::zeros(n_window_samples_ - sub.getLength());
        }

        sub *= fft_window_;

        // Forward FFT
        FFTChunkVector vec = fft_.fft(sub, nfft_, 0);

        real_[k] = vec[0].getReal();
        imag_[k] = vec[0].getImaginary();
        ++k;

        if(once)
        {
            once = false;

            // A frequency for each bin the frames keep, from 0 Hz.
            frequency_axis_ = vec[0].getFrequencyAxis().subbuffer(
                0, real_[0].getLength());
        }

        i += window_step;
    }
}

Buffer
Spectrogram::
getFrequencyAxis() const
{
    return Buffer(frequency_axis_);
}

AudioStream
Spectrogram::
getMagnitude() const
{
    return ((real_^2.0) + (imag_^2.0))^0.5;
}

Buffer
Spectrogram::
getTimeAxis() const
{
    return Buffer(time_axis_);
}

void
Spectrogram::
plot(
    const std::string & title,
    const boolean & use_dB,
    const float64 & squash) const
{
    AudioStream mag = getMagnitude();

    // Transpose so x is the time axis.
    mag.transpose();

    if(use_dB)
    {
        mag += 1.0;
        mag.dB();
    }
    else if(squash > 0.0)
    {
        mag ^= squash;
    }
    else
    {
        cerr << "Nsound::Spectrogram::plot(): "
             << "use_dB is false and squash <= 0.0 ("
             << squash
             << " <= 0.0)"
             << endl;

        return;
    }

    Plotter pylab;

    pylab.figure();
    pylab.imagesc(time_axis_, frequency_axis_, mag);
    pylab.xlabel("Time (sec)");
    pylab.ylabel("Frequency (Hz)");
    pylab.title(title);
}

Buffer
Spectrogram::
computeMagnitude(const Buffer & x)
{
    Buffer signal(n_window_samples_);

    uint32 n_samples = x.getLength();

    // grab last n samples
    if(n_samples >= n_window_samples_)
    {
        uint32 index = n_samples - n_window_samples_;
        signal = x.subbuffer(index, n_window_samples_);
    }

    // pad with zeros
    else
    {
        uint32 n_zeros = n_window_samples_ - n_samples;

        signal << x << Buffer::zeros(n_zeros);
    }

    // Apply window
    signal *= fft_window_;

    // Forward FFT
    FFTChunkVector vec = fft_.fft(signal, nfft_, 0);

    return vec[0].getMagnitude();
}