//-----------------------------------------------------------------------------
//
//  $Id: FFTransform.cc 874 2014-09-08 02:21:29Z weegreenblobbie $
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

#include <Nsound/Buffer.h>
#include <Nsound/FFTChunk.h>
#include <Nsound/FFTransform.h>
#include <Nsound/Generator.h>
#include <Nsound/Plotter.h>

#include <cmath>

using namespace Nsound;

//-----------------------------------------------------------------------------
FFTransform::
FFTransform(const float64 & sample_rate)
    :
    sample_rate_(static_cast<uint32>(sample_rate)),
    type_(RECTANGULAR)
{
}

Buffer
FFTransform::
fft(const Buffer & time_domain) const
{
    int32 N = roundUp2(time_domain.getLength());

    FFTChunkVector v = fft(time_domain, N);

    if(v.size() >= 1)
    {
        // Calculate the magnitude of the frequency domain.
        Buffer f_domain = v[0].getMagnitude();

        // Resample so to return exacly sample_rate / 2 samples.
        float64 sr_2 = sample_rate_ / 2.0;

        float64 factor = static_cast<float64>(f_domain.getLength()) / sr_2;

        // Simple down sample.
        return f_domain.getSpeedUp(factor);
    }

    return time_domain;
}

FFTChunkVector
FFTransform::
fft(const Buffer & input, int32 n_order, int32 n_overlap) const
{
    const int32 N = roundUp2(n_order);

    const int32 input_length = input.getLength();

    FFTChunkVector vec;

    Generator gen(1);

    Buffer fft_window = gen.drawWindow(N, type_);

    for(int32 n = 0; n < input_length; n += (N - n_overlap))
    {
        // Create an FFTChunk to operate on.
        FFTChunk chunk(N, sample_rate_, input.getLength());

        // Grab N samples from the input buffer

        Buffer sub_signal = input.subbuffer(n,N);

        // Apply window
        int32 sub_length = sub_signal.getLength();
        if(sub_length == N)
        {
            sub_signal *= fft_window;
        }
        else
        {
            sub_signal *= gen.drawWindow(sub_length, type_);
        }

        (*chunk.real_) = sub_signal;

        int32 chunk_size = chunk.real_->getLength();

        // If there is less than N samples, pad with zeros
        for(int32 i = 0; i < N - chunk_size; ++i)
        {
            (*chunk.real_) << 0.0;
        }

        // Zero out the imaginary side.
        (*chunk.imag_) = gen.silence(N);

        fft(*chunk.real_, *chunk.imag_, N);

        vec.push_back(chunk);
    }

    return vec;
}

void
FFTransform::
fft(Buffer & real, Buffer & img, const int32 N) const
{
    const float64 pi = M_PI;
    const int32 n_minus_1 = N - 1;
    const int32 n_devide_2 = N / 2;
    const int32 m = static_cast<uint32>(
        std::log10(static_cast<float64>(N)) / std::log10(2.0) + 0.5);

    int32 j = n_devide_2;

    // Bit reversal sorting.
    for(int32 i = 1; i <= N - 2 ; ++i)
    {
        if(i < j)
        {
            float64 temp_real = real[j];
            float64 temp_img  = img[j];

            real[j] = real[i];
            img[j]  = img[i];

            real[i] = temp_real;
            img[i]  = temp_img;
        }

        int32 k = n_devide_2;

        while(k <= j)
        {
            j -= k;
            k /= 2;
        }
        j += k;
    }

    // Loop for each fft stage.
    for(int32 l = 1; l <= m; ++l)
    {
        int32 le = static_cast<int32>(std::pow(2.0, l) + 0.5);
        int32 le2 = le / 2;

        float64 ur = 1.0;
        float64 ui = 0.0;

        // Calculate sine and cosine values.
        float64 sr = std::cos(pi / static_cast<float64>(le2));
        float64 si = -1.0 * std::sin(pi / static_cast<float64>(le2));

        // Loop for each sub DFT.
        for(j = 1; j <= le2; ++j)
        {
            int32 j_minus_1 = j - 1;

            // Loop for each butterfly.
            for(int32 i = j_minus_1; i <= n_minus_1; i += le)
            {
                int32 ip = i + le2;

                // Butterfly calculation.
                float64 temp_real = ur * real[ip] - ui * img[ip];
                float64 temp_img  = ui * real[ip] + ur * img[ip];

                real[ip] = real[i] - temp_real;
                img[ip]  = img[i]  - temp_img;

                real[i] += temp_real;
                img[i]  += temp_img;
            }
            float64 temp = ur;
            ur = temp * sr - ui * si;
            ui = temp * si + ui * sr;
        }
    }
}

Buffer
FFTransform::
ifft(const FFTChunkVector & vec) const
{
    Buffer output;

    FFTChunkVector::const_iterator itor = vec.begin();
    FFTChunkVector::const_iterator end = vec.end();

    const uint32 N = roundUp2(itor->real_->getLength() - 2);
    const float64 f_N = static_cast<float64>(N);

    while(itor != end)
    {
        FFTChunk chunk(*itor);

        if(chunk.isPolar()) chunk.toCartesian();

        // Change the sign of img.
        *chunk.imag_ *= -1.0;

        // Perform the forwared fft
        fft(*chunk.real_, *chunk.imag_, N);

        *chunk.real_ = chunk.real_->subbuffer(0, itor->getOriginalSize());

        // Divide the real by N.
        *chunk.real_ /= f_N;

        // Change the sign of img.  But the img is never used again so don't
        // bother.
//~        img *= -1.0;

        output << *chunk.real_;

        ++itor;
    }

    return output;
}

Buffer
FFTransform::
ifft(const Buffer & frequency_domain) const
{
    int32 N = roundUp2(frequency_domain.getLength());

    FFTChunk chunk(N, sample_rate_);

    *chunk.real_ << frequency_domain;

    for(uint32 i = 0; i < 2 * (N - frequency_domain.getLength()); ++i)
    {
        *chunk.real_ << 0.0;
    }

    *chunk.real_ << frequency_domain.getReverse();

    *chunk.imag_ = 0.0 * *chunk.real_;

    FFTChunkVector vec;

    vec.push_back(chunk);

    return ifft(vec).subbuffer(0, frequency_domain.getLength());
}

int32
FFTransform::
roundUp2(int32 raw)
{
    raw = static_cast<int32>(::fabs(static_cast<float64>(raw - 1)));

    int32 n;

    n = 1;
    while(raw)
    {
        n   <<= 1;  // Multiply n by 2
        raw >>= 1;  // Divide raw by 2
    }

    return n;
}

void
FFTransform::
setWindow(WindowType type)
{
    type_ = type;
}
