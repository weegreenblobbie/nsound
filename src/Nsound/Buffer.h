//-----------------------------------------------------------------------------
//
//  $Id: Buffer.h 912 2015-07-26 00:50:29Z weegreenblobbie $
//
//  Nsound is a C++ library and Python module for audio synthesis featuring
//  dynamic digital filters. Nsound lets you easily shape waveforms and write
//  to disk or plot them. Nsound aims to be as powerful as Csound but easy to
//  use.
//
//  Copyright (c) 2004-Present Nick Hilton
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
#ifndef _NSOUND_BUFFER_H_
#define _NSOUND_BUFFER_H_

#include <Nsound/Nsound.h>
#include <Nsound/BufferSelection.h>
#include <Nsound/CircularIterators.h>

#include <cmath>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

namespace Nsound
{

// Forward declaration
class AudioStream;

//-----------------------------------------------------------------------------
//! A Buffer for storing audio samples.
//
//! This is the workhourse of Nsound.  This class dynamically allocates
//! memory and frees the user of the class from worrying about buffer sizes.
//! This class does not have a sample rate, samples are indexed by an integer
//! index.
//!
class Buffer
{
    public:

    // Forward declaration

    typedef Nsound::circular_iterator       circular_iterator;
    typedef Nsound::const_circular_iterator const_circular_iterator;

    typedef FloatVector::iterator         iterator;
    typedef FloatVector::const_iterator   const_iterator;

    //! Creates an empty Buffer that also calss std::vector::reserve() to preallocate memory for samples.
    //
    //! \param chunk_size the number of samples to preallocate.
    //! \return void
    //!
    //! \par Example:
    //! \code
    //! // C++
    //! Buffer b(1024);
    //!
    //! // Python
    //! b = Buffer(1024)
    //! \endcode
    Buffer();
    explicit Buffer(uint32 chunk_size);

    //! Creates a Buffer and fills it with the contents of list.
    //
    //! \param list a FloatVector holding samples
    //! \return Buffer
    //!
    //! \par Example:
    //! \code
    //! // C++
    //! FloatVector fv;
    //! fv.push_back(1.0);
    //! fv.push_back(2.0);
    //! fv.push_back(3.0);
    //! fv.push_back(4.0);
    //! fv.push_back(5.0);
    //! Buffer b(fv);
    //!
    //! // Python
    //! b = Buffer([1,2,3,4,5])
    //! \endcode
    explicit Buffer(const FloatVector & list);

    //! Creates an Buffer from a Wavefile, also specifies the chunk size.
    //
    //! \param filename a string with a wave filename to read.
    //! \param chunk_size \i optional, sets the chunk size for the Buffer.
    //!
    //! \par Example:
    //! \code
    //! // C++
    //! Buffer b("california.wav");
    //!
    //! // Python
    //! b = Buffer("california.wav")
    //! \endcode
    Buffer(const std::string & filename, uint32 chunk_size = 4096);

    //! Destroys the Buffer.
    ~Buffer();

    //! Copy constructor, must be used in Python to copy Buffers.
    //
    //! \par Example:
    //! \code
    //! // C++
    //! Buffer b1("california.wav");
    //! Buffer b2(b1);
    //!
    //! // Python
    //! b1 = Buffer("california.wav")
    //! b2 = Buffer(b1)                 // Copies memory, Python does not have
    //!                                 // an operator=
    //! \endcode
    Buffer(const Buffer & rhs);

    #ifndef SWIG
    Buffer(Buffer && move);
    #endif

    //! Modifies the Buffer by making any negative value positive.
    //
    //! \par Example:
    //! \code
    //! // C++
    //! Buffer b1("california.wav");
    //! b1.abs();
    //!
    //! // Python
    //! b1 = Buffer("california.wav")
    //! b1.abs()
    //! \endcode
    void
    abs();

    //! Modifies a copy of the Buffer by making any negative value positive.
    //
    //! \par Example:
    //! \code
    //! // C++
    //! Buffer b1("california.wav");
    //! Buffer b2 = b1.getAbs();
    //!
    //! // Python
    //! b1 = Buffer("california.wav")
    //! b2 = b1.getAbs();
    //! \endcode
    Buffer
    getAbs() const
    { Buffer temp(*this); temp.abs(); return temp; }

    // add()
    //
    //! This method adds buffer to *this.
    //
    //! If the length of 'buffer' is longer than *this, *this will be
    //! extended.  If n_samples is 0, the whole length of buffer
    //! will get added, otherwise, only n_samples will get added.
    //!
    //! For example:
    //!
    //! <pre>
    //!
    //! a = sample in *this
    //! b = sample in buffer
    //! c = a + b
    //!
    //! this = [aaaaaaaaaaaaaaaaaaaaaaaaaaaa]
    //!      + |--offset--|[bbbbbbbbb buffer bbbbbbbbbbbbbbbbbb]
    //!
    //!      = [aaaaaaaaaaacccccccccccccccccbbbbbbbbbbbbbbbbbbb]
    //! </pre>
    //!
    //! Another exampple:
    //!
    //! <pre>
    //!
    //! a = sample in *this
    //! b = sample in buffer
    //! c = a + b
    //!
    //! this = [aaaaaaaaaaaaaaaaaaaaaaaaaaaa]
    //!      + |--offset--|[bbbbbbbbb buffer bbbbbbbbbbbbbbbbbb]
    //!                    |--------n_samples------|
    //!
    //!      = [aaaaaaaaaaaccccccccccccccccccbbbbbbb]
    //!
    //! </pre>
    //!
    //!
    //! \param offset - The index into this Buffer to start adding
    //!
    //! \param buffer - The Buffer to add data from
    //!
    //! \param n_samples - The number of samples from buffer to add
    //!
    //! \par Example:
    //! \code
    //! // C++
    //! Buffer b1("california.wav");
    //! Buffer b2("walle.wav");
    //! Buffer b1.add(0,b2);
    //!
    //! // Python
    //! b1 = Buffer("california.wav")
    //! b2 = Buffer("walle.wav")
    //! b1.add(0,b2);
    //! \endcode
    //
    void
    add(const Buffer & buffer,
        uint32 offset = 0,
        uint32 n_samples = 0);

    //! Retruns the index of the maximum value in the Buffer.
    //
    //! \par Example:
    //! \code
    //! // C++
    //! Buffer b1("california.wav");
    //! uint32 max_index = b1.argmax();
    //!
    //! // Python
    //! b1 = Buffer("california.wav")
    //! max_index = b1.argmax()
    //! \endcode
    uint32
    argmax() const;

    //! Retruns the index of the minimum value in the Buffer.
    //
    //! \par Example:
    //! \code
    //! // C++
    //! Buffer b1("california.wav");
    //! uint32 min_index = b1.argmin();
    //!
    //! // Python
    //! b1 = Buffer("california.wav")
    //! min_index = b1.argmin()
    //! \endcode
    uint32
    argmin() const;

    //! Retruns the itreator at the start of the Buffer.
    //
    //! \par Example:
    //! \code
    //! // C++
    //! Buffer b1("california.wav");
    //! Buffer::iterator itor = b1.begin();
    //!
    //! // Python, you can already iterate over the samples
    //! b1 = Buffer("california.wav");
    //! for sample in b1:
    //!     // do something
    //! \endcode
    inline
    iterator
    begin()
    { return data_.begin(); }

    //! Retruns the itreator at the start of the Buffer.
    //
    //! \par Example:
    //! \code
    //! // C++
    //! Buffer b1("california.wav");
    //! Buffer::cons_iterator itor = b1.begin();
    //!
    //! // Python, you can iterate over the samples
    //! b1 = Buffer("california.wav");
    //! for sample in b1:
    //!     // do something
    //! \endcode
    inline
    const_iterator
    begin() const
    { return data_.begin(); }

    //! Retruns the itreator at the start of the Buffer.
    //
    //! \par Example:
    //! \code
    //! // C++
    //! Buffer b1("california.wav");
    //! Buffer::circular_iterator ctor = b1.begin();
    //!
    //! // Python, currently not supported
    //! \endcode
    inline
    circular_iterator
    cbegin()
    { return circular_iterator(data_); }

    //! Retruns the itreator at the start of the Buffer.
    //
    //! \par Example:
    //! \code
    //! // C++
    //! const Buffer b1("california.wav");
    //! Buffer::circular_iterator ctor = b1.begin();
    //!
    //! // Python, currently not supported
    //! \endcode
    inline
    const_circular_iterator
    cbegin() const
    { return const_circular_iterator(data_); }

    FloatVector data() { return data_; }
    const FloatVector & data() const { return data_; }

    //! Retruns the itreator at the end of the Buffer.
    //
    //! \par Example:
    //! \code
    //! // C++
    //! Buffer b1("california.wav");
    //! Buffer::iterator end = b1.end();
    //!
    //! // Python, currently not supported
    //! \endcode
    inline
    iterator
    end()
    { return data_.end(); }

    //! Retruns the itreator at the end of the Buffer.
    //
    //! \par Example:
    //! \code
    //! // C++
    //! Buffer b1("california.wav");
    //! Buffer::iterator end = b1.end();
    //!
    //! // Python, currently not supported
    //! \endcode
    inline
    const_iterator
    end() const
    { return data_.end(); }

    //! Retruns the number of bytes held in the Buffer array.
    //
    //! \par Example:
    //! \code
    //! // C++
    //! Buffer b1("california.wav");
    //! uint32 n_bytes = b1.getNBytes();
    //!
    //! // Python
    //! b1 = Buffer("california.wav")
    //! n_bytes = b1.getNBytes()
    //! \endcode
    inline
    uint32
    getNBytes() const
    { return static_cast<uint32>(bytes_per_sample_ * data_.size()); }

    //! Each sample in the Buffer becomes the power e^x.
    //
    //! Each sample in the Buffer becomes the power e^x, where x is the Buffer
    //! sample.
    //!
    //! \par Example:
    //! \code
    //! // C++
    //! Buffer b1("california.wav");
    //! b1.exp();
    //!
    //! // Python
    //! b1 = Buffer("california.wav")
    //! b1.exp()
    //! \endcode
    void
    exp();

    //! Each sample in the Buffer becomes the power e^x.
    //
    //! Each sample in the Buffer becomes the power e^x, where x is the Buffer
    //! sample.
    //!
    //! \par Example:
    //! \code
    //! // C++
    //! Buffer b1("california.wav");
    //! Buffer b2 = b1.getExp();
    //!
    //! // Python
    //! b1 = Buffer("california.wav")
    //! b2 = b1.getExp()
    //! \endcode
    Buffer
    getExp() const
    { Buffer temp(*this); temp.exp(); return temp; }

    //! Convolves the Buffer with another Buffer.
    //
    //! \param H another Buffer to convole with.
    //!
    //! \par Example:
    //! \code
    //! // C++
    //! Buffer b1("california.wav");
    //! Buffer b2("walle.wav");
    //! b1.convolve(b2);
    //!
    //! // Python
    //! b1 = Buffer("california.wav")
    //! b2 = Buffer("walle.wav")
    //! b1.convolve(b2)
    //! \endcode
    void
    convolve(const Buffer & H);

    //! Convolves a copy of the Buffer with another Buffer.
    //
    //! \param H another Buffer to convole with.
    //!
    //! \par Example:
    //! \code
    //! // C++
    //! Buffer b1("california.wav");
    //! Buffer b2("walle.wav");
    //! Buffer b3 = b1.getConvolve(b2);
    //!
    //! // Python
    //! b1 = Buffer("california.wav")
    //! b2 = Buffer("walle.wav")
    //! b3 = b1.getConvolve(b2)
    //! \endcode
    Buffer
    getConvolve(const Buffer & H) const;

    //! Modifies the Buffer so each sample is converted to dB, 20 * log10(sample).
    //
    //!
    //! \par Example:
    //! \code
    //! // C++
    //! Buffer b1("california.wav");
    //! b1.dB();
    //!
    //! // Python
    //! b1 = Buffer("california.wav")
    //! b1.dB()
    //! \endcode
    void
    dB();

    //! Returns the Buffer in dB.
    //
    //! \par Example:
    //! \code
    //! // C++
    //! Buffer b1("california.wav");
    //! Buffer b2 = b1.getdB();
    //!
    //! // Python
    //! b1 = Buffer("california.wav")
    //! b2 = b1.getdB()
    //! \endcode
    Buffer
    getdB() const
    { Buffer temp(*this); temp.dB(); return temp; }

    //! Calculates the nth derivative of the Buffer.
    //
    //! \par Example:
    //! \code
    //! // C++
    //! Buffer b1("california.wav");
    //! b1.derivative(1);
    //!
    //! // Python
    //! b1 = Buffer("california.wav")
    //! b1.derivative(1)
    //! \endcode
    void
    derivative(uint32 n);

    //! Returns the nth derivative of the Buffer.
    //
    //! \par Example:
    //! \code
    //! // C++
    //! Buffer b1("california.wav");
    //! Buffer b2 = b1.getDerivative(1);
    //!
    //! // Python
    //! b1 = Buffer("california.wav")
    //! b2 = b1.getDerivative(1)
    //! \endcode
    Buffer
    getDerivative(uint32 n) const
    { Buffer temp(*this); temp.derivative(n); return temp; }

    //! Downsample this Buffer by a integral factor. N must be > 1.
    //
    //! \par Example:
    //! \code
    //! // C++
    //! Buffer b1("california.wav");
    //! b1.downSample(2);
    //!
    //! // Python
    //! b1 = Buffer("california.wav")
    //! b1.downSample(2)
    //! \endcode
    void
    downSample(uint32 n);

    //! Returns a copy of this Buffer downsampled by a integral factor. N must be > 1.
    //
    //! \par Example:
    //! \code
    //! // C++
    //! Buffer b1("california.wav");
    //! Buffer b2 = b1.getDownSample(2);
    //!
    //! // Python
    //! b1 = Buffer("california.wav")
    //! b2 = b1.getDownSample(2)
    //! \endcode
    Buffer
    getDownSample(uint32 n) const;

    //! Find the peaks in the Buffer and return a vector of indicies.
    //
    //! \par Example:
    //! \code
    //! // C++
    //! Buffer b1("california.wav");
    //! Uint32Vector v = b1.findPeaks(1000, 0.0);
    //!
    //! // Python
    //! b1 = Buffer("california.wav")
    //! peaks = b1.findPeaks(1000, 0.0)
    //! \endcode
    Uint32Vector
    findPeaks(
        uint32 window_size = 0,
        float64 min_height = 0.0) const;

//~    Experimental, doesn't work yet
//~//~    //! Returns the estimated fundamental frequency of the Buffer.
//~    Buffer
//~    findPitch(float64 sample_rate);

    //! Returns the number of samples in the Buffer.
    //
    //! \par Example:
    //! \code
    //! // C++
    //! Buffer b1("california.wav");
    //! uint32 length = b1.getLength();
    //!
    //! // Python
    //! b1 = Buffer("california.wav")
    //! length = b1.getLength()
    //! \endcode
    inline
    uint32
    getLength() const
    { return static_cast<uint32>(data_.size()); }

    //! Limits the Buffer to min and max.
    //
    //! \par Example:
    //! \code
    //! // C++
    //! Buffer b1("california.wav");
    //! b1.limit(-1.0, 1.0);
    //!
    //! // alternate expressions
    //!
    //! b1(b1 >  1.0) = 1.0
    //! b1(b1 < -1.0) = -1.0
    //!
    //! # Python
    //! b1 = Buffer("california.wav")
    //! b1.limit(-1.0, 1.0);
    //!
    //! # alternalte expressions
    //!
    //! b1(b1 >  1.0).set(1.0)
    //! b1(b1 < -1.0).set(-1.0)
    //!
    //! \endcode
    void
    limit(float64 min, float64 max);

    //! Limits the Buffer to min and max.
    //
    //! \par Example:
    //! \code
    //! // C++
    //! Sine gen(100.0);
    //! Buffer b1("california.wav");
    //! Buffer b2 = gen.generate(5.0, 2.0);  // 5 seconds at 2 Hz
    //! b1.limit(b2, b2 + 1.0);
    //!
    //! // Python
    //! gen = Sine(100.0)
    //! b1 = Buffer("california.wav")
    //! b2 = Buffer(100)                 // sets chunk size to 100
    //! b2 << gen.generate(5.0, 2.0)     // 5 seconds at 2 Hz
    //! b1.limit(b2, b2 + 1.0)
    //! \endcode
    void
    limit(const Buffer & min, const Buffer & max);

    //! Returns a copy of the Buffer with min and max limits.
    //
    //! \par Example:
    //! \code
    //! // C++
    //! Buffer b1("california.wav");
    //! Buffer b2 = b1.getLimit(-1.0, 1.0);
    //!
    //! // Python
    //! b1 = Buffer("california.wav")
    //! b2 = b1.getLimit(-1.0, 1.0);
    //! \endcode
    Buffer
    getLimit(float64 min, float64 max) const
    { Buffer temp(*this); temp.limit(min, max); return temp; }

    //! Returns a copy of the Buffer with min and max limits.
    //
    //! \par Example:
    //! \code
    //! // C++
    //! Sine gen(100.0);
    //! Buffer b1("california.wav");
    //! Buffer b2 = gen.generate(5.0, 2.0);  // 5 seconds at 2 Hz
    //! Buffer b3 = b1.getLimit(b2, b2 + 1.0);
    //!
    //! // Python
    //! gen = Sine(100.0)
    //! b1 = Buffer("california.wav")
    //! b2 = Buffer(100)                 // sets chunk size to 100
    //! b2 << gen.generate(5.0, 2.0)     // 5 seconds at 2 Hz
    //! b3 = b1.getLimit(b2, b2 + 1.0)
    //! \endcode
    Buffer
    getLimit(const Buffer & min, const Buffer & max) const
    { Buffer temp(*this); temp.limit(min, max); return temp; }

    //! Sets each sample in the Buffer to the natural log.
    //
    //! \par Example:
    //! \code
    //! // C++
    //! Buffer b1("california.wav");
    //! b1.log();
    //!
    //! // Python
    //! b1 = Buffer("california.wav")
    //! b1.log()
    //! \endcode
    void
    log();

    //! Returns the the Buffer where each sample is the natural log.
    //
    //! \par Example:
    //! \code
    //! // C++
    //! Buffer b1("california.wav");
    //! Buffer b1_log = b1.getLog();
    //!
    //! // Python
    //! b1 = Buffer("california.wav")
    //! l = b1.getLog()
    //! \endcode
    Buffer
    getLog() const
    { Buffer temp(*this); temp.log(); return temp; }

    //! Sets each sample in the Buffer to log base 10.
    //
    //! \par Example:
    //! \code
    //! // C++
    //! Buffer b1("california.wav");
    //! b1.log10();
    //!
    //! // Python
    //! b1 = Buffer("california.wav")
    //! b1.log10()
    //! \endcode
    void
    log10();

    //! Returns the the Buffer where each sample is the natural log.
    //
    //! \par Example:
    //! \code
    //! // C++
    //! Buffer b1("california.wav");
    //! Buffer b1_log = b1.getLog();
    //!
    //! // Python
    //! b1 = Buffer("california.wav")
    //! l = b1.getLog()
    //! \endcode
    Buffer
    getLog10() const
    { Buffer temp(*this); temp.log10(); return temp; }

    //! Returns the maximum sample value in the Buffer.
    //
    //! \par Example:
    //! \code
    //! // C++
    //! Buffer b1("california.wav");
    //! float64 max = b1.getMax();
    //!
    //! // Python
    //! b1 = Buffer("california.wav")
    //! m = b1.getMax()
    //! \endcode
    float64
    getMax() const;

    //! Returns the maximum magnitude value in the Buffer, i.e. max(abs(samples)).
    //
    //! \par Example:
    //! \code
    //! // C++
    //! Buffer b1("california.wav");
    //! float64 max = b1.getMaxMagnitude();
    //!
    //! // Python
    //! b1 = Buffer("california.wav")
    //! m = b1.getMaxMagnitude()
    //! \endcode
    float64
    getMaxMagnitude() const;

    //! Returns the mean sample value in the Buffer.
    //
    //! \par Example:
    //! \code
    //! // C++
    //! Buffer b1("california.wav");
    //! float64 mean = b1.getMean();
    //!
    //! // Python
    //! b1 = Buffer("california.wav")
    //! m = b1.getMean()
    //! \endcode
    float64
    getMean() const;

    //! Returns the minimum sample value in the Buffer.
    //
    //! \par Example:
    //! \code
    //! // C++
    //! Buffer b1("california.wav");
    //! float64 min = b1.getMin();
    //!
    //! // Python
    //! b1 = Buffer("california.wav")
    //! m = b1.getMin()
    //! \endcode
    float64
    getMin() const;


    // mul()
    //
    //! This method multiplies a Buffer to *this over a sub range.
    //
    //! If the length of 'buffer' is longer than *this, *this will be
    //! extended.  If n_samples is 0, the whole length of buffer
    //! will get added, otherwise, only n_samples will get added.
    //!
    //! Examples:
    //!
    //! <pre>
    //!
    //! a = sample in *this
    //! b = sample in buffer
    //! c = a * b
    //!
    //! this = [aaaaaaaaaaaaaaaaaaaaaaaaaaaa]
    //!      * |  offset  |[bbbbbbbbb buffer bbbbbbbbbbbbbbbbbb]
    //!
    //!      = [aaaaaaaaaaaccccccccccccccccc]
    //! </pre>
    //!
    //! <pre>
    //!
    //! a = sample in *this
    //! b = sample in buffer
    //! c = a * b
    //!
    //! this = [aaaaaaaaaaaaaaaaaaaaaaaaaaaa]
    //!      + |  offset  |[bbbbbbbbb buffer bbbbbbbbbbbbbbbbbb]
    //!                    |-n_samples---|
    //!
    //!      = [aaaaaaaaaaacccccccccccccccaa]
    //!
    //! </pre>
    //!
    //!
    //! \param offset - The index into this Buffer to start multiplying
    //!
    //! \param buffer - The Buffer to multiply data from
    //!
    //! \param n_samples - The number of samples from buffer to multiply
    //!
    //! \par Example:
    //! \code
    //! // C++
    //! Buffer b1("california.wav");
    //! Buffer b2("walle.wav");
    //! Buffer b1.mul(0, b2);
    //!
    //! // Python
    //! b1 = Buffer("california.wav")
    //! b2 = Buffer("walle.wav")
    //! b1.mul(0, b2);
    //! \endcode
    //
    void
    mul(const Buffer & buffer,
        uint32 offset = 0,
        uint32 n_samples = 0);

    //! Multiplies the Buffer by a constant gain so the peak sample has magnitude 1.0.
    //
    //! \par Example:
    //! \code
    //! // C++
    //! Buffer b1("california.wav");
    //! b1.normalize();
    //!
    //! // Python
    //! b1 = Buffer("california.wav")
    //! b1.normalize()
    //! \endcode
    void
    normalize();

    //! Returns a copy of the Buffer normalized.
    //
    //! \par Example:
    //! \code
    //! // C++
    //! Buffer b1("california.wav");
    //! Buffer b2 = b1.getNormalize();
    //!
    //! // Python
    //! b1 = Buffer("california.wav")
    //! b2 = b1.getNormalize()
    //! \endcode
    Buffer
    getNormalize() const
    { Buffer temp(*this); temp.normalize(); return temp; }

    //! Returns the signal energy: E = 1 / N * sum(|x(i)|) over the window of N samples
    //
    //! \par Example:
    //! \code
    //! // C++
    //! Buffer b1("california.wav");
    //! float64 energy = b1.getSignalEnergy();
    //!
    //! // Python
    //! b1 = Buffer("california.wav")
    //! energy = b1.getSignalEnergy()
    //! \endcode
    Buffer
    getSignalEnergy(uint32 window_size) const;

    //! Returns the standard deviation of the Buffer.
    //
    //! \par Example:
    //! \code
    //! // C++
    //! Buffer b1("california.wav");
    //! float64 std = b1.getStd();
    //!
    //! // Python
    //! b1 = Buffer("california.wav")
    //! std = b1.getStd()
    //! \endcode
    float64
    getStd() const;

    //! Returns the sum of all samples.
    //
    //! \par Example:
    //! \code
    //! // C++
    //! Buffer b1("california.wav");
    //! float64 sum = b1.getSum();
    //!
    //! // Python
    //! b1 = Buffer("california.wav")
    //! sum = b1.getSum()
    //! \endcode
    float64
    getSum() const;

    //! Normalized the Buffer using Z score normalizing.
    //
    //! z = (x - mean(x)) / std(x)
    //!
    //! \par Example:
    //! \code
    //! // C++
    //! Buffer b1("california.wav");
    //! b1.zNorm();
    //!
    //! // Python
    //! b1 = Buffer("california.wav")
    //! b1.zNorm()
    //! \endcode
    void
    zNorm();

    //! Returns a Z score normalized copy of the Buffer.
    //
    //! z = (x - mean(x)) / std(x)
    //!
    //! \par Example:
    //! \code
    //! // C++
    //! Buffer b1("california.wav");
    //! Buffer b2 = b1.getNorm();
    //!
    //! // Python
    //! b1 = Buffer("california.wav")
    //! b2 = b1.getZNorm()
    //! \endcode
    Buffer
    getZNorm() const
    { Buffer temp(*this); temp.zNorm(); return temp; }

    //! Returns a BufferSelection object used for manipulation of a selected region of samples.
    //
    //! \par Example:
    //! \code
    //! // C++
    //! Buffer b1("california.wav");
    //! BufferSelection bs = b1(b1 > 0.5);
    //!
    //! // Python
    //! b1 = Buffer("california.wav")
    //! bs = b1(b1 > 0.5)
    //! \endcode
    BufferSelection
    operator()(const BooleanVector & bv);

    //! The assignment operator.  C++ only, for Python, use the copy constructor.
    Buffer &
    operator=(const Buffer & rhs);

    //! Tests of equality.
    boolean
    operator==(const Buffer & rhs) const;

    //! Tests of inequality.
    boolean
    operator!=(const Buffer & rhs) const;

    //! Retruns a reference into the Buffer.
    inline
    float64 &
    operator[](uint32 index)
    { return data_[index]; }

    //! Returns a const reference into the Buffer.
    inline
    const float64 &
    operator[](uint32 index) const
    { return data_[index]; }

    //! Concatenates Buffers and AudioStreams together.
    //
    //! \par Example:
    //! \code
    //! // C++
    //! Buffer b1("california.wav");
    //! AudioStream a2("walle.wav");
    //! b1 << a2;
    //!
    //! // Python
    //! b1 = Buffer("california.wav")
    //! a2 = AudioStream("walle.wav")
    //! b1 << a2
    //! \endcode
    Buffer &
    operator<<(const AudioStream & rhs);

    //! Concatenates Buffers together.
    //
    //! \par Example:
    //! \code
    //! // C++
    //! Buffer b1("california.wav");
    //! Buffer b2("walle.wav");
    //! b1 << b2;
    //!
    //! // Python
    //! b1 = Buffer("california.wav")
    //! b2 = Buffer("walle.wav")
    //! b1 << b2
    //! \endcode
    Buffer &
    operator<<(const Buffer & rhs);

    //! Concatenates samples to the Buffer.
    inline
    Buffer &
    operator<<(float64 d)
    { data_.push_back(d); return *this; }

    //! Adds each sample from the right hand side (rhs) Buffer with this Buffer.
    //
    //! All arithmatic operations only operate for N samples where N is
    //! min(b1.getLength(), b2.getLength()).
    //!
    //! \par Example:
    //! \code
    //! // C++
    //! Buffer b1("california.wav");
    //! Buffer b2("walle.wav");
    //! b1 += b2;
    //!
    //! // Python
    //! b1 = Buffer("california.wav")
    //! b2 = Buffer("walle.wav")
    //! b1 += b2
    //! \endcode
    Buffer & operator+=(const Buffer & rhs);

    //! Subracts each sample from the right hand side (rhs) Buffer from this Buffer.
    //
    //! All arithmatic operations only operate for N samples where N is
    //! min(b1.getLength(), b2.getLength()).
    //!
    //! \par Example:
    //! \code
    //! // C++
    //! Buffer b1("california.wav");
    //! Buffer b2("walle.wav");
    //! b1 -= b2;
    //!
    //! // Python
    //! b1 = Buffer("california.wav")
    //! b2 = Buffer("walle.wav")
    //! b1 -= b2
    //! \endcode
    Buffer & operator-=(const Buffer & rhs);

    //! Multiplies each sample from the right hand side (rhs) with this Buffer.
    //
    //! All arithmatic operations only operate for N samples where N is
    //! min(b1.getLength(), b2.getLength()).
    //!
    //! \par Example:
    //! \code
    //! // C++
    //! Buffer b1("california.wav");
    //! Buffer b2("walle.wav");
    //! b1 *= b2;
    //!
    //! // Python
    //! b1 = Buffer("california.wav")
    //! b2 = Buffer("walle.wav")
    //! b1 *= b2
    //! \endcode
    Buffer & operator*=(const Buffer & rhs);

    //! Divides each sample in this Buffer with the right hand side (rhs) Buffer.
    //
    //! All arithmatic operations only operate for N samples where N is
    //! min(b1.getLength(), b2.getLength()).
    //!
    //! \par Example:
    //! \code
    //! // C++
    //! Buffer b1("california.wav");
    //! Buffer b2("walle.wav");
    //! b1 /= b2;
    //!
    //! // Python
    //! b1 = Buffer("california.wav")
    //! b2 = Buffer("walle.wav")
    //! b1 /= b2
    //! \endcode
    Buffer & operator/=(const Buffer & rhs);

    //! Each sample in the Buffer becomes the power x^n.
    //
    //! Each sample in the Buffer becomes the power x^n, where x is the Buffer
    //! sample and N is the power.
    //!
    //! \par Example:
    //! \code
    //! // C++
    //! Buffer b1("california.wav");
    //! Buffer b2("california.wav");
    //! b1 ^= b2;
    //!
    //! // Python
    //! b1 = Buffer("california.wav")
    //! b2 = Buffer("california.wav")
    //! b1 **= b2
    //! \endcode
    Buffer & operator^=(const Buffer & powers);

    //! Adds the sample d to each sample in this Buffer.
    //
    //! \par Example:
    //! \code
    //! // C++
    //! Buffer b1("california.wav");
    //! b1 += 0.26;
    //!
    //! // Python
    //! b1 = Buffer("california.wav")
    //! b1 += 0.26
    //! \endcode
    Buffer & operator+=(float64 d);

    //! Subtracts the sample d from each sample in this Buffer.
    //
    //! \par Example:
    //! \code
    //! // C++
    //! Buffer b1("california.wav");
    //! b1 += 0.26;
    //!
    //! // Python
    //! b1 = Buffer("california.wav")
    //! b1 += 0.26
    //! \endcode
    Buffer & operator-=(float64 d);

    //! Multiplies the sample d with each sample in this Buffer.
    //
    //! \par Example:
    //! \code
    //! // C++
    //! Buffer b1("california.wav");
    //! b1 *= 0.26;
    //!
    //! // Python
    //! b1 = Buffer("california.wav")
    //! b1 *= 0.26
    //! \endcode
    Buffer & operator*=(float64 d);

    //! Divides each sample in this Buffer by d.
    //
    //! \par Example:
    //! \code
    //! // C++
    //! Buffer b1("california.wav");
    //! b1 /= 0.26;
    //!
    //! // Python
    //! b1 = Buffer("california.wav")
    //! b1 /= 0.26
    //! \endcode
    Buffer & operator/=(float64 d);

    //! Each sample in the Buffer becomes the power x^n.
    //
    //! Each sample in the Buffer becomes the power x^n, where x is the Buffer
    //! sample and N is the power.
    //!
    //! \par Example:
    //! \code
    //! // C++
    //! Buffer b1("california.wav");
    //! b1 ^= 2.0;
    //!
    //! // Python
    //! b1 = Buffer("california.wav")
    //! b1 **= 2.0
    //! \endcode
    //!
    //! \par WARNING: sometimes one may want to do this:
    //! \code
    //! // C++
    //! mag = real ^ 2 + imag ^ 2;
    //! \endcode
    //! But C++'s default operator precedence will actually compile this:
    //! \code
    //! mag = (real ^ (2 + imag)) ^ 2;
    //! \endcode
    //! because operator+ has high presedence.  To avoid this, in C++ you must
    //! use parentheses.
    //! \code
    //! mag = (real ^ 2) + (imag ^ 2);
    //! \endcode
    Buffer & operator^=(float64 power);

    #ifndef SWIG
        //! Sends the contents of the Buffer to the output stream.
        friend
        std::ostream &
        operator<<(std::ostream & out, const Buffer & rhs);
    #endif

    //! Creates a BooleanVector where each value is true iff Buffer[n] > rhs.
    //
    //! \par Example:
    //! \code
    //! // C++
    //! Buffer b1("california.wav");
    //! BufferSelection bs = b1 > 0.26;
    //!
    //! // Python
    //! b1 = Buffer("california.wav")
    //! bs = b1 > 0.26
    //! \endcode
    BooleanVector operator>(float64 rhs);

    //! Creates a BooleanVector where each value is true iff Buffer[n] >= rhs.
    //
    //! \par Example:
    //! \code
    //! // C++
    //! Buffer b1("california.wav");
    //! BufferSelection bs = b1 >= 0.26;
    //!
    //! // Python
    //! b1 = Buffer("california.wav")
    //! bs = b1 >= 0.26
    //! \endcode
    BooleanVector operator>=(float64 rhs);

    //! Creates a BooleanVector where each value is true iff Buffer[n] < rhs.
    //
    //! \par Example:
    //! \code
    //! // C++
    //! Buffer b1("california.wav");
    //! BufferSelection bs = b1 < 0.26;
    //!
    //! // Python
    //! b1 = Buffer("california.wav")
    //! bs = b1 < 0.26
    //! \endcode
    BooleanVector operator<(float64 rhs);

    //! Creates a BooleanVector where each value is true iff Buffer[n] <= rhs.
    //
    //! \par Example:
    //! \code
    //! // C++
    //! Buffer b1("california.wav");
    //! BufferSelection bs = b1 <= 0.26;
    //!
    //! // Python
    //! b1 = Buffer("california.wav")
    //! bs = b1 <= 0.26
    //! \endcode
    BooleanVector operator<=(float64 rhs);

    //! Creates a BooleanVector where each value is true iff Buffer[n] == rhs.
    //
    //! \par Example:
    //! \code
    //! // C++
    //! Buffer b1("california.wav");
    //! BufferSelection bs = b1 == 0.26;
    //!
    //! // Python
    //! b1 = Buffer("california.wav")
    //! bs = b1 == 0.26
    //! \endcode
    BooleanVector operator==(float64 rhs);

    //! Creates a BooleanVector where each value is true iff Buffer[n] != rhs.
    //
    //! \par Example:
    //! \code
    //! // C++
    //! Buffer b1("california.wav");
    //! BufferSelection bs = b1 != 0.26;
    //!
    //! // Python
    //! b1 = Buffer("california.wav")
    //! bs = b1 != 0.26
    //! \endcode
    BooleanVector operator!=(float64 rhs);

	//~	//~    //! EXPERIMENTAL!  Transforms the Buffer to the frequency domain and shifts it by n_bins_to_shift and then transforms it back to the time domain.
	//~    //
	//~    //! \par Example:
	//~    //! \code
	//~    //! // C++
	//~    //! Buffer b1("california.wav");
	//~    //! b1.pitchShift(2048, 2048, 2);
	//~    //!
	//~    //! // Python
	//~    //! b1 = Buffer("california.wav")
	//~    //! b1.pitchShift(2048, 2048, 2)
	//~    //! \endcode
	//~    void
	//~    pitchShift(uint32 window_size, uint32 fft_size, int32 n_bins_to_shift );
	//~
	//~	//~    //! EXPERIMENTAL!  Transforms a copy of the Buffer to the frequency domain and shifts it by n_bins_to_shift and then transforms it back to the time domain.
	//~    //
	//~    //! \par Example:
	//~    //! \code
	//~    //! // C++
	//~    //! Buffer b1("california.wav");
	//~    //! Buffer b2 = b1.getPitchShift(2048, 2048, 2);
	//~    //!
	//~    //! // Python
	//~    //! b1 = Buffer("california.wav")
	//~    //! b2 = b1.getPitchShift(2048, 2048, 2)
	//~    //! \endcode
	//~    Buffer
	//~    getPitchShift(uint32 window_size, uint32 fft_size, int32 n_bins_to_shift) const
	//~    { Buffer temp(*this);
	//~        temp.pitchShift(window_size, fft_size, n_bins_to_shift); return temp;}

    //! Requires matplotlib.  Creates a plot of this Buffer.
    //
    //! \par Example:
    //! \code
    //! // C++
    //! Buffer b1("california.wav");
    //! b1.plot("This is california.wav");
    //! Plotter::show(); // This will render any plots created.
    //!
    //! // Python
    //! b1 = Buffer("california.wav")
    //! b1.plot("This is california.wav")
    //! Plotter.show()
    //! \endcode
    void
    plot(const std::string & title = "Buffer") const;

    //! Returns the raw point to the first sample in the Buffer.
    //
    //! \par Example:
    //! \code
    //! // C++
    //! Buffer b1("california.wav");
    //! const flaot64 * ptr = b1.getPointer();
    //!
    //! // Python, not supported or needed.
    //! \endcode
    const
    float64 *
    getPointer() const
    { return data_.data(); }

    //! Requires matplotlib.  Creates a plot of this Buffer.
    //
    //! \par Example:
    //! \code
    //! // C++
    //! Buffer b1("california.wav");
    //! b1.plot("This is california.wav");
    //! Plotter::show(); // This will render any plots created.
    //!
    //! // Python, not supported or needed.
    //! \endcode
    float64 *
    getPointer()
    { return data_.data(); }

    //! Preallocates memory to hold an additional n samples.
    //
    //! This makes a call to std::vector::reserve( this->getLength() + n).
    //!
    //! \par Example:
    //! \code
    //! // C++
    //! Buffer b1("california.wav");
    //! b1.preallocate(100);
    //!
    //! // Python
    //! b1 = Buffer("california.wav")
    //! b1.preallocate(100);
    //! \endcode
    void
    preallocate(uint32 n);

    //! Sets this Buffer to the contents of the wavefile on the disk.
    //
    //! If the wavefile is stereo, it only reads the first channel.
    //! \par Example:
    //! \code
    //! // C++
    //! Buffer b1;
    //! b1.readWavefile("california.wav");
    //!
    //! // Python
    //! b1 = Buffer()
    //! b1.readWavefile("california.wav")
    //! \endcode
    void
    readWavefile(const char * filename);

    //! Resamples this Buffer using discrete-time resampling.
    //
    //! \par Example:
    //! \code
    //! // C++
    //! Buffer b1("california.wav");
    //! b1.resample(0.666);
    //!
    //! // Python
    //! b1 = Buffer("california.wav")
    //! b1.resample(0.666)
    //! \endcode
    void
    resample(float64 factor)
    { *this = getResample(factor, 10, 5.0); }

    //! Resamples this Buffer using discrete-time resampling.
    //
    //! \par Example:
    //! \code
    //! // C++
    //! Buffer b1("california.wav");
    //! Sine gen(44100);
    //! Buffer factor = gen.generate(2.0, 5.0);
    //! b1.resample(factor);
    //!
    //! // Python
    //! b1 = Buffer("california.wav")
    //! gen = Sine(44100)
    //! factor = Buffer()
    //! factor << gen.generate(2.0, 5.0)
    //! b1.resample(factor)
    //! \endcode
    void
    resample(const Buffer & factor)
    { *this = getResample(factor, 10, 5.0); }

    //! Resamples a copy of this Buffer using discrete-time resampling.
    //
    //! \par Example:
    //! \code
    //! // C++
    //! Buffer b1("california.wav");
    //! Buffer b2 = b1.getResample(0.666);
    //!
    //! // Python
    //! b1 = Buffer("california.wav")
    //! b2 = b1.getResample(0.666)
    //! \endcode
    Buffer
    getResample(
        float64 factor,
        const uint32 N = 10,
        float64 beta = 5.0) const;

    //! Resamples a copy of this Buffer using discrete-time resampling.
    //
    //! \par Example:
    //! \code
    //! // C++
    //! Buffer b1("california.wav");
    //! Sine gen(44100);
    //! Buffer factor = gen.generate(2.0, 5.0);
    //! Buffer b2 = b1.getResample(factor);
    //!
    //! // Python
    //! b1 = Buffer("california.wav")
    //! gen = Sine(44100)
    //! factor = Buffer()
    //! factor << gen.generate(2.0, 5.0)
    //! b2 = b1.getResample(factor)
    //! \endcode
    Buffer
    getResample(
        const Buffer & factor,
        const uint32 N = 10,
        float64 beta = 5.0) const;

    //! Resamples a copy of this Buffer using discrete-time resampling.
    //
    //! \par Example:
    //! \code
    //! // C++
    //! Buffer b1("california.wav");
    //! Buffer b2 = b1.getResample(1, 3);
    //!
    //! // Python
    //! b1 = Buffer("california.wav")
    //! b2 = b1.getResample(1, 3)
    //! \endcode
    Buffer
    getResample(
        const uint32 L,
        const uint32 M,
        const uint32 N = 10,
        float64 beta = 5.0) const;

    //! Reverses the samples in this Buffer.
    //
    //! \par Example:
    //! \code
    //! // C++
    //! Buffer b1("california.wav");
    //! b1.reverse();
    //!
    //! // Python
    //! b1 = Buffer("california.wav")
    //! b1.reverse()
    //! \endcode
    void
    reverse();

    //! Reverses the samples in a copy of this Buffer.
    //
    //! \par Example:
    //! \code
    //! // C++
    //! Buffer b1("california.wav");
    //! Buffer b2 = b1.getReverse();
    //!
    //! // Python
    //! b1 = Buffer("california.wav")
    //! b2 = b1.getReverse()
    //! \endcode
    Buffer
    getReverse() const
    { Buffer temp(*this); temp.reverse(); return temp; }

    //! Rounds the samples in this Buffer to the nearest integer value.
    //
    //! Values with remainders 0.5 or greater are rounded up, otherwise truncated.
    //!
    //! \par Example:
    //! \code
    //! // C++
    //! Buffer b1("california.wav");
    //! b1.round();
    //!
    //! // Python
    //! b1 = Buffer("california.wav")
    //! b1.round()
    //! \endcode
    void
    round();

    //! Rounds the samples in a copy of this Buffer to the nearest integer value.
    //
    //! Values with remainders 0.5 or greater are rounded up, otherwise truncated.
    //!
    //! \par Example:
    //! \code
    //! // C++
    //! Buffer b1("california.wav");
    //! Buffer b2 = b1.getRound();
    //!
    //! // Python
    //! b1 = Buffer("california.wav")
    //! b2 = b1.getRound()
    //! \endcode
    Buffer
    getRound() const
    { Buffer temp(*this); temp.round(); return temp; }

    //! Returns a BufferSelection for the range of indicies.
    //
    //! \param start_index the start of the selection
    //! \param stop_index the stop of the selection
    //!
    //! \par Example:
    //! \code
    //! // C++
    //! Buffer b1("california.wav");
    //! BufferSelection selection = b1.select(20,80);
    //!
    //! // Python
    //! b1 = Buffer("california.wav")
    //! selection = b1.select(20,80)
    //! \endcode
    BufferSelection
    select(const uint32 start_index, const uint32 stop_index);

    #ifndef SWIG
    //! Serializes the Buffer to output stream, no endian checks.
    //
    //! \param out the std::ostream to write bytes to
    //
    std::ostream &
    write(std::ostream & out) const;
    #endif

    std::string
    write() const;

    #ifndef SWIG
    //! Constructs a Buffer from seralized data in the inputstream.
    //
    //! \param in the std::istream to read bytes from
    //
    std::istream &
    read(std::istream & stream_in);
    #endif

    void
    read(const std::string & string_in);

    //! Applies a moving average filter to smooth this Buffer.
    //
    //! \param n_passes the number of times to repeat the moving average
    //! \param n_samples_per_average the number of samples to average
    //!
    //! \par Example:
    //! \code
    //! // C++
    //! Buffer b1("california.wav");
    //! b1.smooth(1, 15);
    //!
    //! // Python
    //! b1 = Buffer("california.wav")
    //! b1.smooth(1,15)
    //! \endcode
    void
    smooth(uint32 n_passes, uint32 n_samples_per_average);

    //! Applies a moving average filter to smooth a copy of this Buffer.
    //
    //! \param n_passes the number of times to repeat the moving average
    //! \param n_samples_per_average the number of samples to average
    //!
    //! \par Example:
    //! \code
    //! // C++
    //! Buffer b1("california.wav");
    //! Buffer b2 = b1.getSmooth(1, 15);
    //!
    //! // Python
    //! b1 = Buffer("california.wav")
    //! b2 = b1.getSmooth(1,15)
    //! \endcode
    Buffer
    getSmooth(uint32 n_passes, uint32 n_samples_per_average) const
    { Buffer temp(*this); temp.smooth(n_passes, n_samples_per_average); return temp; }

    //! Resamples this Buffer by the step_size, no interpolation.
    //
    //! \par Example:
    //! \code
    //! // C++
    //! Buffer b1("california.wav");
    //! b1.speedUp(2.0);                // b1 is now half the length
    //! b1.speedUp(0.5);                // b1 is now original length
    //!
    //! // Python
    //! b1 = Buffer("california.wav")
    //! b1.speedUp(2.0)                 // b1 is now half the length
    //! b1.speedUp(0.5)                 // b1 is now original length
    //! \endcode
    void
    speedUp(float64 step_size);

    //! Resamples a copy of this Buffer by the step_size, no interpolation.
    //
    //! \par Example:
    //! \code
    //! // C++
    //! Buffer b1("california.wav");
    //! b1.speedUp(2.0);                // b1 is now half the length
    //! b1.speedUp(0.5);                // b1 is now original length
    //!
    //! // Python
    //! b1 = Buffer("california.wav")
    //! b1.speedUp(2.0)                 // b1 is now half the length
    //! b1.speedUp(0.5)                 // b1 is now original length
    //! \endcode
    Buffer
    getSpeedUp(float64 step_size) const
    { Buffer temp(*this); temp.speedUp(step_size); return temp; }

    //! Resamples this Buffer on a sample by sample basis, no interpolation.
    //
    //! \par Example:
    //! \code
    //! // C++
    //! Buffer b1("california.wav");
    //! Sine gen(44100);
    //! Buffer factor = gen.generate(2.0, 5.0);
    //! b1.speedUp(factor);
    //!
    //! // Python
    //! b1 = Buffer("california.wav")
    //! gen = Sine(44100)
    //! factor = Buffer()
    //! factor << gen.generate(2.0, 5.0)
    //! b1.speedUp(factor)
    //! \endcode
    void
    speedUp(const Buffer & step_size);

    //! Resamples a copy of this Buffer on a sample by sample basis, no interpolation.
    //
    //! \par Example:
    //! \code
    //! // C++
    //! Buffer b1("california.wav");
    //! Sine gen(44100);
    //! Buffer factor = gen.generate(2.0, 5.0);
    //! Buffer b2 = b1.getSpeedUp(factor);
    //!
    //! // Python
    //! b1 = Buffer("california.wav")
    //! gen = Sine(44100)
    //! factor = Buffer()
    //! factor << gen.generate(2.0, 5.0)
    //! b2 = b1.getSpeedUp(factor)
    //! \endcode
    Buffer
    getSpeedUp(const Buffer & step_size) const
    { Buffer temp(*this); temp.speedUp(step_size); return temp; }

    //! Takes the square root of each sample in this Buffer.
    //
    //! \par Example:
    //! \code
    //! // C++
    //! Buffer b1("california.wav");
    //! b1.sqrt();
    //!
    //! // Python
    //! b1 = Buffer("california.wav")
    //! b1.sqrt()
    //! \endcode
    void
    sqrt();

    //! Taks the square root of each sample in a copy of this Buffer.
    //
    //! \par Example:
    //! \code
    //! // C++
    //! Buffer b1("california.wav");
    //! Buffer b2 = b1.getSqrt();
    //!
    //! // Python
    //! b1 = Buffer("california.wav")
    //! b2 = b1.getSqrt()
    //! \endcode
    Buffer
    getSqrt() const
    { Buffer temp(*this); temp.sqrt(); return temp; }

    //! Slice the Buffer.
    //
    //! \par Example:
    //! \code
    //! // C++
    //! Buffer b1("california.wav");
    //! Buffer b2 = b1.subbuffer(5,1000);
    //!
    //! // Python
    //! b1 = Buffer("california.wav")
    //! b2 = b1.subbuffer(5,1000)
    //! // OR
    //! b2 = b1[5:1005]
    //! \endcode
    Buffer
    subbuffer(uint32 start_index, uint32 n_samples = 0) const;

    //! Upsample this Buffer by a integral factor. N must be > 1.
    //
    //! \par Example:
    //! \code
    //! // C++
    //! Buffer b1("california.wav");
    //! b1.upSample(2);
    //!
    //! // Python
    //! b1 = Buffer("california.wav")
    //! b1.upSample(2)
    //! \endcode
    void
    upSample(uint32 n);

    //! Upsample a copy of this Buffer by a integral factor. N must be > 1.
    //
    //! \par Example:
    //! \code
    //! // C++
    //! Buffer b1("california.wav");
    //! Buffer b2 = b1.getUpSample(2);
    //!
    //! // Python
    //! b1 = Buffer("california.wav")
    //! b2 = b1.getUpSample(2)
    //! \endcode
    Buffer
    getUpSample(uint32 n) const;

    //! Writes the Buffer to a Wavefile.
    //
    //! \par Example:
    //! \code
    //! // C++
    //! Buffer b1("california.wav");
    //! b1.writeWavefile("california_copy.wav");
    //!
    //! // Python
    //! b1 = Buffer("california.wav")
    //! b1.writeWavefile("california_copy.wav")
    //! \endcode
    void
    writeWavefile(const char * filename) const;

    //! SWIG helper function function to shadow.
    void _set_at_index(int32 index, float64 x);

    //! SWIG helper function function to shadow.
    void _swig_shadow(){}

    //! Returns a Buffer full of ones of length n_samples.
    static
    Buffer
    ones(const uint32 n_samples);

    //! Returns a Buffer full of random values of length n_samples.
    static
    Buffer
    rand(const uint32 n_samples);

    //! Returns a Buffer full of zeros of length n_samples.
    static
    Buffer
    zeros(const uint32 n_samples);

protected:

    Buffer
    _get_resample(
        const uint32 L,
        const uint32 M,
        const uint32 N,
        float64 beta) const;

    FloatVector data_;

    static const uint32 bytes_per_sample_ = sizeof(float64);

}; // class Buffer



//-----------------------------------------------------------------------------
// Non class methods
//-----------------------------------------------------------------------------

//! Add the Buffers together on a sample by sample basis.
inline Buffer operator+(const Buffer & lhs, const Buffer & rhs)
{
    Buffer temp(lhs);
    return temp += rhs;
}

//! Subtract the Buffers together on a sample by sample basis.
inline Buffer operator-(const Buffer & lhs, const Buffer & rhs)
{
    Buffer temp(lhs);
    return temp -= rhs;
}

//! Multiply the Buffers together on a sample by sample basis.
inline Buffer operator*(const Buffer & lhs, const Buffer & rhs)
{
    Buffer temp(lhs);
    return temp *= rhs;
}

//! Divide the samples in the Buffers sample by sample basis.
inline Buffer operator/(const Buffer & lhs, const Buffer & rhs)
{
    Buffer temp(lhs);
    return temp /= rhs;
}

//! Raise the left hand side (lhs) samples to the power in the of the samples in the right hand side (rhs).
inline Buffer operator^(const Buffer & lhs, const Buffer & rhs)
{
    Buffer temp(lhs);
    return temp ^= rhs;
}

// Scalar operators

//! Add the scalar d to every sample in the Buffer.
inline Buffer operator+(const Buffer & lhs, float64 d)
{
    Buffer temp(lhs);
    return temp += d;
}

//! Subtract the scalar d to every sample in the Buffer.
inline Buffer operator-(const Buffer & lhs, float64 d)
{
    Buffer temp(lhs);
    return temp -= d;
}

//! Multiply the scalar d to every sample in the Buffer.
inline Buffer operator*(const Buffer & lhs, float64 d)
{
    Buffer temp(lhs);
    return temp *= d;
}

//! Divide every sample in the Buffer by d.
inline Buffer operator/(const Buffer & lhs, float64 d)
{
    Buffer temp(lhs);
    return temp /= d;
}

///////////////////////////////////////////////////////////////////////////
//! Each sample in the Buffer becomes the power x^n.
//
//! Each sample in the Buffer becomes the power x^n, where x is the Buffer
//! sample and N is the power.
//!
//! \par Example:
//! \code
//! // C++
//! Buffer b1("california.wav");
//! Buffer b2 = b1 ^ 2.0;
//!
//! // Python
//! b1 = Buffer("california.wav")
//! b2 = b1 ** 2.0
//! \endcode
//!
//! \par WARNING: sometimes one may want to do this:
//! \code
//! // C++
//! mag = real ^ 2 + imag ^ 2;
//! \endcode
//! But C++'s default operator precedence will actually compile this:
//! \code
//! mag = (real ^ (2 + imag)) ^ 2;
//! \endcode
//! because operator+ has high presedence.  To avoid this, in C++ you must
//! use parentheses.
//! \code
//! mag = (real ^ 2) + (imag ^ 2);
//! \endcode
inline Buffer operator^(const Buffer & lhs, float64 d)
{
    Buffer temp(lhs);
    return temp ^= d;
}

// Reverse scalar operators

//! Add every sample in the Buffer to the scalar d.
inline Buffer operator+(float64 d, const Buffer & rhs)
{
    return rhs + d;
}

//! Subtract every sample in the Buffer from the scalar d.
inline Buffer operator-(float64 d, const Buffer & rhs)
{
    return (rhs * -1.0) + d;
}

//! Multiply the scalar d by every sample in the Buffer
inline Buffer operator*(float64 d, const Buffer & rhs)
{
    return rhs * d;
}

//! Divide the scalar d by every sample in the Buffer
inline Buffer operator/(float64 d, const Buffer & rhs)
{
    Buffer temp(rhs);

    uint32 size = rhs.getLength();

    for(uint32 i = 0; i < size; ++i)
    {
        temp[i] = d / rhs[i];
    }

    return temp;
}

//! Raise the scalar d to the power of every sample in the Buffer
inline Buffer operator^(float64 d, const Buffer & rhs)
{
    Buffer temp(rhs);

    uint32 size = rhs.getLength();

    for(uint32 i = 0; i < size; ++i)
    {
        temp[i] = ::pow(d ,rhs[i]);
    }

    return temp;
}

// DOXME
typedef std::vector<Buffer>   BufferVector;
typedef std::vector<Buffer *> BufferPointerVector;

// Friends
std::ostream &
operator<<(std::ostream & out, const Buffer & rhs);

} // namespace

// :mode=c++: jEdit modeline
#endif
