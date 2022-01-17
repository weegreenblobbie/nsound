//////////////////////////////////////////////////////////////////////////////
//
//  $Id: Buffer.cc 913 2015-08-08 16:41:22Z weegreenblobbie $
//
//  Copyright (c) 2004-2006 Nick Hilton
//
//  weegreenblobbie2_gmail_com (replace '_' with '@' and '.')
//
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
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
//////////////////////////////////////////////////////////////////////////////

#include <Nsound/AudioStream.h>
#include <Nsound/Buffer.h>
#include <Nsound/BufferWindowSearch.h>
#include <Nsound/DelayLine.h>
#include <Nsound/FFTransform.h>
#include <Nsound/FilterLeastSquaresFIR.h>
#include <Nsound/FilterLowPassIIR.h>
#include <Nsound/FilterMovingAverage.h>
#include <Nsound/Generator.h>
#include <Nsound/Nsound.h>
#include <Nsound/Plotter.h>
#include <Nsound/StreamOperators.h>
#include <Nsound/Wavefile.h>


#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>

using std::cerr;
using std::cout;
using std::endl;
using std::flush;
using std::ofstream;
using std::ostream;
using std::string;

namespace Nsound
{


Buffer::
Buffer()
    :
    data_()
{
    data_.reserve(4096);
}

Buffer::
Buffer(uint32 chunk_size)
    :
    data_()
{
    data_.reserve(chunk_size);
}

Buffer::
Buffer(const FloatVector & list)
    :
    data_(list) // Copies contents
{
}

Buffer::
Buffer(const std::string & filename, uint32 chunk_size)
    :
    data_()
{
    data_.reserve(chunk_size);

    // This function is defined in Wavefile.h.
    *this << filename.c_str();
}

Buffer::
Buffer(const Buffer & copy)
    :
    data_(copy.data_)
{
}

Buffer::
Buffer(Buffer && move)
    :
    data_()
{
    std::swap(data_, move.data_);
}


Buffer::
~Buffer()
{
}


void
Buffer::
abs()
{
    for(uint32 i = 0; i < data_.size(); ++i)
    {
        data_[i] = ::fabs(data_[i]);
    }
}

//  There are 11 cases to consider.
//
//  c = a + b
//
//  Case 1: n_samples == 0
//          && a_length < offset
//
//           [aaaaaaa]
//         + |--offset--|[bbbbbbbbbbbbbbbbbbbbbb]
//  result = |aaaaaaaa---bbbbbbbbbbbbbbbbbbbbbbb|
//
//  Case 2: n_samples != 0
//          && a_length < offset
//          && n_samples <= b_length
//
//           [aaaaaaa]
//         + |--offset--|[bbbbbbbbbbbbbbbbbbbbbb]
//                       |---n_samples---|
//  result = |aaaaaaaa---bbbbbbbbbbbbbbbb|
//
//  Case 3: n_samples != 0
//          && a_length < offset
//          && n_samples > b_length
//
//           [aaaaaaa]
//         + |--offset--|[bbbbbbbbbbbbbbbbbbbbbb]
//                       |---n_samples----------------|
//  result = |aaaaaaaa---bbbbbbbbbbbbbbbbbbbbbbbb-----|
//
//
//  Case 4: n_samples == 0
//          && a_length >= offset
//          && a_length < offset + b_length
//
//           [aaaaaaaaaaaaaaaaaaaaaaaaaaaa]
//         + |--offset--|[bbbbbbbbbbbbbbbbbbbbbb]
//  result = |aaaaaaaaaaaccccccccccccccccccbbbbb|
//
//
//  Case 5: n_samples != 0
//          && a_length >= offset
//          && n_samples <= b_length
//          && a_length <= offset + n_samples
//
//           [aaaaaaaaaaaaaaaaaaaa]
//         + |--offset--|[bbbbbbbbbbbbbbbbbbbbbb]
//                      |----n_samples----|
//  result = |aaaaaaaaaacccccccccccbbbbbbb|
//
//
//  Case 6: n_samples != 0
//          && a_length >= offset
//          && n_samples < b_length
//          && a_length > offset + n_samples
//
//           [aaaaaaaaaaaaaaaaaaaaaaaaaaaa]
//         + |--offset--|[bbbbbbbbbbbbbbbbbbbbbb]
//                      |--n_samples--|
//  result = |aaaaaaaaaacccccccccccccccaaa|
//
//  Case 7: n_samples != 0
//          && a_length >= offset
//          && n_samples > b_length
//          && a_length <= offset + b_length
//
//           [aaaaaaaaaaaaaaaaaaaa]
//         + |--offset--|[bbbbbbbbbbbbbbbbbbbbbb]
//                      |----n_samples---------------|
//  result = |aaaaaaaaaaaccccccccccbbbbbbbbbbbbbb----|
//
//
//  Case 8: n_samples == 0
//          && a_length >= offset + b_length
//
//           [aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa]
//         + |--offset--|[bbbbbbbbbbbbbbbbbbbbbb]
//  result = |aaaaaaaaaaaccccccccccccccccccccccccaaaaaa|
//
//  Case 10: n_samples != 0
//           && a_length >= offset + n_samples
//           && n_samples >= b_length
//
//           [aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa]
//         + |--offset--|[bbbbbbbbbbbbbbbbbbbbb]
//                      |----n_samples---------------|
//  result = |aaaaaaaaaaacccccccccccccccccccccccaaaaaaa|
//
//  Case 11: n_samples != 0
//           && a_length > offset + b_length
//           && a_length < offset + n_samples
//
//           [aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa]
//         + |--offset--|[bbbbbbbbbbbbbbbbbbbbb]
//                      |----n_samples----------------------|
//  result = |aaaaaaaaaaacccccccccccccccccccccccaaaaaaaa----|
//
void
Buffer::
add(const Buffer & b,
    uint32 offset,
    uint32 n_samples)
{
    uint32 a_length = getLength();
    uint32 b_length = b.getLength();

    //  Case 1: n_samples == 0
    //          && a_length < offset
    //
    //           [aaaaaaa]
    //         + |--offset--|[bbbbbbbbbbbbbbbbbbbbbb]
    //  result = |aaaaaaaa---bbbbbbbbbbbbbbbbbbbbbbb|
    if(n_samples == 0 && a_length < offset)
    {
        // Append zeros
        for(uint32 i = a_length; i < offset; ++i)
        {
            *this << 0.0;
        }

        // Append b
        *this << b;
    }

    //  Case 2: n_samples != 0
    //          && a_length < offset
    //          && n_samples <= b_length
    //
    //           [aaaaaaa]
    //         + |--offset--|[bbbbbbbbbbbbbbbbbbbbbb]
    //                       |---n_samples---|
    //  result = |aaaaaaaa---bbbbbbbbbbbbbbbb|
    else if(n_samples != 0
            && a_length < offset
            && n_samples <= b_length)
    {
        // Append zeros
        for(uint32 i = a_length; i < offset; ++i)
        {
            *this << 0.0;
        }

        for(uint32 i = 0; i < n_samples; ++i)
        {
            *this << b[i];
        }
    }

    //  Case 3: n_samples != 0
    //          && a_length < offset
    //          && n_samples > b_length
    //
    //           [aaaaaaa]
    //         + |--offset--|[bbbbbbbbbbbbbbbbbbbbbb]
    //                       |---n_samples----------------|
    //  result = |aaaaaaaa---bbbbbbbbbbbbbbbbbbbbbbbb-----|
    else if(n_samples != 0
        && a_length < offset
        && n_samples > b_length)
    {
        // Append zeros
        for(uint32 i = a_length; i < offset; ++i)
        {
            *this << 0.0;
        }

        // Append b samples

        *this << b;

        // Append zeros

        for(uint32 i = b_length; i < n_samples; ++i)
        {
            *this << 0.0;
        }
    }

    //  Case 4: n_samples == 0
    //          && a_length >= offset
    //          && a_length < offset + b_length
    //
    //           [aaaaaaaaaaaaaaaaaaaaaaaaaaaa]
    //         + |--offset--|[bbbbbbbbbbbbbbbbbbbbbb]
    //  result = |aaaaaaaaaaaccccccccccccccccccbbbbb|
    else if(n_samples == 0
        && a_length >= offset
        && a_length < offset + b_length)
    {
        uint32 b_index = 0;
        for(uint32 i = offset; i < a_length; ++i)
        {
            data_[i] += b[b_index++];
        }

        for(uint32 i = b_index; i < b_length; ++i)
        {
            *this << b[i];
        }
    }

    //  Case 5: n_samples != 0
    //          && a_length >= offset
    //          && n_samples <= b_length
    //          && a_length <= offset + n_samples
    //
    //           [aaaaaaaaaaaaaaaaaaaa]
    //         + |--offset--|[bbbbbbbbbbbbbbbbbbbbbb]
    //                      |----n_samples----|
    //  result = |aaaaaaaaaacccccccccccbbbbbbb|
    else if(n_samples != 0
        && a_length >= offset
        && a_length <= offset + n_samples
        && n_samples <= b_length)
    {
        uint32 b_index = 0;
        for(uint32 i = offset; i < a_length; ++i)
        {
            data_[i] += b[b_index++];
        }

        for(uint32 i = b_index; i < n_samples; ++i)
        {
            *this << b[i];
        }
    }
    //  Case 6: n_samples != 0
    //          && a_length >= offset
    //          && n_samples < b_length
    //          && a_length > offset + n_samples
    //
    //           [aaaaaaaaaaaaaaaaaaaaaaaaaaaa]
    //         + |--offset--|[bbbbbbbbbbbbbbbbbbbbbb]
    //                      |--n_samples--|
    //  result = |aaaaaaaaaacccccccccccccccaaa|
    else if(n_samples != 0
        && a_length >= offset
        && a_length > offset + n_samples
        && n_samples < b_length)
    {
        uint32 b_index = 0;
        for(uint32 i = offset; i < offset + n_samples; ++i)
        {
            data_[i] += b[b_index++];
        }
    }

    //  Case 7:    a_length >= offset
    //          && n_samples > b_length
    //          && a_length <= offset + b_length
    //
    //           [aaaaaaaaaaaaaaaaaaaa]
    //         + |--offset--|[bbbbbbbbbbbbbbbbbbbbbb]
    //                      |----n_samples---------------|
    //  result = |aaaaaaaaaaaccccccccccbbbbbbbbbbbbbb----|
    else if(a_length >= offset
        &&  a_length <= offset + b_length
        &&  n_samples > b_length)
    {
        uint32 b_index = 0;
        for(uint32 i = offset; i < a_length; ++i)
        {
            data_[i] += b[b_index++];
        }

        for(uint32 i = b_index; i < b_length; ++i)
        {
            *this << b[i];
        }

        for(uint32 i = b_length; i < n_samples; ++i)
        {
            *this << 0.0;
        }
    }
    //  Case 8: n_samples == 0
    //          && a_length >= offset + b_length
    //
    //           [aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa]
    //         + |--offset--|[bbbbbbbbbbbbbbbbbbbbbb]
    //  result = |aaaaaaaaaaaccccccccccccccccccccccccaaaaaa|
    else if(n_samples == 0
        && a_length >= offset + b_length)
    {
        uint32 b_index = 0;
        for(uint32 i = offset; i < offset + b_length; ++i)
        {
            data_[i] += b[b_index++];
        }
    }

    //  Case 9:     a_length >= offset + n_samples
    //           && n_samples >= b_length
    //
    //  result = [aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa]
    //         + |--offset--|[bbbbbbbbbbbbbbbbbbbbb]
    //                      |----n_samples------------|
    //         = |aaaaaaaaaaaacccccccccccccccccccccaaaaaaaa|
    else if( a_length >= offset + n_samples
        &&   n_samples >= b_length)
    {
        uint32 b_index = 0;
        for(uint32 i = offset; i < offset + b_length; ++i)
        {
            data_[i] += b[b_index++];
        }
    }
    //  Case 10: n_samples != 0
    //           && a_length > offset + b_length
    //           && a_length < offset + n_samples
    //
    //  result = [aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa]
    //         + |--offset--|[bbbbbbbbbbbbbbbbbbbbb]
    //                      |----n_samples----------------------|
    //         = |aaaaaaaaaaaccccccccccccccccccccccaaaaaaa------|
    else if(n_samples != 0
        && a_length > offset + b_length
        && a_length < offset + n_samples)
    {
        uint32 b_index = 0;
        for(uint32 i = offset; i < offset + b_length; ++i)
        {
            data_[i] += b[b_index++];
        }

        for(uint32 i = a_length; i < offset + n_samples; ++i)
        {
            *this << 0.0;
        }
    }
}

uint32
Buffer::
argmax() const
{
    uint32 index = 0;

    Buffer::const_iterator itor = begin();
    Buffer::const_iterator tend = end();

    float64 max = *itor;

    uint32 i = 0;
    while(itor != tend)
    {
        if(*itor > max)
        {
            max = *itor;
            index = i;
        }
        ++i;
        ++itor;
    }
    return index;
}

uint32
Buffer::
argmin() const
{
    uint32 index = 0;

    Buffer::const_iterator itor = begin();
    Buffer::const_iterator tend = end();

    float64 min = *itor;

    uint32 i = 0;
    while(itor != tend)
    {
        if(*itor < min)
        {
            min = *itor;
            index = i;
        }
        ++i;
        ++itor;
    }
    return index;
}

void
Buffer::
exp()
{
    for(uint32 i = 0; i < getLength(); ++i)
    {
        data_[i] = ::exp(data_[i]);
    }
}

void
Buffer::
convolve(const Buffer & H)
{
    *this = getConvolve(H);
}

Buffer
Buffer::
getConvolve(const Buffer & H) const
{
    // Alocate the output Buffer will all zeros.
    Buffer y = *this * 0.0;

    // Pad y with H.getLength() samples.
    for(uint32 i = 0; i < H.getLength(); ++i)
    {
        y << 0.0;
    }

//~    uint32 N = getLength() / 1000;

//~    printf("%6.2f", 0.0);

    // For each sample in this Buffer.
    for(uint32 i = 0; i < getLength(); ++i)
    {
//~        // DEBUG
//~        if(i % N == 0)
//~        {
//~            float64 p_done = 100.0 * i / float64(getLength());
//~            printf("\b\b\b\b\b\b%6.2f", p_done);
//~            fflush(stdout);
//~        }

        // For each sample in H.
        for(uint32 j = 0; j < H.getLength(); ++j)
        {
            y[i + j] += (*this)[i] * H[j];
        }
    }

//~    printf("\b\b\b\b\b\b%6.2f\n", 100.0);

    return y;
}

void
Buffer::
dB()
{
//~    FloatVector::iterator itor = data_.begin();
//~    FloatVector::iterator end  = data_.end();

//~    while(itor != end)
//~    {
//~        if(*itor == 0.0)
//~        {
//~            *itor = -150.0;
//~        }
//~        else
//~        {
//~            *itor = 20.0 * ::log10(*itor);
//~        }

//~        ++itor;
//~    }

    log10();
    *this *= 20.0;
}

void
Buffer::
derivative(uint32 n)
{
    for(uint32 pass = 0; pass < n; ++pass)
    {
        for(uint32 i = 1; i < data_.size(); ++i)
        {
            data_[i - 1] = (data_[i] - data_[i - 1]);
        }
    }
}

void
Buffer::
downSample(uint32 n)
{
    *this = getDownSample(n);
}

// This is based on the downsampling description in "Descrete-Time Signal
// Processing" 2nd Eddition aka "the Oppenheim book".  Section 4.6.2 page 172.
//
Buffer
Buffer::
getDownSample(uint32 M) const
{
    M_ASSERT_VALUE(M, >, 0);
    return getResample(static_cast<uint32>(1),M);
}

Uint32Vector
Buffer::
findPeaks(
    uint32 window_size,
    float64 min_height) const
{
    Uint32Vector peaks;
    peaks.reserve(128);

    uint32 h_window_size = window_size / 2 + 1;  // half window size

    uint32 n_samples = getLength();

    for(uint32 i = 1; i < n_samples - 1; ++i)
    {
        boolean is_peak = false;

        float64 sample = data_[i];

        if(window_size <= 2)
        {
            if( sample > data_[i - 1] &&
                sample > data_[i + 1])
            {
                is_peak = true;
            }
        }
        else
        {
            // Assume it's a peak.

            is_peak = true;

            for(uint32 j = 1; j < h_window_size; ++j)
            {
                if(j < i)
                {
                    if(data_[i - j] > sample)
                    {
                        is_peak = false;
                        break;
                    }
                }

                if(i + j < n_samples)
                {
                    if(data_[i + j] > sample)
                    {
                        is_peak = false;
                        break;
                    }
                }
            }
        }

        if(is_peak && sample > min_height)
        {
            peaks.push_back(i);
        }
    }

    return peaks;
}


/*

Buffer
Buffer::
findPitch(float64 sample_rate)
{
    // This is based on the YIN pitch estimation algorithm.

    // Create a window size that will fit 2 cycles of 20 Hz, our minimum
    // detectable frequency.

    uint32 window_size = static_cast<uint32>(2.0 * sample_rate / 20.0);

    Buffer::const_iterator input = this->begin();
    Buffer::const_iterator end   = this->end();

    DelayLine dl(1, window_size);

    // First time, write window_size /2 zeros.
    for(uint32 i = 0; i < window_size / 2; ++i)
    {
        dl.write(0.0);
    }

    // FIXME: what if window_size > getLength()?
    // write window_size / 2 samples.
    for(uint32 i = 0; i < window_size / 2; ++i)
    {
        dl.write(*input);
        ++input;
    }

    uint32 input_done_count = 0;
    boolean input_done = false;

    Generator gen(1);
    FFTransform fft(1);

    Buffer window = gen.drawFatGaussian(window_size, 0.95);

    Buffer y(window_size);

    uint32 i = 0;
    while(true)
    {
        // Grab a window of samples.
        Buffer temp(window_size);

        // Read the delay line.
        for(uint32 j = 0; j < window_size; ++j)
        {
            temp << dl.read();
        }

        // Apply window.
        temp *= window;

        ///////////////////////////////////////////////////////////////////////
        // Calculate Autocorreletion using the FFT.

        // Perform the FFT.
        FFTChunkVector vec = fft.fft(temp, 2 * window_size);

        Buffer power(2 * window_size);
        for(uint32 j = 0; j < 2 * window_size; ++j)
        {
            float64 r = (*vec[0].real_)[j];
            float64 i = (*vec[0].imag_)[j];

            power[j] = r*r + i*i;
        }

        // Inverse fft to get autocorrelation.
        *vec[0].real_ = power;
        *vec[0].imag_  = gen.silence(2*window_size);

        Buffer acf = fft.ifft(vec);

        ///////////////////////////////////////////////////////////////////////
        // Calculate the difference.
        Buffer d(window_size);

        float64 rtTau = acf[0];

        uint32 hs = window_size / 2;

        for(uint32 j = 0; j < hs; ++j)
        {
            d << acf[0] + rtTau - (2.0 * acf[j]);

            rtTau += (temp[hs + j] * temp[hs + j]) - (temp[j] * temp[j]);
        }

        ///////////////////////////////////////////////////////////////////////
        // Calculate the cumlative normal.

        Buffer d_primes(window_size);

        float64 denom = d[0];

        d_primes << 1.0;

        for(uint32 j = 1; j < hs; ++j)
        {
            denom += d[j];

            d_primes << ((j-1) * d[j]) / (denom + 1e-20);
        }

        ///////////////////////////////////////////////////////////////////////
        // Calculate max period

        float64 f0 = 0.0;
        float64 min = 1000.0;
        for(uint32 j = 44; j < 540; ++j)
        {
            if(d_primes[j] < min)
            {
                min = d_primes[j];
                f0 = j + 1;
            }
        }

        if(min >= 1000.0)
        {
            f0 = 0.0;
        }
        else
        {
            // Convert from samples to time, then frequency.
            f0 *= (1.0 / sample_rate);
            f0 = 1.0 / f0;
        }

        y << f0;

        // Read data for next loop.
        if(input_done)
        {
            ++input_done_count;

            if(input_done_count > 2) break;
        }

        // Read in next chunk of input samples.
        for(uint32 j = 0; j < window_size; ++j)
        {
            if(input == end)
            {
                input_done = true;

                dl.read();
                dl.write(0.0);
            }
            else
            {
                dl.read(); // move the read pointer.
                dl.write(*input);
                ++input;
            }
        }
    }

    return y;
}
*/

//~float64
//~Buffer::
//~get_at_index(int32 index) const
//~{
//~    int32 len = getLength();

//~    if(index >= 0 && index < len)
//~    {
//~        return data_[index];
//~    }
//~    else if(index < 0 && index >= -len)
//~    {
//~        return data_[len + index];
//~    }

//~    M_THROW(
//~        "IndexError: " << index << " is out of bounds (0 : " << len << ")");

//~    return 0.0; // keep compiler quiet
//~}

void
Buffer::
limit(float64 min, float64 max)
{
    Buffer::iterator itor = this->begin();
    Buffer::iterator end  = this->end();

    while(itor != end)
    {
        float64 s = *itor;

        if(s > max) s = max;
        if(s < min) s = min;

        *itor = s;

        ++itor;
    }
}

void
Buffer::
limit(const Buffer & min, const Buffer & max)
{
    Buffer::const_circular_iterator imin = min.cbegin();
    Buffer::const_circular_iterator imax = max.cbegin();

    Buffer::iterator itor = this->begin();
    Buffer::iterator end  = this->end();

    while(itor != end)
    {
        float64 s = *itor;

        if(s > *imax) s = *imax;
        if(s < *imin) s = *imin;

        *itor = s;

        ++itor;
        ++imax;
        ++imin;
    }
}

#define LM_MAX(a,b) ((a) > (b)) ? (a) : (b)

void
Buffer::
log()
{
    for(uint32 i = 0; i < getLength(); ++i)
    {
        // Avoid taking log of really tiny numbers.
        float64 t = LM_MAX(data_[i], 1e-9);

        data_[i] = ::log(t);
    }
}

void
Buffer::
log10()
{
    for(uint32 i = 0; i < getLength(); ++i)
    {
        // Avoid taking log of really tiny numbers.
        float64 t = LM_MAX(data_[i], 1e-9);

        data_[i] = ::log10(t);
    }
}

#undef LM_MAX

float64
Buffer::
getMax() const
{
    M_ASSERT_VALUE(data_.size(), >=, 1);

    float64 max = data_[0];

    for(const_iterator itor = data_.begin();
        itor != data_.end();
        ++itor)
    {
        float64 t = *itor;
        if(t > max)
        {
            max = t;
        }
    }

    return max;
}

float64
Buffer::
getMaxMagnitude() const
{
    M_ASSERT_VALUE(data_.size(), >=, 1);

    float64 max = data_[0];

    for(const_iterator itor = data_.begin();
        itor != data_.end();
        ++itor)
    {
        float64 t = std::fabs(*itor);
        if( t > max)
        {
            max = t;
        }
    }

    return max;
}

float64
Buffer::
getMean() const
{
    float64 sum = getSum();

    return sum / static_cast<float64>(getLength());
}

float64
Buffer::
getMin() const
{
    M_ASSERT_VALUE(data_.size(), >=, 1);

    float64 min = data_[0];

    for(const_iterator itor = data_.begin();
        itor != data_.end();
        ++itor)
    {
        if(*itor < min)
        {
            min = *itor;
        }
    }

    return min;
}

void
Buffer::
mul(const Buffer & buffer, uint32 offset, uint32 n_samples)
{
    M_ASSERT_VALUE(offset, >=, 0);
    M_ASSERT_VALUE(offset, <, getLength());

    uint32 b0 = offset;
    uint32 b1 = 0;

    uint32 n = n_samples;

    if(n_samples == 0)
    {
        n = buffer.getLength();
    }

    b1 = b0 + n;

    if(b1 > getLength())
    {
        b1 = getLength();
        n = b1 - b0;
    }

    // Limit
    if(n > buffer.getLength())
    {
        b1 = b0 + buffer.getLength();
    }

    uint32 k = 0;
    for(uint32 i = b0; i < b1; ++i)
    {
        (*this)[i] *= buffer[k];
        ++k;
    }
}

void
Buffer::
normalize()
{
    float64 peak = getMaxMagnitude();

    float64 scale = 1.0;

    if(peak != 0.0) scale = 1.0 / peak;

    *this *= scale;
}

Buffer
Buffer::
getSignalEnergy(uint32 N) const
{
    float64 n = static_cast<float64>(N);

    Buffer y;

    uint32 length = getLength();

    for(uint32 i = 0; i < length; ++i)
    {
        float64 sum = 0.0;

        for(uint32 j = 0; j < N; ++j)
        {
            if(i + j < length - 1)
            {
                sum += ::fabs(data_[i + j]);
            }
        }
        sum /= n;

        y << sum;

    }

    return y;
}

float64
Buffer::
getStd() const
{
    Buffer diff = *this - getMean();

    diff ^= 2.0;

    return ::sqrt(diff.getSum() / static_cast<float64>(getLength()));
}

float64
Buffer::
getSum() const
{
    float64 sum = 0.0;

    for(const_iterator itor = data_.begin();
        itor != data_.end();
        ++itor)
    {
        sum += *itor;
    }

    return sum;
}

void
Buffer::
zNorm()
{
    // z = (x - mean(x)) / std(x)

    // We won't call getStd() to avoid calculatig the diff and mean twice.

    // z = (x - mean(x)) / sqrt( sum((x - mean(x))^2) / N)

    float64 mean = getMean();

    Buffer diff = *this - mean;

    float64 std = (diff*diff).getSum() / static_cast<float64>(getLength());

    std = ::sqrt(std);

    *this = diff / (std + 1e-20);
}


Buffer &
Buffer::
operator=(const Buffer & rhs_buffer)
{
    if(this == &rhs_buffer) // If it is the same object, return it.
    {
        return *this;
    }

    data_ = rhs_buffer.data_;

    return *this;
}

BufferSelection
Buffer::
operator()(const BooleanVector & bv)
{
    BufferSelection bs(*this, bv);

    return bs;
}

boolean
Buffer::
operator==(const Buffer & rhs_buffer) const
{
    if(data_.size() != rhs_buffer.data_.size())
    {
        return false;
    }

    for(uint32 i = 0; i < data_.size(); ++i)
    {
        if(data_[i] != rhs_buffer.data_[i])
        {
            return false;
        }
    }

    return true;
}

boolean
Buffer::
operator!=(const Buffer & rhs_buffer) const
{
    if(data_.size() != rhs_buffer.data_.size())
    {
        return true;
    }

    for(uint32 i = 0; i < data_.size(); ++i)
    {
        if(data_[i] != rhs_buffer.data_[i])
        {
            return true;
        }
    }

    return false;
}

Buffer &
Buffer::
operator<<(const AudioStream & rhs)
{
    return (*this) << rhs.getMono()[0];
}

Buffer &
Buffer::
operator<<(const Buffer & rhs_buffer)
{
    if(this == & rhs_buffer)
    {
        uint32 size = getLength();

        for(uint32 i = 0; i < size; ++i)
        {
            data_.push_back(data_[i]);
        }

        return *this;
    }

    std::vector<float64>::const_iterator itor = rhs_buffer.data_.begin();
    std::vector<float64>::const_iterator end = rhs_buffer.data_.end();

    while(itor != end)
    {
        data_.push_back(*itor);
        ++itor;
    }

    return *this;
}

Buffer &
Buffer::
operator+=(const Buffer & rhs)
{
    std::size_t N = std::min(getLength(), rhs.getLength());
    for(std::size_t i = 0; i < N; ++i) data_[i] += rhs.data_[i];
    return *this;
}

Buffer &
Buffer::
operator-=(const Buffer & rhs)
{
    std::size_t N = std::min(getLength(), rhs.getLength());
    for(std::size_t i = 0; i < N; ++i) data_[i] -= rhs.data_[i];
    return *this;
}

Buffer &
Buffer::
operator*=(const Buffer & rhs)
{
    std::size_t N = std::min(getLength(), rhs.getLength());
    for(std::size_t i = 0; i < N; ++i) data_[i] *= rhs.data_[i];
    return *this;
}

Buffer &
Buffer::
operator/=(const Buffer & rhs)
{
    std::size_t N = std::min(getLength(), rhs.getLength());
    for(std::size_t i = 0; i < N; ++i) data_[i] /= rhs.data_[i];
    return *this;
}

Buffer &
Buffer::
operator^=(const Buffer & rhs)
{
    std::size_t N = std::min(getLength(), rhs.getLength());
    for(std::size_t i = 0; i < N; ++i) data_[i] = std::pow(data_[i], rhs.data_[i]);
    return *this;
}

Buffer &
Buffer::
operator+=(float64 d)
{
    for(auto & x : data_) x += d;
    return *this;
}

Buffer &
Buffer::
operator-=(float64 d)
{
    for(auto & x : data_) x -= d;
    return *this;
}

Buffer &
Buffer::
operator*=(float64 d)
{
    for(auto & x : data_) x *= d;
    return *this;
}

Buffer &
Buffer::
operator/=(float64 d)
{
    for(auto & x : data_) x /= d;
    return *this;
}

Buffer &
Buffer::
operator^=(float64 d)
{
    for(auto & x : data_) x = std::pow(x, d);
    return *this;
}

std::ostream &
operator<<(std::ostream & out, const Buffer & rhs_buffer)
{
    std::vector<float64>::const_iterator rhs = rhs_buffer.data_.begin();
    std::vector<float64>::const_iterator end = rhs_buffer.data_.end();

    while(rhs != end)
    {
        out << *rhs << endl;
        ++rhs;
    }

    return out;
}

//~	///////////////////////////////////////////////////////////////////////////////
//~	void
//~	Buffer::
//~	pitchShift(uint32 window_size, uint32 overlap_size, int32 n_bins_to_shift )
//~	{
//~		int32 length = this->getLength();
//~
//~		uint32 fft_size = FFTransform::roundUp2(window_size);
//~
//~		Buffer::const_iterator input = this->begin();
//~		Buffer::const_iterator end   = this->end();
//~
//~		DelayLine dl(1, window_size);
//~
//~		// First time, write window_size /2 zeros.
//~		for(uint32 i = 0; i < window_size / 2; ++i)
//~		{
//~			dl.write(0.0);
//~		}
//~
//~		// FIXME: what if window_size > getLength()?
//~		// write window_size / 2 samples.
//~		for(uint32 i = 0; i < window_size / 2; ++i)
//~		{
//~			dl.write(*input);
//~			++input;
//~		}
//~
//~		uint32 input_done_count = 0;
//~		boolean input_done = false;
//~
//~		Generator gen(1);
//~		FFTransform fft(1);
//~
//~	//~    Buffer window = gen.drawWindowHanning(window_size);
//~		Buffer window = gen.drawFatGaussian(window_size, 0.95);
//~
//~		Buffer y(length);
//~
//~		uint32 i = 0;
//~		while(true)
//~		{
//~			// Grab a window of samples.
//~			Buffer temp(window_size);
//~
//~			// Read the delay line.
//~			for(uint32 j = 0; j < window_size; ++j)
//~			{
//~				temp << dl.read();
//~			}
//~
//~			// Apply window.
//~			temp *= window;
//~
//~			// Perform the FFT.
//~			FFTChunkVector vec = fft.fft(temp, fft_size);
//~
//~			uint32 fft_length = vec[0].real_->getLength();
//~
//~			// Frequency Domain signal:
//~			// xxxxxxxxxxxxxxxxxxxxxxxxx-yyyyyyyyyyyyyyyyyyyyyyyyyy
//~			//                          ^
//~			//                       the center
//~
//~			int32 center = fft_length / 2;
//~
//~			// Shift all the bins by n_bins_to_shift.
//~			if(n_bins_to_shift >= 0)
//~			{
//~				// Shift the left half bins...
//~				for(uint32 j = center - n_bins_to_shift;
//~					j > 0;
//~					--j)
//~				{
//~					vec[0].real_->operator[](j + n_bins_to_shift) =
//~						vec[0].real_->operator[](j);
//~
//~					vec[0].imag_ ->operator[](j + n_bins_to_shift) =
//~						vec[0].imag_->operator[](j);
//~				}
//~			}
//~			else
//~			{
//~				// Shift the left half bins...
//~				for(int32 j = 0; j < center + n_bins_to_shift; ++j)
//~				{
//~					vec[0].real_->operator[](j) =
//~						vec[0].real_->operator[](j - n_bins_to_shift);
//~
//~					vec[0].imag_ ->operator[](j) =
//~						vec[0].imag_->operator[](j - n_bins_to_shift);
//~				}
//~			}
//~
//~			--fft_length;
//~			// Copy the left half into the right half.
//~			for(int32 j = 0; j < center; ++j)
//~			{
//~				vec[0].real_->operator[](fft_length - j) =
//~					vec[0].real_->operator[](j);
//~
//~				vec[0].imag_ ->operator[](fft_length - j) =
//~					-1.0 * vec[0].imag_->operator[](j);
//~			}
//~
//~			temp = fft.ifft(vec);
//~
//~			temp = temp.subbuffer(0, window_size);
//~			temp *= window;
//~
//~	//~        if(i == 4096)
//~	//~        {
//~	//~            char title[1024];
//~	//~
//~	//~            sprintf(title, "i = %4d: y", i);
//~	//~            y.plot(title);
//~	//~
//~	//~            sprintf(title, "i = %4d: temp", i);
//~	//~            temp.plot(title);
//~	//~        }
//~
//~			if(i >= overlap_size)
//~			{
//~				y.add(temp, y.getLength() - overlap_size);
//~			}
//~			else
//~			{
//~				y << temp;
//~			}
//~
//~			static int nick = 0;
//~
//~			if(nick < 3)
//~			{
//~				++nick;
//~				y.plot();
//~			}
//~
//~	//~        if(i == 4096)
//~	//~        {
//~	//~            char title[1024];
//~	//~
//~	//~            sprintf(title, "i = %4d: y + temp", i);
//~	//~            y.plot(title);
//~	//~        }
//~
//~			i += window_size;
//~
//~			// Read data for next loop.
//~			if(input_done)
//~			{
//~				++input_done_count;
//~
//~				if(input_done_count > 2) break;
//~			}
//~
//~			// Read in next chunk of input samples.
//~			for(uint32 j = 0; j < (window_size - overlap_size); ++j)
//~			{
//~				if(input == end)
//~				{
//~					input_done = true;
//~
//~					dl.read();
//~					dl.write(0.0);
//~				}
//~				else
//~				{
//~					dl.read(); // move the read pointer.
//~					dl.write(*input);
//~					++input;
//~				}
//~			}
//~		}
//~	//~
//~	//~    float64 ratio = static_cast<float64>(this->getLength())
//~	//~                  / static_cast<float64>(y.getLength());
//~	//~
//~	//~    uint32 r = static_cast<uint32>((ratio + 0.005) * 100.0);
//~	//~
//~	//~    ratio = static_cast<float64>(r) / 100.0;
//~	//~
//~	//~    cout << "ratio = " << ratio << endl;
//~
//~		cout << "this->getLength() = " << this->getLength() << endl
//~			 << "y.getLength()     = " << y.getLength() << endl;
//~
//~		*this = y;
//~	}

void
Buffer::
preallocate(uint32 n)
{
    data_.reserve(data_.size() + n);
}

void
Buffer::
plot(const string & title) const
{
    Plotter pylab;

    pylab.figure();
    pylab.plot(*this);
    pylab.title(title);
}

void
Buffer::
readWavefile(const char * filename)
{
	M_CHECK_PTR(filename);

    *this << filename;
}

void
find_fraction(
    float64 fraction,
    float64 gamma,
    uint32 & a,
    uint32 & b)
{
    float64 num = 1.0;
    float64 den = 1.0;

    float64 f = 1.0;

    float64 diff = f - fraction;

    while( fabs(diff) > gamma )
    {
        if(f > fraction)
        {
            den += 1.0;
        }
        else
        {
            num += 1.0;
        }

        f = num / den;
        diff = f - fraction;
    }

    a = static_cast<uint32>(num);
    b = static_cast<uint32>(den);
}

// Implementation based on the "Discrete-Time Signal Processing" book
// Figure 4.28 on page 177 (Second Edition).
//
Buffer
Buffer::
getResample(
    float64 factor,
    const uint32 N,
    float64 beta) const
{
    uint32 L = 0;
    uint32 M = 0;

    find_fraction(factor, 0.0001, L, M);

    return getResample(L,M,N,beta);
}

Buffer
Buffer::
getResample(
    const Buffer & factor,
    const uint32 N,
    float64 beta) const
{
    BufferWindowSearch search(*this, 1024);

    uint32 pos = 0;

    Buffer y;

    while(search.getSamplesLeft() > 0)
    {
        Buffer window = search.getNextWindow();

        uint32 Lw = window.getLength();

        Buffer ratio = factor.subbuffer(pos, Lw);

        float64 r = ratio.getMean();

        y << window.getResample(r,N,beta);

        pos += Lw;
    }

    return y;
}

Buffer
Buffer::
getResample(
    const uint32 L,
    const uint32 M,
    const uint32 N,
    float64 beta) const
{
    Buffer y;

    // If the Buffer is really long, break it up into window to reduce memory
    // usage.
    if(getLength() > 40000)
    {
        BufferWindowSearch search(*this, 2048);

        while(search.getSamplesLeft() > 0)
        {
            Buffer window = search.getNextWindow();

            y << window._get_resample(L,M,N,beta);
        }
    }
    else
    {
        y << _get_resample(L,M,N,beta);
    }

    return y;
}

Buffer
Buffer::
_get_resample(
    const uint32 L,
    const uint32 M,
    const uint32 N,
    float64 beta) const
{
    if(L == 1 && M == 1)
    {
        return *this;
    }

    M_ASSERT_VALUE(L, !=, 0);
    M_ASSERT_VALUE(M, !=, 0);
    M_ASSERT_VALUE(N, !=, 0);
    M_ASSERT_VALUE(beta, >=, 0.0);

    uint32 LMmax = (L > M) ? L : M;

    float64 fc = 1.0 / 2.0 / static_cast<float64>(LMmax);

    // ensure kernel length is a multiple of L
    uint32 Lh = 2 * N * LMmax;

    float64 sr = 1000.0; // arbitrary, but usefull if the filter is plotted.

    Buffer f(4);
    Buffer a(4);

    f << 0.0 << sr * fc  << sr * fc  << sr * 0.5;
    a << 1.0 << 1.0 << 0.0 << 0.0;

    FilterLeastSquaresFIR lpf(sr, Lh, f, a, beta);

    uint32 delay = (Lh-1)/2;

    // Create Polyphase filter bank if interpolating
    std::vector<FilterLeastSquaresFIR> bank;
    if(L > 1)
    {
        // Get the kernel.
        Buffer h = lpf.getKernel();

        Buffer pseudo_f(2);
        Buffer pseudo_a(2);

        pseudo_f << 0.0 << 0.5;
        pseudo_a << 0.0 << 0.0;

        for(uint32 i = 0; i < L; ++i)
        {
            FilterLeastSquaresFIR poly_f(1.0, L, pseudo_f, pseudo_a, beta);

            Buffer poly_h(Lh / L);

            // Pull out cooefficents for the polyphase filter
            for(int32 j = Lh - L; j >= 0 && j < static_cast<int32>(Lh); j -= L)
            {
                poly_h << h[j + i];
            }

            poly_f.setKernel(poly_h.getReverse());

            bank.push_back(poly_f);
        }
    }

    uint32 n_filters = static_cast<uint32>(bank.size());

    // Interpolation

    Buffer::const_iterator x     = data_.begin();
    Buffer::const_iterator x_end = data_.end();

    uint32 Lx = getLength();
    uint32 Ly = getLength() * L / M;

    Buffer y(Ly);

    if(L > 1)
    {
//~        cerr << "Interpolation by " << L << endl << flush;

        float64 scale = static_cast<float64>(L);

        // Loop over all the samples
        while(x != x_end)
        {
            // Polyphase filter the input, for all L-1 samples
            for(uint32 j = 0; j < L; ++j)
            {
                float64 sample = *x * scale;

                for(uint32 k = 0; k < n_filters; ++k)
                {
                    y << bank[k].filter(sample);
                }
                ++x;
                if(x == x_end) break;
            }

            // Add some samples to the end for filter delay compansation.
            if(x == x_end)
            {
                float64 x_n = data_[Lx - 1] * scale;

                for(uint32 j = 0; j < Lh/L; ++j)
                {
                    for(uint32 k = 0; k < n_filters; ++k)
                    {
                        y << bank[k].filter(x_n);
                    }
                }
                break;
            }
        }
    }

    // Only down sampling, still need to filter.
    else
    {
        for(uint32 i = 0; i < Lx; ++i)
        {
            y << lpf.filter(data_[i]);
        }

        // Add some samples to the end for filter delay compansation.
        float64 x_n = data_[Lx - 1];
        for(uint32 i = 0; i < Lh; ++i)
        {
            y << lpf.filter(x_n);
        }
    }

    // Compensate for filter delay.
    y = y.subbuffer(delay, Lx*L);

    // Decimation
    if(M > 1)
    {
//~        cerr << "Decimation by " << M << endl << flush;

        x     = y.begin();
        x_end = y.end();

        Buffer y2(Ly / M);

        uint32 i = 0;
        while(x != x_end)
        {
            if(i % M == 0)
            {
                y2 << *x;
            }

            ++x;
            ++i;
        }

        y = y2;
    }

    return y;
}

void
Buffer::
reverse()
{
    for(uint32 front = 0, back = getLength() - 1;
        front < back;
        ++front, --back)
    {
        float64 temp = data_[front];
        data_[front] = data_[back];
        data_[back] = temp;
    }
}

void
Buffer::
round()
{
    for(Buffer::iterator i = this->begin();
        i != this->end();
        ++i)
    {
        *i = std::floor(*i + 0.5);
    }
}

void
Buffer::
_set_at_index(int32 index, float64 d)
{
    int32 len = getLength();

    if(index >= 0 && index < len)
    {
        data_[index] = d;
    }
    else if(index < 0 && index >= -len)
    {
        data_[len + index] = d;
    }
    else
    {
        M_THROW(
            "IndexError: " << index << " is out of bounds (0 : " << len << ")");
    }
}

BufferSelection
Buffer::
select(const uint32 start_index, const uint32 stop_index)
{
    uint32 n_samples = getLength();

    BooleanVector bv;

    bv.reserve(n_samples);

    for(uint32 i = 0; i < n_samples; ++i)
    {
    	if( i >= start_index && i <= stop_index)
    	{
    		bv.push_back(true);
    	}
    	else
    	{
    		bv.push_back(false);
    	}
    }

    return BufferSelection(*this, bv);
}

std::ostream &
Buffer::
write(std::ostream & out) const
{
    out & 'b' & 'u' & 'f' & 'f' & getLength();

    for(uint32 i = 0; i < getLength(); ++i)
    {
        out & data_[i];
    }

    return out;
}

std::string
Buffer::
write() const
{
    std::stringstream ss;
    write(ss);
    return ss.str();
}

std::istream &
Buffer::
read(std::istream & in)
{
    char id[4];
    uint32 size = 0;

    in & id[0] & id[1] & id[2] & id[3] & size;

    if(id[0] != 'b' || id[1] != 'u' || id[2] != 'f' || id[3] != 'f')
    {
        M_THROW("Did not find any Nsound Buffer data in input stream!");
    }

    data_.clear();
    data_.reserve(size);

    for(uint32 i = 0; i < size; ++i)
    {
        float64 d;
        in & d;
        data_.push_back(d);
    }

    return in;
}

void
Buffer::
read(const void * data, std::size_t size)
{
    std::string in(reinterpret_cast<const char *>(data), size);
    std::stringstream ss(in);

    read(ss);
}

void
Buffer::
smooth(uint32 n_passes, uint32 n_samples_to_average)
{
    if(n_samples_to_average < 2)
    {
        return;
    }

    FilterMovingAverage maf(n_samples_to_average);

    Buffer::iterator end = this->end();

    for(uint32 i = 0; i < n_passes; ++i)
    {
        maf.reset();

        Buffer::iterator itor = this->begin();

        while(itor != end)
        {
            *itor = maf.filter(*itor);
            ++itor;
        }
    }
}

void
Buffer::
speedUp(float64 step_size)
{
    Buffer new_buffer;

    float64 n_samples = static_cast<float64>(data_.size());

    for(float64 i = 0.0; i < n_samples; i += step_size)
    {
        new_buffer << data_[static_cast<uint32>(i)];
    }

    *this = new_buffer;
}

void
Buffer::
speedUp(const Buffer & step_buffer)
{
    Buffer new_buffer;

    float64 n_samples = static_cast<float64>(getLength());
    uint32 step_buffer_size = step_buffer.getLength();

    uint32 step_buffer_index = 0;
    float64 step_size = 0.0;

    for(float64 i = 0.0; i < n_samples; i += step_size)
    {
        new_buffer << data_[static_cast<size_t>(i)];

        step_size = step_buffer[step_buffer_index++];

        if(step_buffer_index >= step_buffer_size)
        {
            step_buffer_index -= step_buffer_size;
        }
    }

    *this = new_buffer;
}

void
Buffer::
sqrt()
{
    Buffer::iterator itor = this->begin();
    Buffer::iterator end  = this->end();

    while(itor != end)
    {
        float64 v = *itor;
        if(v > 0.0)
        {
            *itor = std::sqrt(v);
        }
        else if(v < 0.0)
        {
            *itor = -std::sqrt(-v);
        }
        ++itor;
    }
}

Buffer
Buffer::
subbuffer(uint32 start_index, uint32 n_samples) const
{
    uint32 n = n_samples;

    if(n > getLength()) n = getLength();

    Buffer new_buffer(n);

    if(start_index >= getLength())
        return new_buffer;

    uint32 stop_index = start_index + n;

    if(n == 0 || stop_index >= getLength())
    {
        stop_index = getLength();
    }

    for(uint32 i = start_index; i < stop_index; ++i)
    {
        new_buffer << data_[i];
    }

    return new_buffer;
}

void
Buffer::
upSample(uint32 n)
{
    *this = getUpSample(n);
}

// This is based on the upsampling description in "Descrete-Time Signal
// Processing" 2nd Eddition aka "the Oppenheim book".  Section 4.6.2 page 172.
//
Buffer
Buffer::
getUpSample(uint32 L) const
{
    return getResample(L,static_cast<uint32>(1));
}

void
Buffer::
writeWavefile(const char * filename) const
{
    M_CHECK_PTR(filename);

    operator>>(*this, filename);
}

BooleanVector
Buffer::
operator>(float64 rhs)
{
    BooleanVector bv;

    Buffer::const_iterator itor = this->begin();
    Buffer::const_iterator end  = this->end();

    while(itor != end)
    {
        if(*itor > rhs) bv.push_back(true);
        else            bv.push_back(false);
        ++itor;
    }

    return bv;
}

BooleanVector
Buffer::
operator>=(float64 rhs)
{
    BooleanVector bv;

    Buffer::const_iterator itor = this->begin();
    Buffer::const_iterator end  = this->end();

    while(itor != end)
    {
        if(*itor >= rhs) bv.push_back(true);
        else             bv.push_back(false);
        ++itor;
    }

    return bv;
}

BooleanVector
Buffer::
operator<(float64 rhs)
{
    BooleanVector bv;

    Buffer::const_iterator itor = this->begin();
    Buffer::const_iterator end  = this->end();

    while(itor != end)
    {
        if(*itor < rhs) bv.push_back(true);
        else            bv.push_back(false);
        ++itor;
    }

    return bv;
}

BooleanVector
Buffer::
operator<=(float64 rhs)
{
    BooleanVector bv;

    Buffer::const_iterator itor = this->begin();
    Buffer::const_iterator end  = this->end();

    while(itor != end)
    {
        if(*itor <= rhs) bv.push_back(true);
        else             bv.push_back(false);
        ++itor;
    }

    return bv;
}

BooleanVector
Buffer::
operator==(float64 rhs)
{
    BooleanVector bv;

    Buffer::const_iterator itor = this->begin();
    Buffer::const_iterator end  = this->end();

    while(itor != end)
    {
        bv.push_back(*itor == rhs);
        ++itor;
    }

    return bv;
}

BooleanVector
Buffer::
operator!=(float64 rhs)
{
    BooleanVector bv;

    Buffer::const_iterator itor = this->begin();
    Buffer::const_iterator end  = this->end();

    while(itor != end)
    {
        bv.push_back(*itor != rhs);
        ++itor;
    }

    return bv;
}

Buffer
Buffer::
ones(const uint32 n_samples)
{
    Buffer b(n_samples);

    Generator g(1);

    b << g.drawLine(n_samples, 1.0, 1.0);

    return b;
}

Buffer
Buffer::
rand(const uint32 n_samples)
{
    Buffer b(n_samples);

    Generator g(1);

    b << g.whiteNoise(static_cast<std::size_t>(n_samples));

    return b;
}

Buffer
Buffer::
zeros(const uint32 n_samples)
{
    Buffer b(n_samples);

    Generator g(1);

    b << g.drawLine(n_samples, 0.0, 0.0);

    return b;
}

} // namespace
