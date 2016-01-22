//-----------------------------------------------------------------------------
//
//  $Id: AudioPlayback.h 874 2014-09-08 02:21:29Z weegreenblobbie $
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
#ifndef _NSOUND_AUDIO_PLAYBACK_H_
#define _NSOUND_AUDIO_PLAYBACK_H_

#include <Nsound/Nsound.h>
#include <Nsound/AudioBackend.h>
#include <Nsound/AudioBackendType.h>

#include <string>
#include <vector>

namespace Nsound
{

// forward declare
class AudioStream;
class Buffer;

class AudioPlayback
{
    public:

    AudioPlayback(
        const float64 & sample_rate  = 44100.0,
        const uint32 channels        = 1,
        const uint32 bits_per_sample = 16);

    ~AudioPlayback();

    //! Sets the AudioBackendType.
    static
    void
    setBackendType(const AudioBackendType ab);

    //! Gets the AudioBackendType that is currently set.
    static
    AudioBackendType
    getBackendType();

    uint32
    getBitsPerSample();

    void
    setBitsPerSample(uint32 bits_per_sample)
    {bits_per_sample_ = bits_per_sample;};

    uint32
    getChannels();

    void
    setChannels(uint32 channels)
    {channels_ = channels;};

    //! Returns an error string describing any backend error.
    std::string
    getError();

    //! Returns information about the backend driver.
    std::string
    getInfo();

    uint32
    getSampleRate();

    void
    setSampleRate(uint32 sample_rate)
    {sample_rate_ = sample_rate;};

    //! Returns the backend state.
    AudioBackend::State
    getState();

    //! Returns the backend state string.
    std::string
    getStateString();

    //! Returns the backend state string.
    std::string
    getStateString(const AudioBackend::State & state);

    //! Initializes the backend and transitions to the BACKEND_READY state on success.
    void
    initialize();

    //! Plays the AudioStream throuh the backend.
    void
    play(const AudioStream & a);

    //! Plays the Buffer through the backend.
    void
    play(const Buffer & b);

    //! Scans for devices and tries to play a test sound.
    void
    scanDevices();

    //! Sets an options, must be called before initialize().
    void
    setOption(const std::string & key, const std::string & value);

    //! Shuts down the backend.
    void
    shutdown();

    #ifndef SWIG
    friend void operator>>(const AudioStream & lhs, AudioPlayback & rhs);
    friend void operator>>(const Buffer & lhs, AudioPlayback & rhs);
    #endif

    private:

    AudioPlayback(const AudioPlayback & copy)
        :
        sample_rate_(copy.sample_rate_),
        channels_(copy.channels_),
        bits_per_sample_(copy.bits_per_sample_),
        backend_(NULL){};

    AudioPlayback & operator=(const AudioPlayback & rhs){return *this;};

    // Members

    uint32 sample_rate_;
    uint32 channels_;
    uint32 bits_per_sample_;

    AudioBackend * backend_;

    static AudioBackendType backend_type_;
}; // AudioPlayback

// Must delcare friend functions here to give them proper namespace scope.
void operator>>(const AudioStream & lhs, AudioPlayback & rhs);
void operator>>(const Buffer & lhs, AudioPlayback & rhs);

//-----------------------------------------------------------------------------
//! Selects the AudioBackend to use by name.
//
//! \param backend the common name for the backend to use
//!
//! \par Example:
//! \code
//! // C++
//! Nsound::use("portaudio");
//! Nsound::use("ao");
//!
//! // Python
//! Nsound.use("portaudio");
//! Nsound.use("ao");
//! \endcode
void use(const std::string & backend);
void use(const AudioBackendType & type);

//-----------------------------------------------------------------------------
//! Returns a list of the available audio backends by name.
//
//! \par Example:
//! \code
//! // C++
//! Nsound::getBackends();
//!
//! // Python
//! Nsound.getBackends();
//! \endcode
std::vector< std::string > getBackends();

//-----------------------------------------------------------------------------
//! Returns a list of the available audio backends types.
//
//! \par Example:
//! \code
//! // C++
//! Nsound::getBackendsTypes();
//!
//! // Python
//! Nsound.getBackendsTypes();
//! \endcode
std::vector< AudioBackendType > getBackendTypes();

} // namespace Nsound

// :mode=c++: jEdit modeline
#endif
