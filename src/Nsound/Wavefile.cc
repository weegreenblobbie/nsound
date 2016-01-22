//-----------------------------------------------------------------------------
//
//  $Id: Wavefile.cc 875 2014-09-27 22:25:13Z weegreenblobbie $
//
//  Nsound is a C++ library and Python module for audio synthesis featuring
//  dynamic digital filters. Nsound lets you easily shape waveforms and write
//  to disk or plot them. Nsound aims to be as powerful as Csound but easy to
//  use.
//
//  Copyright (c) 2004-2006 Nick Hilton
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

#include <Nsound/Nsound.h>

#include <Nsound/AudioStream.h>
#include <Nsound/Buffer.h>
#include <Nsound/Wavefile.h>

#include <math.h>
#include <string.h>
#include <stdio.h>
#include <fstream>
#include <iostream>
#include <sstream>

using std::cerr;
using std::endl;
using std::ifstream;
using std::ios_base;
using std::ofstream;
using std::string;
using std::stringstream;

using namespace Nsound;

const raw_float64 Wavefile::SIGNED_64_BIT_ = 9223372036854775807.0;
const raw_float64 Wavefile::SIGNED_48_BIT_ = 140737488355327.0;
const raw_float64 Wavefile::SIGNED_32_BIT_ = 2147483647.0;
const raw_float64 Wavefile::SIGNED_24_BIT_ = 8388607.0;
const raw_float64 Wavefile::SIGNED_16_BIT_ = 32767.0;
const raw_float64 Wavefile::SIGNED_8_BIT_  = 127.0;

uint32 Wavefile::default_sample_rate_ = 44100;
uint32 Wavefile::default_sample_size_ = 16;
uint16 Wavefile::default_wave_format_ = Wavefile::WAVE_FORMAT_PCM_;

void
Wavefile::
setDefaultSampleRate(int32 rate)
{
    M_ASSERT_VALUE(rate, >, 0);

    default_sample_rate_ = rate;
}

void
Wavefile::
setDefaultSampleSize(uint32 size)
{
    if(size == 64 ||
       size == 48 ||
       size == 32 ||
       size == 24 ||
       size == 16 ||
       size == 8)
    {
        default_sample_size_ = size;
    }
    else
    {
        M_THROW("setDefaultSampleSize(): Can't use size " << size);
    }
}

void
Wavefile::
setIEEEFloat(boolean flag)
{
    if(flag)
    {
    	default_wave_format_ = WAVE_FORMAT_IEEE_FLOAT_;
    	if(default_sample_size_ > 32)
    	{
    		default_sample_size_ = 64;
    	}
    	else
    	{
    		default_sample_size_ = 32;
    	}
    }
    else     default_wave_format_ = WAVE_FORMAT_PCM_;
}

void
Wavefile::
setDefaults(
    const float64 & sample_rate,
    const float64 & sample_bits,
    const boolean & use_ieee_floats)
{
    setDefaultSampleRate(sample_rate);
    setDefaultSampleSize(static_cast<uint32>(sample_bits));
    setIEEEFloat(use_ieee_floats);
}

#ifdef __clang__
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wunused-function"
#endif

//-----------------------------------------------------------------------------
//
//! Endian Conversion
//
//! little endian: intel
//! big endian: power pc, sparc
//!
//! 4 bytes
//! little endian byte order: 1234
//! big endian byte order:    4321
//!
//! 3 bytes
//! little endian byte order: 123
//! big endian byte order:    321
//!
//! 2 bytes
//! little endian byte order: 12
//! big endian byte order:    21
//!
//! The method below takes an int and swaps the bytes.  Note that on either
//! endian system the method will do the correct converstion.  That is, a big
//! endian value will become a little (4321 --> 1234), and a little endian
//! becomes big (1234 --> 4321).
inline static
int64
swapBytesInt64(const uint64 & i)
{
    int64 k = i;
    uint8 * c = reinterpret_cast<uint8 *>(&k);

    //    0 1 2 3 4 5 6 7
    // 0x1234567890abcdef
    uint8 t = c[0];
    c[0] = c[7];
    c[7] = t;
    t = c[1];
    c[1] = c[6];
    c[6] = t;
    t = c[2];
    c[2] = c[5];
    c[5] = t;
    t = c[3];
    c[3] = c[4];
    c[4] = t;

    return k;
};

inline static
int64
swapBytesInt48(const uint64 & i)
{
    int64 k = i;
    uint8 * c = reinterpret_cast<uint8 *>(&k);

    //    0 1 2 3 4 5 6 7
    // 0x1234567890abcdef

    uint8 t = c[2];
    c[2] = c[7];
    c[7] = t;
    t = c[3];
    c[3] = c[6];
    c[6] = t;
    t = c[4];
    c[4] = c[5];
    c[5] = t;

    return k;
};


inline static
int32
swapBytesInt32(const uint32 i)
{
    return static_cast<int32>(((i & 0xff000000) >> 24 |
                              ((i & 0x00ff0000) >>  8 |
                              ((i & 0x0000ff00) <<  8 |
                              ((i & 0x000000ff) << 24)))));
};

inline static
int32
swapBytesInt24(const uint32 i)
{
    return static_cast<int32>(((i & 0x00ff0000) >> 16 |
                              ((i & 0x0000ff00)       |
                              ((i & 0x000000ff) << 16))));
};

inline static
int32
swapBytesInt16(const uint32 i)
{
    return static_cast<int32>(((i & 0x0000ff00) >> 8 |
                              ((i & 0x000000ff) << 8)));
};

#ifdef __clang__
    #pragma clang diagnostic pop
#endif

#ifdef NSOUND_PLATFORM_OS_WINDOWS
    #define LM_UNUSED
#else
    #define LM_UNUSED __attribute__((unused))
#endif

//-----------------------------------------------------------------------------
//! This method reads data from the file and does the proper byte
//! conversion based on the endianess of the platform.
inline static
int64
readInt(FILE * fd, uint32 n_bytes)
{
    int64 temp64 = 0;

    size_t _ LM_UNUSED = fread(reinterpret_cast<char*>(&temp64), n_bytes, 1, fd);

    // Swap the bytes if we are on a big endian platform.
    #ifdef NSOUND_BIG_ENDIAN
        temp64 = swapBytesInt64(temp64);
    #endif

    // Sign extend temp64.
    switch(n_bytes)
    {
        // 48-bit
        case 6:
            if(temp64   & 0x0000800000000000ULL)
            {
                temp64 |= 0xffff000000000000ULL;
            }
            break;

        // 32-bit
        case 4:
            if(temp64   & 0x0000000080000000ULL)
            {
                temp64 |= 0xffffffff00000000ULL;
            }
            break;

        // 24-bit
        case 3:
            if(temp64   & 0x0000000000800000ULL)
            {
                temp64 |= 0xffffffffff000000ULL;
            }
            break;

        // 16-bit
        case 2:
            if(temp64   & 0x0000000000008000ULL)
            {
                temp64 |= 0xffffffffffff0000ULL;
            }
            break;

        // 8 bit wavefiles are stored unsigned (silly that it's different)
        case 1:
            break;
    }

    return temp64;
}

//-----------------------------------------------------------------------------
//! This method reads data from the file and does the proper byte
//! conversion based on the endianess of the platform.
inline static
float32
readFloat32(FILE * fd)
{
    int32 temp32 = 0;
    float32 * f32 = reinterpret_cast<float32 *>(&temp32);

    size_t _ LM_UNUSED = fread(reinterpret_cast<char*>(&temp32), sizeof(float32), 1, fd);

    // Swap the bytes if we are on a big endian platform.
    #ifdef NSOUND_BIG_ENDIAN
        temp32 = swapBytesInt32(temp32);
    #endif

    return *f32;
}

//-----------------------------------------------------------------------------
//! This method reads data from the file and does the proper byte
//! conversion based on the endianess of the platform.
inline static
float64
readFloat64(FILE * fd)
{
    int64 temp64 = 0;
    float64 * f64 = reinterpret_cast<float64 *>(&temp64);

    size_t _ LM_UNUSED = fread(reinterpret_cast<char*>(&temp64), sizeof(float64), 1, fd);

    // Swap the bytes if we are on a big endian platform.
    #ifdef NSOUND_BIG_ENDIAN
        if(sizeof(float64) == 8) temp64 = swapBytesInt64(temp64);
        else if(sizeof(float64) == 4) temp64 = swapBytesInt32(temp64);
    #endif

    return *f64;
}

//-----------------------------------------------------------------------------
//! This method writes data to the file and does the proper byte
//! conversion based on the endianess of the platform.
inline static
void
writeInt(
    FILE * output,
    uint32 n_bytes,
    int64 value)
{
    // Swap the bytes if we are on a big endian platform
    #ifdef NSOUND_BIG_ENDIAN
        value = swapBytesInt64(value);
    #endif

    fwrite(reinterpret_cast<char*>(&value), n_bytes, 1, output);
}

//-----------------------------------------------------------------------------
//! This method writes data to the file and does the proper byte
//! conversion based on the endianess of the platform.
inline static
void
writeFloat32(
    FILE * output,
    float32 value)
{
    // Swap the bytes if we are on a big endian platform
    #ifdef NSOUND_BIG_ENDIAN
        uint32 * v = reinterpret_cast<uint32 *>(&value);
        *v = swapBytesInt32(*v);
    #endif

    fwrite(reinterpret_cast<char*>(&value), 1, sizeof(float32), output);
}

//-----------------------------------------------------------------------------
//! This method writes data to the file and does the proper byte
//! conversion based on the endianess of the platform.
inline static
void
writeFloat64(
    FILE * output,
    float64 value)
{
    // Swap the bytes if we are on a big endian platform
    #ifdef NSOUND_BIG_ENDIAN
        uint64 * v = reinterpret_cast<uint64 *>(&value);

        if(sizeof(float64) == 8) *v = swapBytesInt64(*v);
        else if(sizeof(float64) == 4) *v = swapBytesInt32(*v);
    #endif

    fwrite(reinterpret_cast<char*>(&value), sizeof(float64), 1, output);
}

boolean
Wavefile::
read(
    const std::string & filename,
    BufferPointerVector * b_vector,
    AudioStream * as,
    std::stringstream * ss)
{
    FILE * fd;
    fd = fopen(filename.c_str(), "rb");

    if(fd == NULL)
    {
        M_THROW("Wavefile::read(): unable to open file '" << filename << "'");

        return false;
    }

    uint32 chunk_id = 0;
    uint32 chunk_size = 0;

    // Read in RIFF header
    chunk_id = static_cast<uint32>(readInt(fd,4));
    if(chunk_id != Wavefile::RIFF_)
    {
        fclose(fd);

        M_THROW("Wavefile::read(): '" << filename
            << "', could not read 'RIFF' from file");

        return false;
    }

    // Read in RIFF chunk length
    uint32 riff_chunk_length = static_cast<uint32>(readInt(fd,4));

    // Read in 'WAVE'
    chunk_id = static_cast<uint32>(readInt(fd,4));

    if(chunk_id != Wavefile::WAVE_)
    {
        fclose(fd);

        M_THROW(
            "Wavefile::read(): '" << filename
            << "', could not read 'WAVE' from file");

        return false;
    }

    // Determine end of file position.

    size_t cur_pos = ftell(fd);
    fseek(fd, 0, SEEK_END);
    size_t end_pos = ftell(fd);

    // Seek back to where we were.
    #ifdef NSOUND_PLATFORM_OS_WINDOWS
    	fseek(fd, static_cast<long>(cur_pos), SEEK_SET);
    #else
		fseek(fd, cur_pos, SEEK_SET);
	#endif

    // Read in chunk ids until we find the data chunk, skipping all other chunk
    // ids except for the format chunk.

    uint16 format_tag = 0;
    uint16 channels = 0;
    uint32 sample_rate = 0;
    uint32 average_bytes_per_second = 0;
    uint32 block_alignment = 0;
    uint32 bits_per_sample = 0;

    chunk_id = static_cast<uint32>(readInt(fd, 4));

    while(chunk_id != Wavefile::DATA_)
    {
        chunk_size = static_cast<uint32>(readInt(fd, 4));

        if(chunk_id == Wavefile::FMT_)
        {
            // Save the current postion.
            size_t pos = ftell(fd);

            format_tag               = static_cast<uint16>(readInt(fd, 2));
            channels                 = static_cast<uint16>(readInt(fd, 2));
            sample_rate              = static_cast<uint32>(readInt(fd, 4));
            average_bytes_per_second = static_cast<uint32>(readInt(fd, 4));
            block_alignment          = static_cast<uint32>(readInt(fd, 2));
            bits_per_sample          = static_cast<uint32>(readInt(fd, 2));

            #ifdef NSOUND_PLATFORM_OS_WINDOWS
				fseek(fd, static_cast<long>(pos + static_cast<size_t>(chunk_size)), SEEK_SET);
			#else
				fseek(fd, pos + static_cast<size_t>(chunk_size), SEEK_SET);
			#endif
        }
        else
        {
            // Seek over chunk.
            fseek(fd, chunk_size, SEEK_CUR);
        }

        // Read in next chunk id
        chunk_id = static_cast<uint32>(readInt(fd, 4));

        cur_pos = ftell(fd);

        if(cur_pos >= end_pos)
        {
            fclose(fd);

            M_THROW("Wavefile::read(): '"
                 << filename
                 << "', reached end of file before finding the "
                 << "'data' chunk");

            return false;
        }
    }

    // Data length
    uint32 data_length = static_cast<uint32>(readInt(fd, 4));

    // Determine data_length by using fseek.
    cur_pos = ftell(fd);

    if(data_length > (end_pos - cur_pos))
    {
        cerr << WARNING_HEADER
             << "the data chunk size indicates the file is truncated!" << endl
             << "    data_chunk_size  = " << data_length << endl
             << "    actual data size = " << (end_pos - cur_pos) << endl;
    }

    boolean fmt_found = true;

    if(data_length == 0)
    {
        M_THROW("Wavefile::read(): '"
             << filename
             << "', data length is zero!");

        fmt_found = false;
    }

    if(channels == 0)
    {
        M_THROW("Wavefile::read(): '"
             << filename
             << "', channels is zero!");

        fmt_found = false;
    }

    if(bits_per_sample == 0)
    {
        M_THROW("Wavefile::read(): '"
             << filename
             << "', bits_per_sample is zero!");

        fmt_found = false;
    }

    uint32 n_samples = 0;

    if(fmt_found)
    {
        n_samples = data_length / channels / (bits_per_sample / 8);
    }

    if(ss != NULL)
    {
        *ss << "'"
            << filename
            << "'"
            << endl

            << "['RIFF']               = ['RIFF']"
            << endl

            << "[riff_chunk_length]    = ["
            << riff_chunk_length
            << "]"
            << endl

            << "['WAVE']               = ['WAVE']"
            << endl

            << "['fmt ']               = ['fmt ']"
            << endl

            << "[format_tag]           = ["
            << decodeFormatTag(format_tag)
            << " ("
            << format_tag
            << ")]"
            << endl

            << "[channels]             = ["
            << channels
            << "]"
            << endl

            << "[sample_rate]          = ["
            << sample_rate
            << "]"
            << endl

            << "[bytes_per_second]     = ["
            << average_bytes_per_second
            << "]"
            << endl

            << "[block_alignment]      = ["
            << block_alignment
            << "]"
            << endl

            << "[bits_per_sample]      = ["
            << bits_per_sample
            << "]"
            << endl

            << "['data']               = ['data']"
            << endl

            << "[data_length_bytes]    = ["
            << data_length
            << "]"
            << endl

            << "[data_length_samples]  = [";

        if(fmt_found)
        {
            *ss << n_samples
                << "]"
                << endl
                << "[data_length_seconds]  = ["
                <<  (static_cast<float32>(n_samples) /
                     static_cast<float32>(sample_rate))
                << "]"
                << endl;
        }
        else
        {
            *ss << "unknown!"
                << "]"
                << endl
                << "[data_length_seconds]  = [unknown!]"
                << endl;
        }
    }

    if(!fmt_found)
    {
        fclose(fd);
        return false;
    }

    // Check if the format is PCM or IEEE Float
    if( format_tag != Wavefile::WAVE_FORMAT_PCM_ &&
        format_tag != Wavefile::WAVE_FORMAT_IEEE_FLOAT_)
    {
        fclose(fd);

        M_THROW("Wavefile::read(): '"
             << filename
             << "', Nsound currently only supports PCM and IEEE Floating "
             << "Point formats, not '"
             << decodeFormatTag(format_tag)
             << "'");

        return false;
    }

    ///////////////////////////////////////////////////////////////////////
    // set data sizes
    uint32 n_bytes = 0;
//    raw_uint64 max_sample;
    raw_float64 data_scale = static_cast<float64>(0.0);
    switch(bits_per_sample)
    {
        case 64:
        {
            n_bytes = 8;
            data_scale = Wavefile::SIGNED_64_BIT_;
//            max_sample = Wavefile::UNSIGNED_64_BIT_;
            break;
        }
        case 48:
        {
            n_bytes = 6;
            data_scale = Wavefile::SIGNED_48_BIT_;
//            max_sample = Wavefile::UNSIGNED_48_BIT_;
            break;
        }
        case 32:
        {
            n_bytes = 4;
            data_scale = Wavefile::SIGNED_32_BIT_;
//            max_sample = Wavefile::UNSIGNED_32_BIT_;
            break;
        }
        case 24:
        {
            n_bytes = 3;
            data_scale = Wavefile::SIGNED_24_BIT_;
//            max_sample = Wavefile::UNSIGNED_24_BIT_;
            break;
        }
        case 16:
        {
            n_bytes = 2;
            data_scale = Wavefile::SIGNED_16_BIT_;
//            max_sample = Wavefile::UNSIGNED_16_BIT_;
            break;
        }
        case 8:
        {
            n_bytes = 1;
            data_scale = Wavefile::SIGNED_8_BIT_;
//            max_sample = Wavefile::UNSIGNED_8_BIT_;
            break;
        }
        default:
            fclose(fd);

            M_THROW("Wavefile::read(): bits_per_sample = "
                 << bits_per_sample);

            return false;
    }

    ///////////////////////////////////////////////////////////////////////
    // Read the pulse code modulation (PCM) data.

    // If we are only doing a readHeader.
    if(ss != NULL)
    {
        // Only read and print out 10 samples per channel.
        if(n_samples > 10)
        {
            n_samples = 10;
        }

        BufferVector bv;

        // Init BufferVector.
        for(uint32 ch = 0; ch < channels; ++ch)
        {
            bv.push_back(Buffer(10));

            bool format_mismatch = false;

            // Read the audio samples without scaling them.
            for(uint32 i = 0; i < n_samples; ++i)
            {
                if(format_tag == WAVE_FORMAT_PCM_)
                {
                    bv[ch] << static_cast<float64>(readInt(fd,n_bytes));
                }
                else if(format_tag == WAVE_FORMAT_IEEE_FLOAT_)
                {
                    if(bits_per_sample == 32)
                    {
                        bv[ch] << static_cast<float64>(readFloat32(fd));
                    }
                    else if(bits_per_sample == 64)
                    {
                        bv[ch] << readFloat64(fd);
                    }
                    else
                    {
                    	format_mismatch = true;
                    }
                }
            }

            if(format_mismatch)
            {
				(*ss)
					<< "Wavefile::read(\""
					<< filename
					<< "\"): format is "
					<< "IEEE Float but bits_per_sample = "
					<< bits_per_sample;
				return false;
			}

            // Print out the results.
            (*ss) << "channel[" << ch << "] = ";

            for(uint32 i = 0; i < n_samples; ++i)
            {
                (*ss) << bv[ch][i];
                (*ss) << " ";
            }

            (*ss) << endl;
        }
    }

    // If we are really reading in the data.
    else if(b_vector != NULL)
    {
        int64 sample;
        for(uint32 i = 0; i < n_samples; ++i)
        {
            // Two cases.
            //
            // Case 1: there are less channels in the file than Buffers in
            //         b_vector
            //
            // Case 2: there are greater than or equal number of channels in
            //         the file as there are Buffers in b_vector

            uint32 channels_to_store = static_cast<uint32>(b_vector->size());

            if(channels < channels_to_store)
            {
                channels_to_store = channels;
            }

            // Read data into buffers stored in b_vector.
            for(uint32 ch = 0; ch < channels_to_store; ++ch)
            {
                if(format_tag == WAVE_FORMAT_PCM_)
                {
                    sample = readInt(fd,n_bytes);

                    if(bits_per_sample == 8)
                    {
                        sample -= 127;
                    }

                    *((*b_vector)[ch]) << static_cast<float64>(sample)
                                        / data_scale;
                }
                else if(format_tag == WAVE_FORMAT_IEEE_FLOAT_)
                {
                    if(bits_per_sample == 32)
                    {
                        *((*b_vector)[ch]) << static_cast<float64>(
                            readFloat32(fd));
                    }
                    else if(bits_per_sample == 64)
                    {
                        *((*b_vector)[ch]) << readFloat64(fd);
                    }
                    else
                    {
                        M_THROW("Wavefile::read(): format is "
                            << "IEEE Float but bits_per_sample = "
                            << bits_per_sample);
                    }
                }
            }

            // Skip data in other channels.
            for(uint32 ch = channels_to_store; ch < channels; ++ch)
            {
                readInt(fd, n_bytes);
            }
        }
    }
    else if(as != NULL)
    {
        int64 sample;
        as->setSampleRate(sample_rate);
        as->setNChannels(channels);

        for(uint32 i = 0; i < n_samples; ++i)
        {
            // Read data into buffers stored in b_vector.
            for(uint32 ch = 0; ch < channels; ++ch)
            {
                if(format_tag == WAVE_FORMAT_PCM_)
                {
                    sample = readInt(fd,n_bytes);

                    if(bits_per_sample == 8)
                    {
                        sample -= 127;
                    }

                    (*as)[ch] << static_cast<float64>(sample) / data_scale;
                }
                else if(format_tag == WAVE_FORMAT_IEEE_FLOAT_)
                {
                    if(bits_per_sample == 32)
                    {
                        (*as)[ch] << static_cast<float64>(readFloat32(fd));
                    }
                    else if(bits_per_sample == 64)
                    {
                        (*as)[ch] << readFloat64(fd);
                    }
                    else
                    {
                        M_THROW("Wavefile::read(): format is "
                            << "IEEE Float but bits_per_sample = "
                            << bits_per_sample);
                    }
                }
            }
        }
    }
    else
    {
        fclose(fd);
        M_THROW("Wavefile::read(): unhandled case!");
        return false;
    }

    fclose(fd);

    return true;
}

boolean
Wavefile::
read(const std::string & filename, AudioStream & as)
{
    boolean was_successful = read(filename, NULL, &as, NULL);

    return was_successful;
}

//-----------------------------------------------------------------------------
// readHeader(string filename)
//
// This method opens the wavefile specified by filename and prints the
// contents of the RIFF header and the first 5 samples.
boolean
Wavefile::
readHeader(
    const std::string & filename,
    std::string & info)
{
    std::stringstream ss;
    boolean success = Wavefile::read(filename, NULL, NULL, &ss);
    info = ss.str();
    return success;
}

///////////////////////////////////////////////////////////////////////////
// write(string filename, AudioStream as)
//
// This method writes pulse code modulation (PCM) to the file
// specified by filename.  If filename exists already, it will be
// overwritten.
boolean
Wavefile::
write(const string & filename,
      const AudioStream & as,
      uint32 bits_per_sample)
{
    if( bits_per_sample !=  8 &&
        bits_per_sample != 16 &&
        bits_per_sample != 24 &&
        bits_per_sample != 32 &&
        bits_per_sample != 48 &&
        bits_per_sample != 64)
    {
        M_THROW("Wavefile::write(): "
            << "bits per sample must be 8, 16, 24, 32, 48, 64");
        return false;
    }

    // open the file
    FILE * output;
    output = fopen(filename.c_str(), "wb");

    if(output == NULL)
    {
        M_THROW("Wavefile::write(): "
            << "unable to open file '"
            << filename);
        return false;
    }

    // verify channels_
    uint32 channels = as.getNChannels();

    uint32 n_samples = as.getLength();

    // The data length is the total number of bytes of the raw sound
    // data.  There are 8 bits per byte.
    uint32 data_length = n_samples * channels * (bits_per_sample / 8);

    // riff_chunk_lenth is always 36 bytes larger than the data length,
    // the 36 bytes is added to include all the header information for
    // the riff wavefile.  Take a look at Wavefile.hxx to see a block
    // diagram of the header format.
    uint32 riff_chunk_length = data_length + 36;

    // calculate block_alignemnt
    uint32 block_alignment = channels * (bits_per_sample / 8);

    // calculate bytes_per_second
    uint32 sample_rate = static_cast<uint32>(as.getSampleRate());
    uint32 bytes_per_second = sample_rate * block_alignment;

    uint32 format_tag = Wavefile::default_wave_format_;

    ///////////////////////////////////////////////////////////////////
    //  Write out the data

    writeInt(output,4,Wavefile::RIFF_);
    writeInt(output,4,riff_chunk_length);
    writeInt(output,4,Wavefile::WAVE_);
    writeInt(output,4,Wavefile::FMT_);
    writeInt(output,4,16); // format_chunk_length = 16
    writeInt(output,2,format_tag);
    writeInt(output,2,channels);
    writeInt(output,4,sample_rate);
    writeInt(output,4,bytes_per_second);
    writeInt(output,2,block_alignment);
    writeInt(output,2,bits_per_sample);

    if(format_tag == WAVE_FORMAT_IEEE_FLOAT_)
    {
        writeInt(output,4,Wavefile::FACT_);
        writeInt(output,4, 4);
        writeInt(output,4, n_samples);

        writeInt(output,4,Wavefile::PEAK_);
        writeInt(output,4, 16);
        writeInt(output,4, 0);
        writeInt(output,4, 0);
        writeInt(output,4, 0);
        writeInt(output,4, 0);
    }

    writeInt(output,4,Wavefile::DATA_);
    writeInt(output,4,data_length);

    ///////////////////////////////////////////////////////////////////////
    // set data sizes
    uint32 n_bytes = 0;
    float64 data_scale = static_cast<float64>(0.0);
    switch(bits_per_sample)
    {
        case 64:
        {
            n_bytes = 8;
            data_scale = Wavefile::SIGNED_64_BIT_;
            break;
        }
        case 48:
        {
            n_bytes = 6;
            data_scale = Wavefile::SIGNED_48_BIT_;
            break;
        }
        case 32:
        {
            n_bytes = 4;
            data_scale = Wavefile::SIGNED_32_BIT_;
            break;
        }
        case 24:
        {
            n_bytes = 3;
            data_scale = Wavefile::SIGNED_24_BIT_;
            break;
        }
        case 16:
        {
            n_bytes = 2;
            data_scale = Wavefile::SIGNED_16_BIT_;
            break;
        }
        case 8:
        {
            n_bytes = 1;
            data_scale = Wavefile::SIGNED_8_BIT_;
            break;
        }
    }

    // write the data
    int64 scaled = 0;
    int64 positive_data_scale = static_cast<int64>(data_scale);
    int64 negitive_data_scale = static_cast<int64>(-1.0 * data_scale);

    for(uint32 i = 0; i < n_samples; i++)
    {
        for(uint32 ch = 0; ch < channels; ++ch)
        {
            if(format_tag == WAVE_FORMAT_PCM_)
            {
                scaled = static_cast<int64>(as[ch][i] * data_scale);

                if(scaled > positive_data_scale)
                {
                    scaled = positive_data_scale;
                }
                else if(scaled < negitive_data_scale)
                {
                    scaled = negitive_data_scale;
                }
                writeInt(output,n_bytes,scaled);
            }
            else if(format_tag == WAVE_FORMAT_IEEE_FLOAT_)
            {
                if(bits_per_sample == 32)
                {
                    writeFloat32(output, static_cast<float32>(as[ch][i]));
                }
                else if(bits_per_sample == 64)
                {
                    writeFloat64(output, as[ch][i]);
                }
                else
                {
                    M_THROW(
                    	"Wavefile::read(\""
                    	<< filename
                    	<< "\"): format is "
                        << "IEEE Float but bits_per_sample = "
                        << bits_per_sample);
                }
            }
        }
    }

    fclose(output);
    return true;
}

boolean
Wavefile::
write(
    const string & filename,
    const Buffer & buffer,
    uint32 bits_per_sample,
    uint32 sample_rate)
{
    if( bits_per_sample !=  8 &&
        bits_per_sample != 16 &&
        bits_per_sample != 24 &&
        bits_per_sample != 32 &&
        bits_per_sample != 48 &&
        bits_per_sample != 64)
    {
        M_THROW("Wavefile::write(): "
             << "bits per sample must be 8, 16, 24, 32, 64");
        return false;
    }

    // open the file

    FILE * output;
    output = fopen(filename.c_str(), "wb");
    if(output == NULL)
    {
        M_THROW("Wavefile::write(): "
             << "unable to open file '"
             << filename);
        return false;
    }

    // verify channels_
    uint32 channels = 1;

    uint32 n_samples = buffer.getLength();

    // The data length is the total number of bytes of the raw sound
    // data.  There are 8 bits per byte.
    uint32 data_length = n_samples * channels * (bits_per_sample / 8);

    // riff_chunk_lenth is always 36 bytes larger than the data length,
    // the 36 bytes is added to include all the header information for
    // the riff wavefile.  Take a look at Wavefile.hxx to see a block
    // diagram of the header format.
    uint32 riff_chunk_length = data_length + 36;

    // calculate block_alignemnt
    uint32 block_alignment = channels * (bits_per_sample / 8);

    // calculate bytes_per_second
    uint32 bytes_per_second = sample_rate * block_alignment;

    uint32 format_tag = Wavefile::default_wave_format_;

    ///////////////////////////////////////////////////////////////////
    //  Write out the data

    writeInt(output,4,Wavefile::RIFF_);
    writeInt(output,4,riff_chunk_length);
    writeInt(output,4,Wavefile::WAVE_);
    writeInt(output,4,Wavefile::FMT_);
    writeInt(output,4,16); // format_chunk_length = 16
    writeInt(output,2,format_tag);
    writeInt(output,2,channels);
    writeInt(output,4,sample_rate);
    writeInt(output,4,bytes_per_second);
    writeInt(output,2,block_alignment);
    writeInt(output,2,bits_per_sample);

    if(format_tag == WAVE_FORMAT_IEEE_FLOAT_)
    {
        writeInt(output,4,Wavefile::FACT_);
        writeInt(output,4, 4);
        writeInt(output,4, n_samples);

        writeInt(output,4,Wavefile::PEAK_);
        writeInt(output,4, 16);
        writeInt(output,4, 0);
        writeInt(output,4, 0);
        writeInt(output,4, 0);
        writeInt(output,4, 0);
    }

    writeInt(output,4,Wavefile::DATA_);
    writeInt(output,4,data_length);

    ///////////////////////////////////////////////////////////////////////
    // set data sizes
    uint32 n_bytes = 0;
    float64 data_scale = static_cast<float64>(0.0);
    switch(bits_per_sample)
    {
        case 64:
        {
            n_bytes = 8;
            data_scale = Wavefile::SIGNED_64_BIT_;
            break;
        }
        case 48:
        {
            n_bytes = 6;
            data_scale = Wavefile::SIGNED_48_BIT_;
            break;
        }
        case 32:
        {
            n_bytes = 4;
            data_scale = Wavefile::SIGNED_32_BIT_;
            break;
        }
        case 24:
        {
            n_bytes = 3;
            data_scale = Wavefile::SIGNED_24_BIT_;
            break;
        }
        case 16:
        {
            n_bytes = 2;
            data_scale = Wavefile::SIGNED_16_BIT_;
            break;
        }
        case 8:
        {
            n_bytes = 1;
            data_scale = Wavefile::SIGNED_8_BIT_;
            break;
        }
    }

    ///////////////////////////////////////////////////////////////////////
    // write the data
    int64 scaled;
    int64 positive_data_scale = static_cast<int64>(data_scale);
    int64 negitive_data_scale = static_cast<int64>(-1.0 * data_scale);

    for(uint32 i = 0; i < n_samples; ++i)
    {
            if(format_tag == WAVE_FORMAT_PCM_)
            {
                scaled = static_cast<int64>(buffer[i] * data_scale);

                if(scaled > positive_data_scale)
                {
                    scaled = positive_data_scale;
                }
                else if(scaled < negitive_data_scale)
                {
                    scaled = negitive_data_scale;
                }
                writeInt(output,n_bytes,scaled);
            }
            else if(format_tag == WAVE_FORMAT_IEEE_FLOAT_)
            {
                if(bits_per_sample == 32)
                {
                    writeFloat32(output, static_cast<float32>(buffer[i]));
                }
                else if(bits_per_sample == 64)
                {
                    writeFloat64(output, buffer[i]);
                }
                else
                {
                    M_THROW("Wavefile::read(): format is "
                        << "IEEE Float but bits_per_sample = "
                        << bits_per_sample);
                }
            }
    }

    fclose(output);
    return true;
}

//-----------------------------------------------------------------------------
std::string
Wavefile::
decodeFormatTag(const uint16 format_tag)
{
    switch(format_tag)
    {
        case 0x0000: return "Unknown";
        case 0x0001: return "Microsoft PCM";
        case 0x0002: return "Microsoft ADPCM";
        case 0x0003: return "IEEE Float";
        case 0x0004: return "Compaq VSELP";
        case 0x0005: return "IBM CVSD";
        case 0x0006: return "Microsoft ALAW";
        case 0x0007: return "Microsoft MULAW";
        case 0x000A: return "Microsoft Windows Media Audio Speech";
        case 0x0010: return "OKI ADPCM";
        case 0x0011: return "Intel DVI ADPCM";
        case 0x0012: return "Videologic MediaSpace ADPCM";
        case 0x0013: return "Sierra ADPCM";
        case 0x0014: return "Antex Electronics G.723 ADPCM";
        case 0x0015: return "DSP Solution DIGISTD";
        case 0x0016: return "DSP Solution DIGIFIX";
        case 0x0017: return "Dialogic OKI ADPCM";
        case 0x0018: return "MediaVision ADPCM";
        case 0x0019: return "HP CU";
        case 0x0020: return "Yamaha ADPCM";
        case 0x0021: return "Speech Compression Sonarc";
        case 0x0022: return "DSP Group True Speech";
        case 0x0023: return "Echo Speech EchoSC1";
        case 0x0024: return "Audiofile AF36";
        case 0x0025: return "APTX";
        case 0x0026: return "AudioFile AF10";
        case 0x0027: return "Prosody 1612";
        case 0x0028: return "LRC";
        case 0x0030: return "Dolby AC2";
        case 0x0031: return "Microsoft GSM610";
        case 0x0032: return "Microsoft MSNAudio";
        case 0x0033: return "Antex ADPCME";
        case 0x0034: return "Control Res VQLPC";
        case 0x0035: return "Digireal";
        case 0x0036: return "DigiADPCM AC2";
        case 0x0037: return "Control Res CR10";
        case 0x0038: return "NMS VBXADPCM AC2";
        case 0x0039: return "Roland RDAC";
        case 0x003A: return "EchoSC3";
        case 0x003B: return "Rockwell ADPCM";
        case 0x003C: return "Rockwell Digit LK";
        case 0x003D: return "Xebec";
        case 0x0040: return "Antex Electronics G.721";
        case 0x0041: return "Antex Electronics G.728 CELP";
        case 0x0042: return "Microsoft MSG723";
        case 0x0050: return "MPEG";
        case 0x0052: return "Voxware RT24";
        case 0x0053: return "InSoft PAC";
        case 0x0055: return "MPEG Layer 3";
        case 0x0057: return "AMR NB";
        case 0x0058: return "AMR WB";
        case 0x0059: return "Lucent G.723";
        case 0x0060: return "Cirrus";
        case 0x0061: return "ESPCM";
        case 0x0062: return "Voxware";
        case 0x0063: return "Canopus Atrac";
        case 0x0064: return "APICOM G.726 ADPCM";
        case 0x0065: return "APICOM G.722 ADPCM";
        case 0x0066: return "Microsoft DSAT";
        case 0x0067: return "Microsoft DSAT Display";
        case 0x0069: return "Voxware Byte Aligned";
        case 0x0070: return "Voxware AC8";
        case 0x0071: return "Voxware AC10";
        case 0x0072: return "Voxware AC16";
        case 0x0073: return "Voxware AC20";
        case 0x0074: return "Voxware Metavoice";
        case 0x0075: return "Voxware Metasound";
        case 0x0076: return "Voxware RT29HW";
        case 0x0077: return "Voxware VR12";
        case 0x0078: return "Voxware VR18";
        case 0x0079: return "Voxware TQ40";
        case 0x0080: return "Softsound";
        case 0x0081: return "Voxware TQ60";
        case 0x0082: return "MSRT24";
        case 0x0083: return "AT&T G.729A";
        case 0x0084: return "Motion Pixels MVI MV12";
        case 0x0085: return "DF G.726";
        case 0x0086: return "DF GSM610";
        case 0x0088: return "ISIAudio";
        case 0x0089: return "Onlive";
        case 0x0091: return "Siemens SBC24";
        case 0x0092: return "Dolby AC3 SPDIF";
        case 0x0093: return "Mediasonic G723";
        case 0x0094: return "Prosody 8KBPS";
        case 0x0097: return "ZyXEL ADPCM";
        case 0x0098: return "Philips LPCBB";
        case 0x0099: return "Packed";
        case 0x00A0: return "Malden PhonyTalk";
        case 0x00FF: return "AAC";
        case 0x0100: return "Rhetorex ADPCM";
        case 0x0101: return "IBM MULAW";
        case 0x0102: return "IBM ALAW";
        case 0x0103: return "IBM ADPCM";
        case 0x0111: return "Vivo G.723";
        case 0x0112: return "Vivo Siren";
        case 0x0123: return "DEC G.723";
        case 0x0125: return "Sanyo LD ADPCM";
        case 0x0130: return "Siprolab ACEPLNET";
        case 0x0131: return "Siprolab ACELP4800";
        case 0x0132: return "Siprolab ACELP8V3";
        case 0x0133: return "Siprolab G729";
        case 0x0134: return "Siprolab G729A";
        case 0x0135: return "Siprolab Kelvin";
        case 0x0140: return "G726 ADPCM";
        case 0x0150: return "Qualcomm Purevoice";
        case 0x0151: return "Qualcomm Halfrate";
        case 0x0155: return "Tub GSM";
        case 0x0160: return "WMAV1";
        case 0x0161: return "WMAV2";
        case 0x0162: return "WMAV3";
        case 0x0163: return "WMAV3 L";
        case 0x0200: return "Creative ADPCM";
        case 0x0202: return "Creative FastSpeech8";
        case 0x0203: return "Creative FastSpeech10";
        case 0x0210: return "UHER ADPCM";
        case 0x0220: return "Quarterdeck";
        case 0x0230: return "iLink VC";
        case 0x0240: return "Raw Sport";
        case 0x0250: return "IPI HSX";
        case 0x0251: return "IPI RPELP";
        case 0x0260: return "CS2";
        case 0x0270: return "Sony ATRAC3";
        case 0x028E: return "Siren";
        case 0x0300: return "Fujitsu FM Towns Snd";
        case 0x0400: return "BTV Digital";
        case 0x0401: return "IMC";
        case 0x0450: return "QDesign Music";
        case 0x0680: return "AT&T VME VMPCM";
        case 0x0681: return "TCP";
        case 0x1000: return "Olivetti OLIGSM";
        case 0x1001: return "Olivetti OLIADPCM";
        case 0x1002: return "Olivetti OLICELP";
        case 0x1003: return "Olivetti OLISBC";
        case 0x1004: return "Olivetti OLIOPR";
        case 0x1100: return "LH Codec";
        case 0x1400: return "Norris";
        case 0x1401: return "AT&T ISIAudio";
        case 0x1500: return "AT&T Soundspace Music Compression";
        case 0x2000: return "DVM";
        case 0x2001: return "DTS";
        case 0x2048: return "Sonic";
        case 0x4143: return "AAC AC";
        case 0x674f: return "Vorbis 1";
        case 0x6750: return "Vorbis 2";
        case 0x6751: return "Vorbis 3";
        case 0x676f: return "Vorbis 1+";
        case 0x6770: return "Vorbis 2+";
        case 0x6771: return "Vorbis 3+";
        case 0x706d: return "AAC PM";
        case 0x7A21: return "GSM AMR CBR";
        case 0x7A22: return "GSM AMR VBR";
        case 0xF1AC: return "FLAC";
        case 0xFFFE: return "WAVE_FORMAT_EXTENSIBLE";
        case 0xFFFF: return "Experimental";
    }

    return "UNKOWN";
}

//-----------------------------------------------------------------------------
Buffer &
Nsound::
operator<<(Buffer & lhs, const char * rhs)
{
    BufferPointerVector b_vector;

    b_vector.push_back(&lhs);

    Wavefile::read(rhs,&b_vector, NULL, NULL);

    return lhs;
}

void
Nsound::
operator>>(const Buffer & lhs, const char * rhs)
{
    Wavefile::write(
        rhs,
        lhs,
        Wavefile::getDefaultSampleSize(),
        Wavefile::getDefaultSampleRate());
}

//-----------------------------------------------------------------------------
AudioStream &
Nsound::
operator<<(AudioStream & lhs, const char * rhs)
{
    Wavefile::read(rhs, NULL, &lhs, NULL);

    return lhs;
}

void
Nsound::
operator>>(const AudioStream & lhs, const char * rhs)
{
    Wavefile::write(
        rhs,
        lhs,
        Wavefile::getDefaultSampleSize());
}

//-----------------------------------------------------------------------------
typedef struct RawTag
{
    char header[3];
    char title[30];
    char artist[30];
    char album[30];
    char year[4];
    char comment[30];
    char genre;

} RawTag;

//-----------------------------------------------------------------------------
ID3v1Tag::
ID3v1Tag(const std::string & filename, boolean show_warnings)
    : title(),
      artist(),
      album(),
      year(),
      comment(),
      genre(' ')
{
    if(filename.size() > 0)
    {
        this->read(filename,show_warnings);
    }
}

//-----------------------------------------------------------------------------
// searches the file for ID3v1 'TAG', if it finds it, return the file position
// where it is located, -1 if not found, -2 for all other errors.

#define NOT_FOUND -1
#define IO_ERROR -2

int64
findID3v1Tag(FILE * fin)
{
    uint32 chunk_id = 0;
    uint32 header_limit = 72 ;  // The 'data' should be near the begging
                                // of the file if it is a WAVE file.

    size_t data_length = 0;

    uint32 count = 0;

    M_CHECK_PTR(fin);
    if(fin == NULL) return IO_ERROR;

    fseek(fin, 0, SEEK_SET);

    // Verify first tag is RIFF
    chunk_id = static_cast<uint32>(readInt(fin, 4));
    count += 4;

    if(chunk_id != Wavefile::RIFF_)
    {
        M_THROW("ID3v1Tag::find(): could not read 'RIFF' from file");
        return IO_ERROR;
    }

    // Read RIFF chunk length
    chunk_id = static_cast<uint32>(readInt(fin, 4));
    count += 4;

    // Read in 'WAVE'
    chunk_id = static_cast<uint32>(readInt(fin, 4));
    count += 4;

    if(chunk_id != Wavefile::WAVE_)
    {
        M_THROW("ID3v1Tag::find(): could not read 'WAVE' from file");
        return IO_ERROR;
    }

    // Okay, we now know we are reading a RIFF WAVE file.

    // Search for the 'data' tag.
    while(count < header_limit)
    {
        chunk_id = static_cast<uint32>(readInt(fin, 4));

        count += 4;

        if(chunk_id == Wavefile::DATA_)
        {
            data_length = static_cast<uint32>(readInt(fin,4));
            break;
        }
    }

    if(count >= header_limit)
    {
        cerr << "*** Error --> ID3v1Tag::find(): did not find the 'data'"
             << endl;
        return IO_ERROR;
    }

    size_t cur_pos = ftell(fin);
    fseek(fin, 0, SEEK_END);
    size_t end_pos = ftell(fin);

    // Seek to the end of the data
	#ifdef NSOUND_PLATFORM_OS_WINDOWS
		fseek(fin, static_cast<long>(cur_pos), SEEK_SET);
		fseek(fin, static_cast<long>(data_length), SEEK_CUR);
	#else
		fseek(fin, cur_pos, SEEK_SET);
		fseek(fin, data_length, SEEK_CUR);
	#endif

    // Start scanning for 'TAG'
    cur_pos = ftell(fin);
    while(cur_pos < end_pos-2)
    {
        char t[3];

        if(3 != fread(t, 1, 3, fin))
        {
            M_THROW("ID3v1Tag::find(): IO error reading file");
            return IO_ERROR;
        }

        if(t[0] == 'T' && t[1] == 'A' && t[2] == 'G')
        {
            break;
        }

        cur_pos = ftell(fin);
    }

    // No tag.
    if(cur_pos >= end_pos)
    {
        return NOT_FOUND;
    }

    // We have the tag, back up three bytes and return position.
    fseek(fin, -3, SEEK_CUR);

    return ftell(fin);
}

boolean
ID3v1Tag::
read(const std::string & filename, boolean show_warnings)
{
    FILE * fin = fopen(filename.c_str(), "rb");

    if(fin == NULL)
    {
        M_THROW("ID3v1Tag::read(): could not open the file '"
             << filename
             << "'");
        return false;
    }

    int64 tag_pos = static_cast<size_t>(findID3v1Tag(fin));

    if(tag_pos == IO_ERROR)
    {
        fclose(fin);
        M_THROW("ID3v1Tag::read(): ID3v1Tag::find() failed!");
        return false;
    }

    if(tag_pos == NOT_FOUND && show_warnings)
    {
        fclose(fin);
        cerr << "*** Warning --> ID3v1Tag::read(): could not find the ID3v1 'TAG'"
             << endl;
        return false;
    }

    // Seek to the tag.
    #ifdef NSOUND_PLATFORM_OS_WINDOWS
		fseek(fin, static_cast<long>(tag_pos), SEEK_SET);
	#else
		fseek(fin, tag_pos, SEEK_SET);
	#endif

    RawTag tag;

    memset(reinterpret_cast<char *>(&tag), 0, sizeof(RawTag));

    // Read the tag.
    if(sizeof(RawTag) != fread(tag.header, 1, sizeof(RawTag), fin))
    {
        fclose(fin);
        cerr << "*** Error --> ID3v1Tag::read(): failed to read the ID3v1 TAG!"
             << endl;
        return false;
    }

    fclose(fin);

    this->title.clear();
    this->artist.clear();
    this->album.clear();
    this->year.clear();
    this->comment.clear();
    this->genre = ' ';

    // Title
    for(uint32 i = 0; i < 30; ++i)
    {
        if(tag.title[i] != '\0') this->title += tag.title[i];
        else break;
    }

    // Artist
    for(uint32 i = 0; i < 30; ++i)
    {
        if(tag.artist[i] != '\0') this->artist += tag.artist[i];
        else break;
    }

    // Album
    for(uint32 i = 0; i < 30; ++i)
    {
        if(tag.album[i] != '\0') this->album += tag.album[i];
        else break;
    }

    // Year
    for(uint32 i = 0; i < 4; ++i)
    {
        if(tag.year[i] != '\0') this->year += tag.year[i];
        else break;
    }

    // Comment
    for(uint32 i = 0; i < 30; ++i)
    {
        if(tag.comment[i] != '\0') this->comment += tag.comment[i];
        else break;
    }

    // Genre
    this->genre = tag.genre;

    return true;
}

boolean
ID3v1Tag::
write(const std::string & filename, boolean show_warnings)
{
    FILE * fin = fopen(filename.c_str(), "r+b");

    if(fin == NULL)
    {
        M_THROW("ID3v1Tag::write(): could not open the file '"
             << filename
             << "'");
        return false;
    }

    int64 tag_pos = findID3v1Tag(fin);

    if(tag_pos == IO_ERROR)
    {
       M_THROW("ID3v1Tag::write(): ID3v1Tag::find() failed!"
             << filename
             << "'");
        return false;
    }

    // If he tag wasn't found, just go to the end of the file.
    else if(tag_pos == NOT_FOUND)
    {
        fseek(fin, 0, SEEK_END);
    }

    // Seek to the tag.
    else
    {
		#ifdef NSOUND_PLATFORM_OS_WINDOWS
			fseek(fin, static_cast<long>(tag_pos), SEEK_SET);
		#else
			fseek(fin, tag_pos, SEEK_SET);
		#endif
    }

    RawTag tag;

    memset(reinterpret_cast<char *>(&tag), 0, sizeof(RawTag));

    tag.header[0] = 'T';
    tag.header[1] = 'A';
    tag.header[2] = 'G';

    #define MIN(a,b) ((a < b) ? a : b)

    // Title
    for(uint32 i = 0; i < MIN(30, this->title.size()); ++i)
    {
        tag.title[i] = this->title[i];
    }

    // Artist
    for(uint32 i = 0; i < MIN(30, this->artist.size()); ++i)
    {
        tag.artist[i] = this->artist[i];
    }

    // Album
    for(uint32 i = 0; i < MIN(30, this->album.size()); ++i)
    {
        tag.album[i] = this->album[i];
    }

    // Year
    for(uint32 i = 0; i < MIN(4, this->year.size()); ++i)
    {
        tag.year[i] = this->year[i];
    }

    // Comment
    for(uint32 i = 0; i < MIN(30, this->comment.size()); ++i)
    {
        tag.comment[i] = this->comment[i];
    }

    // Genre
    tag.genre = this->genre;

    // Write the tag.
    if(sizeof(RawTag) != fwrite(tag.header, 1, sizeof(RawTag), fin))
    {
        fclose(fin);
        M_THROW("ID3v1Tag::write(): failed to write the ID3v1Tag!");
        return false;
    }

    fclose(fin);
    return true;
}

//-----------------------------------------------------------------------------
std::ostream &
Nsound::
operator<<(std::ostream & out, const ID3v1Tag & rhs)
{
    out << "header  : TAG" << endl
        << "title   : " << rhs.title << endl
        << "artist  : " << rhs.artist << endl
        << "album   : " << rhs.album << endl
        << "year    : " << rhs.year << endl
        << "comment : " << rhs.comment << endl
        << "genre   : " << rhs.genre;

    return out;
}
