//-----------------------------------------------------------------------------
//
//  $Id: AudioStream.cc 927 2015-08-23 18:46:27Z weegreenblobbie $
//
//  Copyright (c) 2009-Present Nick Hilton
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
#include <Nsound/Generator.h>
#include <Nsound/Nsound.h>
#include <Nsound/Plotter.h>
#include <Nsound/Sine.h>
#include <Nsound/Wavefile.h>
#include <Nsound/StreamOperators.h>

#include <algorithm>
#include <iostream>
#include <limits>
#include <sstream>

using namespace Nsound;

using std::cerr;
using std::cout;
using std::endl;
using std::flush;


AudioStream::
AudioStream()
    :
    sample_rate_(44100.0),
    channels_(1),
    buffers_(1, new Buffer())
{
    M_ASSERT_VALUE(channels_, !=, 0);
}

AudioStream::
AudioStream(
    float64 sample_rate,
    uint32 channels,
    uint32 n_samples_pre_allocate)
    :
    sample_rate_(sample_rate),
    channels_(channels),
    buffers_()
{
    M_ASSERT_VALUE(channels_, !=, 0);

    for(uint32 i =0 ; i < channels_; ++i)
    {
        auto ptr = new Buffer();
        ptr->preallocate(n_samples_pre_allocate);
        buffers_.push_back(ptr);
    }
}

AudioStream::
AudioStream(const std::string & filename)
    :
    sample_rate_(44100.0),
    channels_(1),
    buffers_(1, new Buffer())
{
    // This operator is defined in Wavefile.h.
    *this <<  filename.c_str();
}


AudioStream::
AudioStream(const AudioStream & copy)
    :
    sample_rate_(copy.sample_rate_),
    channels_(copy.channels_),
    buffers_()
{
    for(uint32 i = 0; i < channels_; ++i)
    {
        buffers_.push_back(new Buffer(*copy.buffers_[i]));
    }
}

AudioStream::
~AudioStream()
{
    for(auto * ptr : buffers_) delete ptr;
}

void
AudioStream::
abs()
{
    for(auto * ptr : buffers_) ptr->abs();
}

void
AudioStream::
add(
    const AudioStream & as,
    uint32 offset,
    uint32 n_samples)
{
    M_ASSERT_VALUE(as.channels_, <=, channels_);

    for(uint32 i = 0; i < as.channels_; ++i)
    {
        buffers_[i]->add(*as.buffers_[i], offset, n_samples);
    }
}

void
AudioStream::
add(
    const AudioStream & as,
    float64 offset_seconds,
    float64 duration_seconds)
{
    M_ASSERT_VALUE(as.channels_, <=, channels_);

    uint32 off       = static_cast<uint32>(offset_seconds   * sample_rate_);
    uint32 n_samples = static_cast<uint32>(duration_seconds * sample_rate_);

    for(uint32 i = 0; i < as.channels_; ++i)
    {
        buffers_[i]->add(*as.buffers_[i], off, n_samples);
    }
}

void
AudioStream::
convolve(const Buffer & b)
{
    for(auto * ptr : buffers_) ptr->convolve(b);
}

void
AudioStream::
dB()
{
    for(auto * ptr : buffers_) ptr->dB();
}

void
AudioStream::
derivative(uint32 n)
{
    for(auto * ptr : buffers_) ptr->derivative(n);
}


void
AudioStream::
downSample(uint32 n)
{
    for(auto * ptr : buffers_) ptr->downSample(n);
}

float64
AudioStream::
getDuration() const
{
    uint32 length = buffers_[0]->getLength();

    for(auto * ptr : buffers_)
    {
        length = std::min(length, ptr->getLength());
    }

    return static_cast<float64>(length) / sample_rate_;
}

uint32
AudioStream::
getLength() const
{
    uint32 length = buffers_[0]->getLength();

    for(auto * ptr : buffers_)
    {
        length = std::min(length, ptr->getLength());
    }

    return length;
}

void
AudioStream::
limit(float64 min, float64 max)
{
    for(auto * ptr : buffers_) ptr->limit(min, max);
}

void
AudioStream::
limit(const Buffer & min, const Buffer & max)
{
    for(auto * ptr : buffers_) ptr->limit(min, max);
}

float64
AudioStream::
getMax() const
{
    float64 max = std::numeric_limits<float64>::min();

    for(auto * ptr : buffers_)  max = std::max(max, ptr->getMax());

    return max;
}

float64
AudioStream::
getMaxMagnitude() const
{
    float64 max = std::numeric_limits<float64>::min();

    for(auto * ptr : buffers_) max = std::max(max, ptr->getMaxMagnitude());

    return max;
}

float64
AudioStream::
getMin() const
{
    float64 min = std::numeric_limits<float64>::max();

    for(auto * ptr : buffers_) min = std::min(min, ptr->getMin());

    return min;
}

void
AudioStream::
mono()
{
    *this = this->getMono();
}

AudioStream
AudioStream::
getMono() const
{
    AudioStream a(sample_rate_, 1);

    a << *buffers_[0];

    for(uint32 i = 1; i < channels_; ++i)
    {
        a += *buffers_[i];
    }

    float64 scale = 1.0 / static_cast<float64>(channels_);

    a *= scale;

    return a;
}

void
AudioStream::
normalize()
{
    float64 peak = getMaxMagnitude();

    float64 normal_factor = 1.0 / peak;

    *this *= normal_factor;
}


const Buffer &
AudioStream::
operator[](uint32 i) const
{
    M_ASSERT_VALUE(i, <, channels_);
    return *buffers_[i];
}

Buffer &
AudioStream::
operator[](uint32 i)
{
    M_ASSERT_VALUE(i, <, channels_);
    return *buffers_[i];
}

AudioStream &
AudioStream::
operator=(const AudioStream & rhs)
{
    if(this == &rhs) return *this;

    setNChannels(rhs.channels_);

    for(uint32 i = 0; i < rhs.channels_; ++i)
    {
        *buffers_[i] = *rhs.buffers_[i];
    }

    sample_rate_ = rhs.sample_rate_;

    return *this;
}

AudioStream &
AudioStream::
operator=(const Buffer & rhs)
{
    for(auto * ptr : buffers_)
    {
        *ptr = rhs;
    }

    return *this;
}

boolean
AudioStream::
operator==(const AudioStream & rhs) const
{
    if(channels_ != rhs.channels_)
    {
        return false;
    }

    if(getLength() != rhs.getLength())
    {
        return false;
    }

    for(uint32 i = 0; i < channels_; ++i)
    {
        if(*buffers_[i] != *rhs.buffers_[i])
        {
            return false;
        }
    }

    return true;
}

float64
AudioStream::
operator()(uint32 channel, uint32 index) const
{
    M_ASSERT_VALUE(channel, <, channels_);
    return (*buffers_[channel])[index];
}

AudioStreamSelection
AudioStream::
operator()(const BooleanVectorVector & bvv)
{
    return AudioStreamSelection(*this, bvv);
}

AudioStream &
AudioStream::
operator<<(const AudioStream & rhs)
{
    if(rhs.channels_ == 1) return *this << rhs[0];

    M_ASSERT_VALUE(channels_, ==, rhs.channels_);

    for(uint32 i = 0; i < channels_; ++i)
    {
        (*this)[i] << rhs[i];
    }

    return *this;
}

AudioStream &
AudioStream::
operator<<(const Buffer & rhs)
{
    for(auto * ptr : buffers_) *ptr << rhs;
    return *this;
}

AudioStream &
AudioStream::
operator+=(const AudioStream & rhs)
{
    if(rhs.channels_ == 1) return *this += rhs[0];

    M_ASSERT_VALUE(channels_, ==, rhs.getNChannels());

    for(uint32 i = 0; i < channels_; ++i)
    {
        (*this)[i] += rhs[i];
    }
    return *this;
}

AudioStream &
AudioStream::
operator+=(const Buffer & rhs)
{
    for(auto * ptr : buffers_) *ptr += rhs;
    return *this;
}

AudioStream &
AudioStream::
operator-=(const AudioStream & rhs)
{
    if(rhs.channels_ == 1) return *this -= rhs[0];

    M_ASSERT_VALUE(channels_, ==, rhs.channels_);

    for(uint32 i = 0; i < channels_; ++i)
    {
        (*this)[i] -= rhs[i];
    }
    return *this;
}

AudioStream &
AudioStream::
operator-=(const Buffer & rhs)
{
    for(auto * ptr : buffers_) *ptr -= rhs;
    return *this;
}

AudioStream &
AudioStream::
operator*=(const AudioStream & rhs)
{
    if(rhs.channels_ == 1) return *this *= rhs[0];

    M_ASSERT_VALUE(channels_, ==, rhs.channels_);

    for(uint32 i = 0; i < channels_; ++i)
    {
        (*this)[i] *= rhs[i];
    }
    return *this;
}

AudioStream &
AudioStream::
operator*=(const Buffer & rhs)
{
    for(auto * ptr : buffers_) *ptr *= rhs;
    return *this;
}


AudioStream &
AudioStream::
operator/=(const AudioStream & rhs)
{
    if(rhs.channels_ == 1) return *this /= rhs[0];

    M_ASSERT_VALUE(channels_, ==, rhs.channels_);

    for(uint32 i = 0; i < channels_; ++i)
    {
        (*this)[i] /= rhs[i];
    }
    return *this;
}


AudioStream &
AudioStream::
operator/=(const Buffer & rhs)
{
    for(auto * ptr : buffers_) *ptr /= rhs;
    return *this;
}


AudioStream &
AudioStream::
operator^=(const AudioStream & rhs)
{
    if(rhs.channels_ == 1) return *this ^= rhs[0];

    M_ASSERT_VALUE(channels_, ==, rhs.channels_);

    for(uint32 i = 0; i < channels_; ++i)
    {
        (*this)[i] ^= rhs[i];
    }
    return *this;
}


AudioStream &
AudioStream::
operator^=(const Buffer & rhs)
{
    for(auto * ptr : buffers_) *ptr ^= rhs;
    return *this;
}


AudioStream &
AudioStream::
operator<<(float64 d)
{
    for(auto * ptr : buffers_) *ptr << d;
    return *this;
}

AudioStream &
AudioStream::
operator+=(float64 d)
{
    for(auto * ptr : buffers_) *ptr += d;
    return *this;
}

AudioStream &
AudioStream::
operator-=(float64 d)
{
    for(auto * ptr : buffers_) *ptr -= d;
    return *this;
}

AudioStream &
AudioStream::
operator*=(float64 d)
{
    for(auto * ptr : buffers_) *ptr *= d;
    return *this;
}

AudioStream &
AudioStream::
operator/=(float64 d)
{
    for(auto * ptr : buffers_) *ptr /= d;
    return *this;
}

AudioStream &
AudioStream::
operator^=(float64 d)
{
    for(auto * ptr : buffers_) *ptr ^= d;
    return *this;
}

std::ostream &
Nsound::
operator<<(std::ostream & out, const AudioStream & rhs)
{
    uint32 n_samples = std::min(10u, rhs.getLength());

    for(uint32 i = 0; i < rhs.channels_; ++i)
    {
        out << "channel["
            << i
            << "].length = "
            << rhs.buffers_[i]->getLength()
            << endl
            << "channel["
            << i
            << "] = ";

        for(uint32 j = 0; j < n_samples; j++)
        {
            out << rhs[i][j]
                << " ";
        }
    }
    return out;
}


void
AudioStream::
pad(float64 fill)
{
    uint32 min = std::numeric_limits<uint32>::max();
    uint32 max = 0;

    for(auto * ptr : buffers_)
    {
        min = std::min(min, ptr->getLength());
        max = std::max(max, ptr->getLength());
    }

    if(min == max) return;

    for(auto * ptr : buffers_)
    {
        uint32 n_fill = max - ptr->getLength();

        if(n_fill == 0) continue;

        *ptr << fill * Buffer::ones(n_fill);
    }
}

void
AudioStream::
pan(float64 pan)
{
    M_ASSERT_VALUE(channels_, ==, 2);

    float64 left_amplitude  = (pan + 1.0) * 0.5;
    float64 right_amplitude = ((pan * -1.0) + 1.0) * 0.5;

    *buffers_[0] *= left_amplitude;
    *buffers_[1] *= right_amplitude;
}

void
AudioStream::
pan(const Buffer & pan)
{
    M_ASSERT_VALUE(channels_, ==, 2);

    uint32 length = std::min(getLength(), pan.getLength());

    Buffer & left = *buffers_[0];
    Buffer & right = *buffers_[1];

    for(uint32 i = 0; i < length; ++i)
    {
        float64 left_amplitude = (pan[i] + 1.0) * 0.5;
        float64 right_amplitude = ((pan[i] * -1.0) + 1.0) * 0.5;

        left[i] *= left_amplitude;
        right[i] *= right_amplitude;
    }
}

void
AudioStream::
plot(const std::string & title) const
{
    Plotter pylab;

    pylab.figure();

    uint32 n_rows = channels_;
    uint32 n_columns = 1;

    // Create the x axis based on seconds.
    Sine sin(sample_rate_);

    Buffer x_axis = sin.drawLine(getDuration(), 0.0, getDuration());

    // For each buffer, plot it
    uint32 i = 0;
    for(auto * ptr : buffers_)
    {
        pylab.subplot(n_rows, n_columns, i + 1);

        if(i == 0)
        {
            pylab.title(title);
        }

        pylab.plot(x_axis, *ptr);

        pylab.xlabel("Time (sec)");
        pylab.ylabel("Amplitude");

        ++i;
    }
}

void
AudioStream::
readWavefile(const char * filename)
{
    M_CHECK_PTR(filename);

    *this << filename;
}

void
AudioStream::
resample(float64 factor)
{
    for(auto * ptr : buffers_) ptr->resample(factor);
}

void
AudioStream::
resample(const Buffer & factor)
{
    for(auto * ptr : buffers_) ptr->resample(factor);
}

void
AudioStream::
resample2(float64 new_sample_rate)
{
    M_ASSERT_VALUE(new_sample_rate, >, 0.0);

    float64 ratio = new_sample_rate / sample_rate_;

    resample(ratio);

    sample_rate_ = new_sample_rate;
}

void
AudioStream::
reverse()
{
    for(auto * ptr : buffers_) ptr->reverse();
}

AudioStreamSelection
AudioStream::
select(const uint32 start_index, const uint32 stop_index)
{
    BooleanVectorVector bvv;

    uint32 n_samples = getLength();

    for(uint32 i = start_index; i < stop_index; ++i)
    {
        // Create a BooleanVector of true for the length of the Buffer
        BooleanVector bv(n_samples, true);

        bvv.push_back(bv);
    }

    return AudioStreamSelection(*this, bvv);
}

void
AudioStream::
_set_at_index(int32 i, const Buffer & b)
{
    int32 len = getNChannels();

    if(i >= 0 && i < len)
    {
        *buffers_[i] = b;
    }
    else
    if(i < 0 && i >= -len)
    {
        *buffers_[len + i] = b;
    }
    else
    {
        M_THROW(
            "IndexError: " << i << " is out of bounds (0 : " << len << ")");
    }
}

void
AudioStream::
setNChannels(uint32 channels)
{
    if(channels_ != channels)
    {
        for(auto * ptr : buffers_) delete ptr;

        buffers_.clear();

        channels_ = channels;

        for(uint32 i = 0 ; i < channels_; ++i) buffers_.push_back(new Buffer());
    }
}

void
AudioStream::
smooth(uint32 n_passes, uint32 n_samples_per_average)
{
    for(auto * ptr : buffers_) ptr->smooth(n_passes, n_samples_per_average);
}

void
AudioStream::
speedUp(float32 step_size)
{
    for(auto * ptr : buffers_) ptr->speedUp(step_size);
}

void
AudioStream::
speedUp(const Buffer & step_buffer)
{
    for(auto * ptr : buffers_) ptr->speedUp(step_buffer);
}

void
AudioStream::
sqrt()
{
    for(auto * ptr : buffers_) ptr->sqrt();
}

AudioStream
AudioStream::
substream(uint32 start_index, uint32 n_samples) const
{
    AudioStream new_stream(sample_rate_, channels_);

    for(uint32 i = 0; i < channels_; ++i)
    {
        new_stream[i] = (*this)[i].subbuffer(start_index, n_samples);
    }

    return new_stream;
}


AudioStream
AudioStream::
substream(int32 start_index, int32 n_samples) const
{
    M_ASSERT_VALUE(start_index, >=, 0);
    M_ASSERT_VALUE(n_samples, >=, 0);

    return substream(
        static_cast<uint32>(start_index),
        static_cast<uint32>(n_samples));
}


AudioStream
AudioStream::
substream(float64 start_time, float64 n_seconds) const
{
    AudioStream new_stream(sample_rate_, channels_);

    uint32 start_index = static_cast<uint32>(start_time * sample_rate_);
    uint32 n_samples = static_cast<uint32>(n_seconds * sample_rate_);

    if(n_seconds == 0 || n_samples + start_index >= getLength())
    {
        n_samples = getLength() - start_index;
    }

    for(uint32 i = 0; i < channels_; ++i)
    {
        new_stream[i] = buffers_[i]->subbuffer(start_index, n_samples);
    }

    return new_stream;
}

void
AudioStream::
transpose()
{
    // Pad with zeros if necessary.
    this->pad(0.0);

    AudioStream new_as(getSampleRate(), getLength());

    for(uint32 i = 0; i < getLength(); ++i)
    {
        for(uint32 j = 0; j < channels_; ++j)
        {
            new_as[i] << (*this)[j][i];
        }
    }

    *this = new_as;
}

void
AudioStream::
upSample(uint32 n)
{
    for(auto * ptr : buffers_) ptr->upSample(n);
}

std::ostream &
AudioStream::
write(std::ostream & out) const
{
    out & 'a' & 'u' & 'd' & 'i' & 'o' & 's' & 't' & 'r'
        & getSampleRate()
        & getNChannels();

    for(auto * ptr : buffers_) ptr->write(out);

    return out;
}

std::string
AudioStream::
write() const
{
    std::stringstream ss;
    write(ss);
    return ss.str();
}

std::istream &
AudioStream::
read(std::istream & in)
{
    char id[8];
    float64 sr = 0;
    uint32 n_channels = 0;

    in
        & id[0] & id[1] & id[2] & id[3] & id[4] & id[5] & id[6] & id[7]
        & sr & n_channels;

    if(
        id[0] != 'a' || id[1] != 'u' || id[2] != 'd' || id[3] != 'i' ||
        id[4] != 'o' || id[5] != 's' || id[6] != 't' || id[7] != 'r')
    {
        M_THROW("Did not find any Nsound AudioStream data in input stream!");
    }

    sample_rate_ = sr;

    setNChannels(n_channels);

    for(auto * ptr : buffers_) ptr->read(in);

    return in;
}

void
AudioStream::
read(const void * data, std::size_t size)
{
    std::string in(reinterpret_cast<const char *>(data), size);
    std::stringstream ss(in);
    read(ss);
}

void
AudioStream::
writeWavefile(const char * filename) const
{
    M_CHECK_PTR(filename);
    *this >> filename;
}


BooleanVectorVector
Nsound::
AudioStream::
operator>(float64 rhs)
{
    BooleanVectorVector bv;

    for(auto * ptr : buffers_) bv.push_back( *ptr > rhs );

    return bv;
}

BooleanVectorVector
Nsound::
AudioStream::
operator>=(float64 rhs)
{
    BooleanVectorVector bv;

    for(auto * ptr : buffers_) bv.push_back( *ptr >= rhs );

    return bv;
}

BooleanVectorVector
Nsound::
AudioStream::
operator<(float64 rhs)
{
    BooleanVectorVector bv;

    for(auto * ptr : buffers_) bv.push_back( *ptr < rhs );

    return bv;
}

BooleanVectorVector
Nsound::
AudioStream::
operator<=(float64 rhs)
{
    BooleanVectorVector bv;

    for(auto * ptr : buffers_) bv.push_back( *ptr <= rhs );

    return bv;
}

BooleanVectorVector
Nsound::
AudioStream::
operator==(float64 rhs)
{
    BooleanVectorVector bv;

    for(auto * ptr : buffers_) bv.push_back( *ptr == rhs );

    return bv;
}

BooleanVectorVector
Nsound::
AudioStream::
operator!=(float64 rhs)
{
    BooleanVectorVector bv;

    for(auto * ptr : buffers_) bv.push_back( *ptr != rhs );

    return bv;
}

AudioStream
AudioStream::
ones(
    float64 sample_rate,
    const uint32 n_channels,
    float64 duration)
{
    AudioStream a(sample_rate, n_channels);

    Generator g(sample_rate);

    a << g.drawLine(duration, 1.0, 1.0);

    return a;
}

AudioStream
AudioStream::
rand(
    float64 sample_rate,
    const uint32 n_channels,
    float64 duration)
{
    AudioStream a(sample_rate, n_channels);

    Generator g(sample_rate);

    a << g.whiteNoise(duration);

    return a;
}

AudioStream
AudioStream::
zeros(
    float64 sample_rate,
    const uint32 n_channels,
    float64 duration)
{
    AudioStream a(sample_rate, n_channels);

    Generator g(sample_rate);

    a << g.drawLine(duration, 0.0, 0.0);

    return a;
}
