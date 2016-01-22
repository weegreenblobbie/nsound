//-----------------------------------------------------------------------------
//
//  $Id: Wavefile.h 900 2015-06-13 19:01:17Z weegreenblobbie $
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

//-----------------------------------------------------------------------------
//
//    This class reads and writes RIFF wave files with the following format:
//
//    --------------------------------------------------------------------
//  0 |      'R'       |       'I'       |       'F'     |     'F'       |
//    --------------------------------------------------------------------
//  4 |                          RIFF chunk size                         |
//    --------------------------------------------------------------------
//  8 |      'W'       |       'A'       |       'V'     |     'E'       |
//    --------------------------------------------------------------------
//
//    Required format chunk:
//
//    --------------------------------------------------------------------
//  0 |      'f'       |       'm'       |       't'     |     ' '       |
//    --------------------------------------------------------------------
//  4 |                FORMAT Chunk Length (16, 18, 30, 40)              |
//    --------------------------------------------------------------------
//  8 | Format Tag: 1=PCM, 3=IEEE_FLOAT  |  n_channels 1, 2, ...         |
//    --------------------------------------------------------------------
// 12 |                             Sample Rate                          |
//    --------------------------------------------------------------------
// 16 |    Average # of Bytes P/Second (Sample rate*Channels*(Bits/8)    |
//    --------------------------------------------------------------------
// 20 | Block Align ((Bits/8)*Channels)  |   Bits per Sample (8 ... 64)  |
//    --------------------------------------------------------------------
// 24 | optional data if FORMAT Chunk Length is 18 or 40                 |
//    --------------------------------------------------------------------
//
//    Required data chunk, the raw audio data:
//
//    --------------------------------------------------------------------
//  0 |     'd'        |       'a'       |      't'      |     'a'       |
//    --------------------------------------------------------------------
//  4 |                Data Length (actual length of raw data)           |
//    --------------------------------------------------------------------
//  8 |                                                                  |
//    |                                                                  |
//    |                                                                  |
//    |                              raw data                            |
//    |                                                                  |
//    |                                                                  |
//    |                                                                  |
//    ----------------------------------EOF-------------------------------
//
//    Optional 'TAG' chunk (aka ID3v1), all fields are plain ASCII unless
//    otherwise stated.
//
//    ----------------------------------------------------
//  0 |     'T'        |       'A'       |      'G'      |
//    --------------------------------------------------------------------
//  3 |                     Title   - 30 bytes                           |
//    --------------------------------------------------------------------
// 33 |                     Artist  - 30 bytes                           |
//    --------------------------------------------------------------------
// 63 |                     Album   - 30 bytes                           |
//    --------------------------------------------------------------------
// 93 |                     Year    -  4 bytes                           |
//    --------------------------------------------------------------------
// 97 |                   Comment   - 30 bytes                           |
//    --------------------------------------------------------------------
//127 | Genre - 1 byte |
//    ------------------
//
//    Total TAG size is 128 bytes.
//
//    All other RIFF WAVE chunk ids are skipped.
//
//-----------------------------------------------------------------------------
#ifndef _NSOUND_WAVEFILE_H_
#define _NSOUND_WAVEFILE_H_

#include <Nsound/Nsound.h>

namespace Nsound
{

class AudioStream;
class Buffer;

//! Very simple Wavefile reading class.
class Wavefile
{
    public:

    // Constants

    static const uint32 DATA_ = 1635017060;
    static const uint32 FACT_ = 1952670054;
    static const uint32 FMT_  =  544501094;
    static const uint32 PEAK_ = 1262568784;
    static const uint32 RIFF_ = 1179011410;
    static const uint32 WAVE_ = 1163280727;

    static const uint16 WAVE_FORMAT_PCM_        = 0x0001;
    static const uint16 WAVE_FORMAT_IEEE_FLOAT_ = 0x0003;

    // Windows won't allow floats to be initialized in the hearder.
    static const raw_float64 SIGNED_64_BIT_; // = 9223372036854775807.0;
    static const raw_float64 SIGNED_48_BIT_; // = 140737488355327.0;
    static const raw_float64 SIGNED_32_BIT_; // = 2147483647.0;
    static const raw_float64 SIGNED_24_BIT_; // = 8388607.0;
    static const raw_float64 SIGNED_16_BIT_; // = 32767.0;
    static const raw_float64 SIGNED_8_BIT_;  // = 127.0;

    static const raw_uint64 UNSIGNED_64_BIT_ = 18446744073709551615ULL;
    static const raw_uint64 UNSIGNED_48_BIT_ = 281474976710655ULL;
    static const raw_uint64 UNSIGNED_32_BIT_ = 4294967295ULL;
    static const raw_uint64 UNSIGNED_24_BIT_ = 16777215ULL;
    static const raw_uint64 UNSIGNED_16_BIT_ = 65535ULL;
    static const raw_uint64 UNSIGNED_8_BIT_  = 255ULL;

    static
    std::string
    decodeFormatTag(const uint16 format_tag);

    static
    uint32
    getDefaultSampleRate() {return default_sample_rate_;};

    static
    uint32
    getDefaultSampleSize() {return default_sample_size_;};

    static
    void
    setDefaultSampleRate(const int32 rate);

    static
    void
    setDefaultSampleRate(const float64 & rate)
    {setDefaultSampleRate(static_cast<int32>(rate));};

    static
    void
    setDefaultSampleSize(uint32 size);

    static
    void
    setIEEEFloat(boolean flag);

    static
    void
    setDefaults(
        const float64 & sample_rate = 44100.0,
        const float64 & sample_bits = 16.0,
        const boolean & use_ieee_floats = false);

    // read(std::string file_name)
    //
    // This method opens the wavefile specified by file_name and loads
    // the waveform into memory.  This methods returns true if the
    // wavefile was successfully read in.
    //
    static
    boolean
    read(const std::string & fileName, AudioStream & astream);

    //! Reads the basic header information and sets the `info` string.
    //
    //! Reads the basic header information and sets the `info` string.
    static
    boolean
    readHeader(const std::string & filename, std::string & info);

    // write(std::string file_name)
    //
    // This method writes pulse code modulation (PCM) to the file
    // specified by filename.  If file_name exists already, it will be
    // overwritten.
    //
    static
    boolean
    write(const std::string & fileName,
          const AudioStream & as,
          uint32 bits_per_sample = 16);

    // write(std::string file_name)
    //
    // This method writes pulse code modulation (PCM) to the file
    // specified by filename.  If file_name exists already, it will be
    // overwritten.
    //
    static
    boolean
    write(const std::string & fileName,
          const Buffer & as,
          uint32 bits_per_sample,
          uint32 sample_rate);

    #ifndef SWIG
    friend Buffer & operator<<(Buffer & lhs, const char * rhs);
    friend void operator>>(const Buffer & lhs, const char * rhs);
    friend AudioStream & operator<<(AudioStream & lhs, const char * rhs);
    friend void operator>>(const AudioStream & lhs, const char * rhs);
    #endif

    protected:

    static uint32 default_sample_rate_; // = 44100;
    static uint32 default_sample_size_; // = 16;
    static uint16 default_wave_format_; // = WAVE_FORMAT_PCM_;

    static
    boolean
    read(
        const std::string & filename,
        std::vector<Buffer *> * b_vector,
        AudioStream * as,
        std::stringstream * out);

}; // Wavefile

// Must declare friend functions here to give them proper namespace scope.
Buffer & operator<<(Buffer & lhs, const char * rhs);
void operator>>(const Buffer & lhs, const char * rhs);
AudioStream & operator<<(AudioStream & lhs, const char * rhs);
void operator>>(const AudioStream & lhs, const char * rhs);

class ID3v1Tag
{
    public:

    ID3v1Tag(const std::string & filename = "", boolean show_warnings = true);

    //! Returns true if it found the tag, false otherwise.
    boolean
    read(const std::string & filename, boolean show_warnings = true);

    //! Returns true if it successfully wrote the tag to the end of the file, false otherwise.
    boolean
    write(const std::string & filename, boolean show_warnings = true);

    std::string title;
    std::string artist;
    std::string album;
    std::string year;
    std::string comment;
    char genre;

    #ifndef SWIG
        ///////////////////////////////////////////////////////////////////////
        //! Sends the contents of the Buffer to the output stream.
        friend
        std::ostream &
        operator<<(std::ostream & out, const ID3v1Tag & rhs);
    #endif
};

std::ostream &
operator<<(std::ostream & out, const ID3v1Tag & rhs);

}; // Nsound

#endif


//-----------------------------------------------------------------------------
// To get the tag constants I used this C program:
//
//    #include <stdio.h>
//
//    int
//    main(int argc, char ** argv)
//    {
//        const unsigned int N_TAGS = 6;
//
//        char * tags[N_TAGS];
//
//        unsigned int i = 0;
//
//        tags[0] = "RIFF";
//        tags[1] = "WAVE";
//        tags[2] = "fmt ";
//        tags[3] = "data";
//        tags[4] = "PEAK";
//        tags[5] = "fact";
//
//        for(i = 0; i < N_TAGS; ++i)
//        {
//            unsigned int * j = (unsigned int *)(tags[i]);
//
//            printf("%s = %d\n", tags[i], *j);
//        }
//
//        return 0;
//    }
