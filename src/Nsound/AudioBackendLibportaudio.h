//-----------------------------------------------------------------------------
//
//  $Id: AudioBackendLibportaudio.h 875 2014-09-27 22:25:13Z weegreenblobbie $
//
//  Nsound is a C++ library and Python module for audio synthesis featuring
//  dynamic digital filters. Nsound lets you easily shape waveforms and write
//  to disk or plot them. Nsound aims to be as powerful as Csound but easy to
//  use.
//
//  Copyright (c) 2011-Present Nick Hilton
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
#ifndef _NSOUND_AUDIO_BACKEND_LIBPORTAUDIO_H_
#define _NSOUND_AUDIO_BACKEND_LIBPORTAUDIO_H_

#include <Nsound/Nsound.h>
#include <Nsound/AudioBackend.h>

#include <sstream>
#include <string>
#include <vector>

// Forward declare
struct PaStreamParameters;
typedef void PaStream;

namespace Nsound
{

//-----------------------------------------------------------------------------
class AudioBackendLibportaudio : public AudioBackend
{
    public:

    AudioBackendLibportaudio(
        uint32 sample_rate     = 44100,
        uint32 channels        = 1,
        uint32 bits_per_sample = 16);

    ~AudioBackendLibportaudio();

    AudioBackendType
    getBackendType();

    std::string
    getError();

    std::string
    getInfo();

    void
    initialize();

    void
    play(void * data, uint32 n_bytes);

    void
    scanDevices(AudioPlayback & pb, const AudioStream & test_clip);

    //! Set Libportaudio options.
    //
    //! Available options:
    //!
    //! <table boarder="1">
    //!   <tr align="left">
    //!     <th style="width:150px">Key</th>
    //!     <th>Value</th>
    //!   </tr>
    //!   <tr align="left">
    //!     <td>"driver"</td>
    //!     <td>
    //!         "alsa", "oss", "asihpi", "jack",
    //!         "coreaudio", "asio", "wasapi",
    //!         "wdm/ks", "directsound", "wmme"
    //!      </td>
    //!   </tr>
    //!   <tr align="left">
    //!     <td>"frames_per_buffer"</td>
    //!     <td><em>integer</em>, default = 64</td>
    //!   </tr>
    //! </table>
    //!
    void
    setOption(const std::string & key, const std::string & value);

    void
    shutdown();

    private:

    // disable copy constructor and op=

    AudioBackendLibportaudio(const AudioBackendLibportaudio & copy)
        :
        AudioBackend(),
        options_(),
        error_buffer_(""),
        out_params_(NULL),
        stream_(NULL),
//~        pos_(0),
        n_frames_per_buffer_(64),
        driver_id_(0){};

    AudioBackendLibportaudio & operator=(const AudioBackendLibportaudio & rhs)
    {return *this;};


    std::vector< std::string > options_;
    std::stringstream error_buffer_;

    PaStreamParameters * out_params_;
    PaStream * stream_;

//~    uint32 pos_;

    uint32 n_frames_per_buffer_;

    uint32 driver_id_;

};

} // namespace Nsound

// :mode=c++: jEdit modeline
#endif
