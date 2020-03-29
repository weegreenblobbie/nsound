//-----------------------------------------------------------------------------
//
//  $Id: AudioStream.h 912 2015-07-26 00:50:29Z weegreenblobbie $
//
//  Nsound is a C++ library and Python module for audio synthesis featuring
//  dynamic digital filters. Nsound lets you easily shape waveforms and write
//  to disk or plot them. Nsound aims to be as powerful as Csound but easy to
//  use.
//
//  Copyright (c) 2004-Present Nick Hilton
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
#ifndef _NSOUND_AUDIO_STREAM_H_
#define _NSOUND_AUDIO_STREAM_H_

#include <Nsound/Nsound.h>
#include <Nsound/AudioStreamSelection.h>
#include <Nsound/Buffer.h>

#include <iostream>
#include <string>
#include <vector>

namespace Nsound
{

//-----------------------------------------------------------------------------
//
//  Class AudioStream
//
//-----------------------------------------------------------------------------
class AudioStream
{

public:

    //! Create an AudioStream.
    AudioStream();
    AudioStream(
        float64 sample_rate,
        uint32 n_channels = 1,
        uint32 n_samples_pre_allocate = 4096);

    AudioStream(const std::string & filename);

    //! Destructor
    ~AudioStream();

    //! Copy Constructor
    AudioStream(const AudioStream & rhs);

    // abs()
    //
    //! This method calls abs on all buffers held in the stream.
    //
    void
    abs();

    //! Modifies the AudioStream by making any negative value positive.
    AudioStream
    getAbs() const
    {AudioStream temp(*this); temp.abs(); return temp;};

    // add()
    //
    //! This method adds the passed AudioStream to this AudioStream.
    //
    //! This method adds the passed AudioStream to this AudioStream.
    //! If the length of as is longer than this AudioStream, this
    //! AudioStream will be extended.  If n_samples is 0, the whole
    //! length of as will get added, otherwise, only n_samples will get
    //! added.
    //!
    //! \param offset - The index into this AudioStream to start adding
    //!
    //! \param as - The AudioStream to add data from
    //!
    //! \param n_samples - The number of samples from as to add
    void
    add(const AudioStream & as,
        uint32 offset,
        uint32 n_samples = 0);

    // add()
    //
    //! This method adds the passed AudioStream to this AudioStream.
    //
    //! This method adds the passed AudioStream to this AudioStream.
    //! If the length of as is longer than this AudioStream, this
    //! AudioStream will be extended.  If duration_seconds is 0, the whole
    //! length of as will get added, otherwise, only duration_seconds will get
    //! added.
    //!
    //! \param offset_seconds - The offset in seconds into this AudioStream to
    //!                         start adding or appending
    //!
    //! \param as - The source AudioStream to add
    //!
    //! \param duration_seconds - The number of seconds from the source
    //!                           AudioStream to add or append.
    void
    add(
        const AudioStream & as,
        float64 offset_seconds,
        float64 duration_seconds = 0.0);


    std::vector<Buffer *>::iterator begin() { return buffers_.begin(); }
    std::vector<Buffer *>::iterator end() { return buffers_.end(); }

    std::vector<Buffer *>::const_iterator begin() const { return buffers_.cbegin(); }
    std::vector<Buffer *>::const_iterator end() const { return buffers_.cend(); }

    //! Convolves every channel in the AudioStream with the Buffer.
    void
    convolve(const Buffer & b);

    //! Convolves every channel in the AudioStream with the Buffer.
    AudioStream
    getConvolve(const Buffer & b) const
    { AudioStream temp(*this); temp.convolve(b); return temp; };

    //! Modifies the AudioStream so each sample is converted to dB, 20 * log10(sample).
    void
    dB();

    //! Returns the AudioStream in dB.
    AudioStream
    getdB() const
    {AudioStream temp(*this); temp.dB(); return temp;};

    //! Calculates the nth derivative of the AudioStream.
    void
    derivative(uint32 n);

    //! Returns the nth derivative of the Buffer.
    AudioStream
    getDerivative(uint32 n) const
    { AudioStream temp(*this); temp.derivative(n); return temp; };

    //! Downsample by a integral factor. N must be > 1.
    void
    downSample(uint32 n);

    //! Downsample by a integral factor. N must be > 1.
    AudioStream
    getDownSample(uint32 n) const
    { AudioStream temp(*this); temp.downSample(n); return temp; };

    //! Returns the number of seconds of audio data in the stream.
    float64
    getDuration() const;

    //! Returns the number of samples of audio data in the stream.
    uint32
    getLength() const;

    //! Limits the AudioStream the min and max values.
    void
    limit(float64 min, float64 max);

    //! Limits the AudioStream the min and max values.
    void
    limit(const Buffer & min, const Buffer & max);

    //! Limits the AudioStream the min and max values.
    AudioStream
    getLimit(float64 min, float64 max) const
    { AudioStream temp(*this); temp.limit(min, max); return temp; };

    //! Limits the AudioStream the min and max values.
    AudioStream
    getLimit(const Buffer & min, const Buffer & max) const
    { AudioStream temp(*this); temp.limit(min, max); return temp; };

    //! Returns the maximum sample value in the Audiostream.
    float64
    getMax() const;

    //! Returns the maximum sample magnitude value in the AudioStream, i.e. max(abs(samples))..
    float64
    getMaxMagnitude() const;

    //! Returns the minimum sample value in the AudioStream.
    float64
    getMin() const;

    //! Collapses all channels into one Buffer making it mono.
    void
    mono();

    //! Collapses all channels into one Buffer making it mono.
    AudioStream
    getMono() const;

    //! Returns the number of audio channels in the stream.
    inline
    uint32
    getNChannels(void) const {return channels_;}

    //! Returns the sample rate of the stream.
    inline
    float64
    getSampleRate() const {return sample_rate_;}

    //! Multiplies the AudioStream by a constant gain so the peak sample has magnitude 1.0.
    void
    normalize();

    //! Returns the normalized AudioStream.
    AudioStream
    getNormalize() const
    {AudioStream temp(*this); temp.normalize(); return temp;};

    //! Returns an AudioStreamSelection object used for manipulation of a selected region of samples.
    AudioStreamSelection
    operator()(const BooleanVectorVector & bv);

    //!  These methods provide a reference to the Buffer object held in the channel.
    const Buffer &
    operator[](uint32 index) const;

    Buffer &
    operator[](uint32 index);

    //  Operator ()
    //! Returns one sample from the specified channel and index.
    float64
    operator()(uint32 channel, uint32 index) const;

    //  Operator =
    //! Assignment operator.
    AudioStream &
    operator=(const AudioStream & rhs);

    //  Operator =
    //! Assignment operator, deletes any existing data and sets one channels to rhs.
    AudioStream &
    operator=(const Buffer & rhs);

    //  Operator ==
    //! Boolean == operator.
    boolean
    operator==(const AudioStream & rhs) const;

    //  Operator !=
    //! Boolean != operator.
    inline boolean
    operator!=(const AudioStream & rhs) const {return !(*this == rhs);};

    //! Concatenates or appends rhs to the AudioStream.
    AudioStream &
    operator<<(const AudioStream & rhs);

    //! Concatenates or appends rhs to all the channels in the AudioStream.
    AudioStream &
    operator<<(const Buffer & rhs);

    //! Concatenates or appends rhs to all the channels in the AudioStream.
    AudioStream &
    operator<<(float64 d);

    BooleanVectorVector operator>(float64 rhs);
    BooleanVectorVector operator>=(float64 rhs);
    BooleanVectorVector operator<(float64 rhs);
    BooleanVectorVector operator<=(float64 rhs);
    BooleanVectorVector operator==(float64 rhs);
    BooleanVectorVector operator!=(float64 rhs);

    //
    //  math operators
    //
    // DOXME
    AudioStream & operator+=(const AudioStream & rhs);
    AudioStream & operator-=(const AudioStream & rhs);
    AudioStream & operator*=(const AudioStream & rhs);
    AudioStream & operator/=(const AudioStream & rhs);
    AudioStream & operator^=(const AudioStream & rhs);

    AudioStream & operator+=(const Buffer & rhs);
    AudioStream & operator-=(const Buffer & rhs);
    AudioStream & operator*=(const Buffer & rhs);
    AudioStream & operator/=(const Buffer & rhs);
    AudioStream & operator^=(const Buffer & rhs);

    AudioStream & operator+=(float64 d);
    AudioStream & operator-=(float64 d);
    AudioStream & operator*=(float64 d);
    AudioStream & operator/=(float64 d);
    AudioStream & operator^=(float64 d);

    #ifndef SWIG
    //  Output Operator << AudioStream
    // DOXME
    friend
    std::ostream &
    operator<<(std::ostream & out, const AudioStream & rhs);
    #endif

    //! Pads the AudioStream so that each channel has exactly the same number of samples.
    //
    // DOXME
    void
    pad(float64 fill = 0.0);

    //
    // DOXME
    AudioStream
    getPad(float64 fill = 0.0) const
    { AudioStream temp(*this); temp.pad(fill); return temp;};

    //! Sets the amplitude level left vs right.
    //
    //! Sets the amplitude level between left and right channels.
    //!
    //!<pre>
    //!  1 = 100% left volume,   0% right volume
    //!  0 =  50% left volume,  50% right volume
    //! -1 =   0% left volume, 100% right volume
    //!</pre>
    //!
    //! \param pan - Static levels between channels accross all samples
    //
    void
    pan(float64 pan);

    //! Sets the amplitude level left vs right.
    //
    //! Sets the amplitude level between left and right channels.
    //!
    //!<pre>
    //!  1 = 100% left volume,   0% right volume
    //!  0 =  50% left volume,  50% right volume
    //! -1 =   0% left volume, 100% right volume
    //!</pre>
    //!
    //! \param pan - Static levels between channels accross all samples
    //
    AudioStream
    getPan(float64 pan) const
    { AudioStream temp(*this); temp.pan(pan); return temp;};

    //! Sets the amplitude level left vs right.
    //
    //! Sets the amplitude level between left and right channels.
    //!
    //!<pre>
    //!  1 = 100% left volume,   0% right volume
    //!  0 =  50% left volume,  50% right volume
    //! -1 =   0% left volume, 100% right volume
    //!</pre>
    //!
    //! \param pan - Dynamic levels between channels
    //
    void
    pan(const Buffer & pan);

    //! Sets the amplitude level left vs right.
    //
    //! Sets the amplitude level between left and right channels.
    //!
    //!<pre>
    //!  1 = 100% left volume,   0% right volume
    //!  0 =  50% left volume,  50% right volume
    //! -1 =   0% left volume, 100% right volume
    //!</pre>
    //!
    //! \param pan - Dynamic levels between channels
    //
    AudioStream
    getPan(const Buffer & pan) const
    { AudioStream temp(*this); temp.pan(pan); return temp;};

    // Plot the AudioStream
    void
    plot(const std::string & title = "AudioStream") const;

    //! Read a Wavefile.
    void
    readWavefile(const char * filename);

    //! Resample by a non-integer factor.
    void
    resample(float64 factor);

    //! Resample by a dynamic non-integer factor.
    void
    resample(const Buffer & factor);

    //! Resample by a non-integer factor.
    AudioStream
    getResample(float64 factor) const
    { AudioStream temp(*this); temp.resample(factor); return temp; };

    //! Resample by a dynamic non-integer factor.
    AudioStream
    getResample(const Buffer & factor) const
    { AudioStream temp(*this); temp.resample(factor); return temp; };

    //! Resample to the specified sample rate.
    void
    resample2(float64 new_sample_rate);

    //! Resample to the specified sample rate.
    AudioStream
    getResample2(float64 new_sample_rate) const
    {
        AudioStream temp(*this);
        temp.resample2(new_sample_rate);
        return temp;
    };

    // reverse()
    //! Reverses the AudioStream.
    void
    reverse();

    //! Reverses the samples in the AudioStream.
    AudioStream
    getReverse() const
    {AudioStream temp(*this); temp.reverse(); return temp;};

    //! Returns an AudioStreamSelection for the range of indicies.
    //
    //! \param start_index the start of the selection
    //! \param stop_index the stop of the selection
    //!
    //! \par Example:
    //! \code
    //! // C++
    //! AudioStream a1("california.wav");
    //! AudioStreamSelection selection = a1.select(20,80);
    //!
    //! // Python
    //! a1 = AudioStream("california.wav")
    //! selection = a1.select(20,80)
    //! \endcode
    AudioStreamSelection
    select(const uint32 start_index, const uint32 stop_index);

    //! Serializes the AudioStream to output stream, no endian checks.
    //
    //! \param out the std::ostream to write bytes to
    //
    std::ostream &
    write(std::ostream & out) const;

    std::string
    write() const;

    //! Constructs an AudioStream from seralized data in the inputstream.
    //
    //! \param in the std::istream to read bytes from
    //
    std::istream &
    read(std::istream & stream_in);

    void
    read(const std::string & string_in);

    //  setNChannels()
    // DOXME
    void
    setNChannels(uint32 channels);

    // setSampleRate()
    // DOXME
    inline
    void
    setSampleRate(uint32 sample_rate) {sample_rate_ = sample_rate;}

    //! Implements a standard moving average filter to smooth the waveform.
    //
    //! n_passes = the number of times to calculate the moving average
    //! n_samples_per_average = the number of samples to average to create one
    //!                         sample in the output.
    void
    smooth(uint32 n_passes, uint32 n_samples_per_average);

    //! Implements a standard moving average filter to smooth the waveform.
    AudioStream
    getSmooth(uint32 n_passes, uint32 n_samples_per_average) const
    {AudioStream temp(*this); temp.smooth(n_passes, n_samples_per_average); return temp;};

    //! Resamples the AudioStream by the step_size, no interpolation.
    void
    speedUp(float32 step_size);

    //! Resamples the AudioStream by the step_size, no interpolation.
    AudioStream
    getSpeedUp(float32 step_size) const
    { AudioStream temp(*this); temp.speedUp(step_size); return temp; };

    //! Resamples the AudioStream by a variable step_size, no interpolation.
    void
    speedUp(const Buffer & step_size);

    //! Resamples the AudioStream by a variable step_size, no interpolation.
    AudioStream
    getSpeedUp(const Buffer & step_size) const
    { AudioStream temp(*this); temp.speedUp(step_size); return temp; };

    //! Takes the square root of each sample in the AudioStream.
    void
    sqrt();

    //! Returns an AudioStream after taking the square root of each sample.
    AudioStream
    getSqrt() const
    { AudioStream temp(*this); temp.sqrt(); return temp; };

    // DOXME
    AudioStream
    substream(uint32 start_index, uint32 n_samples = 0) const;

    AudioStream
    substream(int32 start_index, int32 n_samples = 0) const;

    // DOXME
    AudioStream
    substream(float64 start_time, float64 duration = 0) const;

    //! Treating the AudioStream as a matrix, this peforms a matrix transpose.
    //
    //! Treat the AudioStream like a 2D matrix where the number of channels is
    //! the first dimension, or the rows, and the length of each Buffer in the
    //! AudioStream is the number of columns.
    //!
    //! Transposing the matrix swaps the rows with the columns.  For example,
    //! the first row (or AudioStream channel) becomes the first column, etc.
    //!
    //! | 0  1 |^T     | 0  2  4 |
    //! | 2  3 |    =  | 1  3  5 |
    //! | 4  5 |
    //!
    void
    transpose();

    //! Retuns a copy of the AudioStream transposed.
    AudioStream
    getTranspose() const
    { AudioStream temp(*this); temp.transpose(); return temp; };

    //! Upsample by a integral factor. N must be > 1.
    void
    upSample(uint32 n);

    //! Upsample by a integral factor. N must be > 1.
    AudioStream
    getUpSample(uint32 n) const
    { AudioStream temp(*this); temp.upSample(n); return temp; };

    //! Write the AudioStream to a Wavefile.
    void
    writeWavefile(const char * filename) const;

    //! SWIG helper function
    void _set_at_index(int32 i, const Buffer &);

    //! SWIG helper function function to shadow.
    void _swig_shadow(){};

    //! Returns an AudioStream full of ones of duration seconds.
    static
    AudioStream
    ones(
        float64 sample_rate,
        const uint32 n_channels,
        float64 duration);

    //! Returns a Buffer full of random values of length n_samples.
    static
    AudioStream
    rand(
        float64 sample_rate,
        const uint32 n_channels,
        float64 duration);

    //! Returns a Buffer full of zeros of length n_samples.
    static
    AudioStream
    zeros(
        float64 sample_rate,
        const uint32 n_channels,
        float64 duration);

private:

//~    // DOXME
//~    enum MathOperator
//~    {
//~        ADD,
//~        SUBTRACT,
//~        MULTIPLY,
//~        DIVIDE,
//~        POW
//~    };

//~
//~
//~    AudioStream & vectorOperator(const AudioStream & rhs, MathOperator op);
//~    AudioStream & scalarOperator(float64 rhs, MathOperator op);

    // Data members

    float64  sample_rate_;
    uint32   channels_;

    std::vector<Buffer *> buffers_;

}; // class

//-----------------------------------------------------------------------------
// Non class methods

inline AudioStream operator+(const AudioStream & lhs, const AudioStream & rhs)
{
    AudioStream temp(lhs);
    return temp += rhs;
}

inline AudioStream operator-(const AudioStream & lhs, const AudioStream & rhs)
{
    AudioStream temp(lhs);
    return temp -= rhs;
}

inline AudioStream operator*(const AudioStream & lhs, const AudioStream & rhs)
{
    AudioStream temp(lhs);
    return temp *= rhs;
}

inline AudioStream operator/(const AudioStream & lhs, const AudioStream & rhs)
{
    AudioStream temp(lhs);
    return temp /= rhs;
}

inline AudioStream operator^(const AudioStream & lhs, const AudioStream & rhs)
{
    AudioStream temp(lhs);
    return temp ^= rhs;
}

inline AudioStream operator+(const AudioStream & lhs, const Buffer & rhs)
{
    AudioStream temp(lhs);
    return temp += rhs;
}

inline AudioStream operator-(const AudioStream & lhs, const Buffer & rhs)
{
    AudioStream temp(lhs);
    return temp -= rhs;
}

inline AudioStream operator*(const AudioStream & lhs, const Buffer & rhs)
{
    AudioStream temp(lhs);
    return temp *= rhs;
}

inline AudioStream operator/(const AudioStream & lhs, const Buffer & rhs)
{
    AudioStream temp(lhs);
    return temp /= rhs;
}

inline AudioStream operator^(const AudioStream & lhs, const Buffer & rhs)
{
    AudioStream temp(lhs);
    return temp ^= rhs;
}

inline AudioStream operator+(const AudioStream & lhs, float64 d)
{
    AudioStream temp(lhs);
    return temp += d;
}

inline AudioStream operator-(const AudioStream & lhs, float64 d)
{
    AudioStream temp(lhs);
    return temp -= d;
}

inline AudioStream operator*(const AudioStream & lhs, float64 d)
{
    AudioStream temp(lhs);
    return temp *= d;
}

inline AudioStream operator/(const AudioStream & lhs, float64 d)
{
    AudioStream temp(lhs);
    return temp /= d;
}

inline AudioStream operator^(const AudioStream & lhs, float64 d)
{
    AudioStream temp(lhs);
    return temp ^= d;
}

inline AudioStream operator+(float64 d, const AudioStream & rhs)
{
    AudioStream temp(rhs);
    return temp += d;
}

inline AudioStream operator-(float64 d, const AudioStream & rhs)
{
    AudioStream temp(rhs * -1.0);
    return temp += d;
}

inline AudioStream operator*(float64 d, const AudioStream & rhs)
{
    AudioStream temp(rhs);
    return temp *= d;
}

inline AudioStream operator/(float64 d, const AudioStream & rhs)
{
    AudioStream temp(rhs.getSampleRate(), rhs.getNChannels());

    for(uint32 i = 0; i < rhs.getNChannels(); ++i)
    {
        temp[i] = d / rhs[i];
    }

    return temp;
}


}// Nsound

// :mode=c++:

#endif
