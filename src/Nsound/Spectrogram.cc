//-----------------------------------------------------------------------------
//
//  $Id: Spectrogram.cc 878 2014-11-23 04:51:23Z weegreenblobbie $
//
//  Copyright (c) 2008 Nick Hilton
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

#include <Nsound/AudioStream.h>
#include <Nsound/Buffer.h>
#include <Nsound/FFTransform.h>
#include <Nsound/Generator.h>
#include <Nsound/Spectrogram.h>
#include <Nsound/Plotter.h>

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
    frequency_axis_(NULL),
    time_axis_(NULL),
    real_(NULL),
    imag_(NULL),
    fft_window_(new Buffer()),
    nfft_(0),
    n_window_samples_(0),
    fft_(new FFTransform(sample_rate))
{
    float64 sr = sample_rate_;

    if(sample_rate <= 0.0)
    {
        cerr << "Nsound::Spectrogram(): "
             << "sample_rate <= 0.0 ("
             << sample_rate
             << "<= 0.0)"
             << endl;

        // Use some default.
        sr = 44100.0;
    }

    float64 time_w = time_window;

    if(time_window <= 0.0)
    {
        cerr << "Nsound::Spectrogram(): "
             << "time_window <= 0.0 ("
             << time_window
             << "<= 0.0)"
             << endl;

        // Use some default.
        time_w = 0.020;
    }

    float64 time_s = time_step;

    if(time_step <= 0.0)
    {
        cerr << "Nsound::Spectrogram(): "
             << "time_step <= 0.0 ("
             << time_step
             << "<= 0.0)"
             << endl;

        // Use some default.
        time_s = 0.020;
    }

    n_window_samples_  = static_cast<int32>(time_w * sr + 0.5);
    uint32 window_step = static_cast<int32>(time_s * sr + 0.5);

    // Calculate the fft size.
    nfft_ = FFTransform::roundUp2(n_window_samples_);

    Generator gen(1);

    *fft_window_ = gen.drawWindow(n_window_samples_, type);

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

    frequency_axis_ = new Buffer(1); // gets assigned to later.
    time_axis_ = new Buffer(k);

    // Using AudioStreams for a matrix-like container where channels are rows
    // and Buffer length is columns.  Here I'm setting samplerate to 1 (doesn't
    // really matter) and pre allocating the buffers to length 1, since they
    // get assigned to later.
    real_ = new AudioStream(1, k, 1);
    imag_ = new AudioStream(1, k, 1);

    if(frequency_axis_ == NULL)
    {
        cerr << "Nsound::Spectrogram::Spectrogram(): "
             << "failed to allocate memory for frequency_axis_"
             << endl;

        return;
    }

    if(time_axis_ == NULL)
    {
        cerr << "Nsound::Spectrogram::Spectrogram(): "
             << "failed to allocate memory for time_axis_"
             << endl;

        return;
    }

    if(real_ == NULL)
    {
        cerr << "Nsound::Spectrogram::Spectrogram(): "
             << "failed to allocate memory for real_"
             << endl;

        return;
    }

    if(imag_ == NULL)
    {
        cerr << "Nsound::Spectrogram::Spectrogram(): "
             << "failed to allocate memory for imag_"
             << endl;

        return;
    }

    float64 time = 0.0;
    boolean once = true;
    k = 0;
    i = - h_window_samples;
    while(i < n_samples)
    {
        *time_axis_ << time;

        // Extract a sub signal.
        int32 i0 = i;

        Buffer sub(n_window_samples_);

        // Range check
        if(i0 < 0)
        {
            int32 n_left = i + n_window_samples_;
            int32 n_zeros = n_window_samples_ - n_left;

            sub = gen.silence(n_zeros)
                << (x.subbuffer(0, n_left) * gen.drawWindow(n_left, type));
        }
        else
        {
            sub = x.subbuffer(i0, n_window_samples_) * *fft_window_;
        }

        // Pad with zeros if at the end.
        if(sub.getLength() < n_window_samples_)
        {
            int32 n_left = sub.getLength();
            int32 n_zeros = n_window_samples_ - n_left ;

            sub *= gen.drawWindow(n_left, type);

            sub << gen.silence(n_zeros);
        }

        // Forward FFT
        FFTChunkVector vec = fft_->fft(sub, nfft_, 0);

        (*real_)[k] = vec[0].getReal();
        (*imag_)[k] = vec[0].getImaginary();
        ++k;

        if(once)
        {
            once = false;
            *frequency_axis_ = vec[0].getFrequencyAxis().subbuffer(1);
        }

        i += window_step;
        time += time_step;
    }
}

Spectrogram::
Spectrogram(const Spectrogram & copy)
    :
    frequency_axis_(new Buffer(*copy.frequency_axis_)),
    time_axis_(new Buffer(*copy.time_axis_)),
    real_(new AudioStream(*copy.real_)),
    imag_(new AudioStream(*copy.imag_)),
    fft_window_(new Buffer(*copy.fft_window_)),
    nfft_(copy.nfft_),
    n_window_samples_(copy.n_window_samples_),
    fft_(new FFTransform(*copy.fft_))
{
}

Spectrogram::
~Spectrogram()
{
    delete frequency_axis_;
    delete time_axis_;
    delete real_;
    delete imag_;
    delete fft_window_;
    delete fft_;
};

Buffer
Spectrogram::
getFrequencyAxis() const
{
    return Buffer(*frequency_axis_);
}

AudioStream
Spectrogram::
getMagnitude() const
{
    return ((*real_^2.0) + (*imag_^2.0))^0.5;
}

Buffer
Spectrogram::
getTimeAxis() const
{
    return Buffer(*time_axis_);
}

Spectrogram &
Spectrogram::
operator=(const Spectrogram & rhs)
{
    if(this == &rhs)
    {
        return *this;
    }

    sample_rate_      = rhs.sample_rate_;
    *frequency_axis_  = *rhs.frequency_axis_;
    *time_axis_       = *rhs.time_axis_;
    *real_            = *rhs.real_;
    *imag_            = *rhs.imag_;
    *fft_window_      = *rhs.fft_window_;
    nfft_             = rhs.nfft_;
    n_window_samples_ = rhs.n_window_samples_;
    *fft_             = *rhs.fft_;

    return *this;
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
    pylab.imagesc(*time_axis_, *frequency_axis_, mag);
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
    signal *= *fft_window_;

    // Forward FFT
    FFTChunkVector vec = fft_->fft(signal, nfft_, 0);

    return vec[0].getMagnitude();
}