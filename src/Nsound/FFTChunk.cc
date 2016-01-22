//-----------------------------------------------------------------------------
//
//  $Id: FFTChunk.cc 874 2014-09-08 02:21:29Z weegreenblobbie $
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

#include <Nsound/Buffer.h>
#include <Nsound/FFTChunk.h>
#include <Nsound/Generator.h>
#include <Nsound/Plotter.h>

#include <iostream>
#include <sstream>

using namespace Nsound;

using std::cout;
using std::cerr;
using std::endl;
using std::flush;

//-----------------------------------------------------------------------------
FFTChunk::
FFTChunk(uint32 size, uint32 sample_rate, uint32 original_size)
    :
    real_(NULL),
    imag_(NULL),
    sample_rate_(sample_rate),
    original_size_(original_size),
    is_polar_(false)
{
    real_ = new Buffer(size);
    imag_  = new Buffer(size);

    if(original_size_ == 0)
    {
        original_size_ = size;
    }
}

//-----------------------------------------------------------------------------
FFTChunk::
FFTChunk(const FFTChunk & copy)
    :
    real_(new Buffer(copy.real_->getLength())),
    imag_(new Buffer(copy.real_->getLength())),
    sample_rate_(copy.sample_rate_),
    original_size_(copy.original_size_),
    is_polar_(copy.is_polar_)
{
    // Call operator =
    *this = copy;
}

//-----------------------------------------------------------------------------
FFTChunk::
~FFTChunk()
{
    delete real_;
    delete imag_;
}

Buffer
FFTChunk::
getImaginary() const
{
    if(!is_polar_)
    {
        return imag_->subbuffer(0, imag_->getLength() / 2);
    }

    Buffer out;

    uint32 n_samples = real_->getLength();

    for(uint32 i = 0; i < n_samples / 2; ++i)
    {
        //      mag * sin(phase)
        out << (*real_)[i] * std::sin((*imag_)[i]);
    }

    return out;
}

Buffer
FFTChunk::
getFrequencyAxis() const
{
    // Create the x axis based on Hz.

    uint32 chunk_size = real_->getLength();

    uint32 n_samples = chunk_size / 2 + 1;

    float64 x_step = static_cast<float64>(sample_rate_)
                   / static_cast<float64>(chunk_size);

    Generator gen(1);

    return gen.drawLine(n_samples, 0.0, x_step * n_samples);
}

Buffer
FFTChunk::
getReal() const
{
    if(!is_polar_)
    {
        return real_->subbuffer(0, real_->getLength() / 2);
    }

    uint32 n_samples = real_->getLength();

    Buffer out(n_samples / 2);

    for(uint32 i = 0; i < n_samples / 2; ++i)
    {
        //      mag * cos(phase)
        out << (*real_)[i] * std::cos((*imag_)[i]);
    }

    return out;
}

Buffer
FFTChunk::
getMagnitude() const
{
    uint32 n_samples = real_->getLength();

    if(is_polar_)
    {
        return real_->subbuffer(0, n_samples / 2 + 1);
    }

    float64 x = 0.0;
    float64 y = 0.0;

    Buffer out(n_samples / 2 + 1);

    for(uint32 i = 0; i < n_samples / 2 + 1; ++i)
    {
        x = (*real_)[i];
        y = (*imag_)[i];
        out << std::sqrt(x*x + y*y);
    }

    return out;
}

Buffer
FFTChunk::
getPhase() const
{
    if(is_polar_)
    {
        return imag_->subbuffer(0, imag_->getLength() / 2 + 1);
    }

    uint32 n_samples = real_->getLength();

    float64 r = 0.0;
    float64 i = 0.0;
    float64 phase = 0.0;

    Buffer out(n_samples / 2 + 1);

    for(uint32 j = 0; j < n_samples / 2 + 1; ++j)
    {
        r = (*real_)[j];
        i = (*imag_)[j];

        //if(r == 0.0) r = 1e-20;

        phase = std::atan(i/r);

        if(r < 0.0)
        {
            if(i <  0.0) phase -= M_PI;
            else phase += M_PI;
        }

        out << phase;
    }

    return out;
}

//-----------------------------------------------------------------------------
FFTChunk &
FFTChunk::
operator=(const FFTChunk & rhs)
{
    if(this == &rhs)
    {
        return *this;
    }

    *real_ = *rhs.real_;
    *imag_  = *rhs.imag_;
    sample_rate_ = rhs.sample_rate_;
    original_size_ = rhs.original_size_;
    is_polar_ = rhs.is_polar_;

    return *this;
}

void
FFTChunk::
plot(
    const std::string & title,
    boolean dB,
    boolean show_phase) const
{
    //~    // Create the x axis based on Hz.
    //~
    //~    uint32 chunk_size = real_->getLength();

    uint32 n_samples = real_->getLength() / 2 + 1;

    //~
    //~    float64 x_step = (1.0 / (static_cast<float64>(chunk_size) / 2.0))
    //~                   * (static_cast<float64>(sample_rate_) / 2.0);
    //~
    //~    Buffer x;
    //~
    //~    float64 tx = 0.0;
    //~
    //~    for(uint32 i = 0; i < n_samples; ++i)
    //~    {
    //~        x << tx;
    //~        tx += x_step;
    //~    }

    Buffer x = getFrequencyAxis();

    // Calculate the magnitude for the signal:
    //
    // magnitude = sqrt(x*x + y*y);
    //
    Buffer magnitude = getMagnitude();

    Plotter pylab;

    pylab.figure();

    if(show_phase)
    {
        pylab.subplot(2, 1, 1);
    }

    if(title.length() <= 0)
    {
        pylab.title("Magnitude");
    }
    else
    {
        pylab.title(title);
    }

    if(dB)
    {
        magnitude.dB();
        pylab.ylabel("Magnitude dB");
    }
    else
    {
        pylab.ylabel("Magnitude");
    }

    pylab.plot(x,magnitude.subbuffer(0,n_samples));

    if(show_phase)
    {
        Buffer phase = getPhase().subbuffer(0,n_samples);

        pylab.subplot(2, 1, 2);
        pylab.title("Phase");
        pylab.plot(x,phase);
        pylab.ylabel("Phase Radians");
    }

    pylab.xlabel("Frequency Hz");
}

void
FFTChunk::
setCartesian(const Buffer & real, const Buffer & img)
{
    Buffer zeros = 0.0 * real;

    *real_ = 2.0 * real;
    *real_ << zeros;

    *imag_ = 2.0 * img;
    *imag_ << zeros;

    is_polar_ = false;
}

void
FFTChunk::
setPolar(const Buffer & mag, const Buffer & phase)
{
    Buffer zeros = 0.0 * mag;

    *real_ = 2.0 * mag;
    *real_ << zeros;

    *imag_ = 2.0 * phase;
    *imag_ << zeros;

    is_polar_ = true;
}

void
FFTChunk::
toCartesian()
{
    if(!is_polar_) return;

    uint32 n_samples = real_->getLength();

    float64 m = 0.0;
    float64 p = 0.0;

    for(uint32 j = 0; j < n_samples; ++j)
    {
        m = (*real_)[j];
        p = (*imag_)[j];
        (*real_)[j] = m * std::cos(p);
        (*imag_)[j]  = m * std::sin(p);
    }

    is_polar_ = false;
}

void
FFTChunk::
toPolar()
{
    if(is_polar_) return;

    uint32 n_samples = real_->getLength();

    float64 r = 0.0;
    float64 i = 0.0;

    for(uint32 j = 0; j < n_samples; ++j)
    {
        r = (*real_)[j];
        i = (*imag_)[j];

        (*real_)[j] = std::sqrt(r*r + i*i);
        (*imag_)[j]  = std::atan(i / r);

        if(i <  0.0 && r < 0.0) (*imag_)[j] -= M_PI;
        if(i >= 0.0 && r < 0.0) (*imag_)[j] += M_PI;
    }

    is_polar_ = true;
}

//~///////////////////////////////////////////////////////////////////////////////
//~FFTChunk
//~FFTChunkVector::
//~getMean() const
//~{
//~    FFTChunk chunk;

//~    uint32 n_chunks = this->size();

//~    if(n_chunks == 0) return chunk;

//~    chunk = (*this)[0];

//~    for(uint32 i = 1; i < n_chunks; ++i)
//~    {
//~        *chunk.real_ += *(*this)[i].real_;
//~        *chunk.imag_  += *(*this)[i].imag_;
//~    }

//~    float64 n = static_cast<float64>(n_chunks);

//~    *chunk.real_ /= n;
//~    *chunk.imag_ /= n;

//~    return chunk;
//~}

//~///////////////////////////////////////////////////////////////////////////////
//~FFTChunk
//~FFTChunkVector::
//~getMax() const
//~{
//~    FFTChunk chunk;

//~    uint32 n_chunks = this->size();

//~    if(n_chunks == 0) return chunk;

//~    chunk = (*this)[0];

//~    for(uint32 i = 1; i < n_chunks; ++i)
//~    {
//~        Buffer mag1 = chunk.getMagnitude();
//~        Buffer mag2 = (*this)[i].getMagnitude();

//~        for(uint32 j = 0; j < mag1.getLength(); ++j)
//~        {
//~            if(mag2[j] > mag1[j])
//~            {
//~                (*chunk.real_)[j] = (*(*this)[i].real_)[j];
//~                (*chunk.imag_)[j] = (*(*this)[i].imag_)[j];
//~            }
//~        }
//~    }

//~    return chunk;
//~}

//~///////////////////////////////////////////////////////////////////////////////
//~FFTChunk
//~FFTChunkVector::
//~getMin() const
//~{
//~    FFTChunk chunk;

//~    uint32 n_chunks = this->size();

//~    if(n_chunks == 0) return chunk;

//~    chunk = (*this)[0];

//~    for(uint32 i = 1; i < n_chunks; ++i)
//~    {
//~        Buffer mag1 = chunk.getMagnitude();
//~        Buffer mag2 = (*this)[i].getMagnitude();

//~        for(uint32 j = 0; j < mag1.getLength(); ++j)
//~        {
//~            if(mag2[j] < mag1[j])
//~            {
//~                (*chunk.real_)[j] = (*(*this)[i].real_)[j];
//~                (*chunk.imag_)[j] = (*(*this)[i].imag_)[j];
//~            }
//~        }
//~    }

//~    return chunk;
//~}

