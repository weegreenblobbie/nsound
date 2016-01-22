//-----------------------------------------------------------------------------
//
//  $Id: AudioBackend.h 874 2014-09-08 02:21:29Z weegreenblobbie $
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
#ifndef _NSOUND_AUDIO_BACKEND_H_
#define _NSOUND_AUDIO_BACKEND_H_

#include <Nsound/Nsound.h>
#include <Nsound/AudioBackendType.h>

#include <string>

namespace Nsound
{

class AudioPlayback;
class AudioStream;

//-----------------------------------------------------------------------------
class AudioBackend
{
    public:

    enum State
    {
        BACKEND_NOT_INITIALIZED,
        BACKEND_READY,
        BACKEND_ERROR,
    };

    AudioBackend(
        uint32 sample_rate     = 44100,
        uint32 channels        = 1,
        uint32 bits_per_sample = 16)
        :
        sample_rate_(sample_rate),
        channels_(channels),
        bits_per_sample_(bits_per_sample),
        state_(BACKEND_NOT_INITIALIZED){};

    virtual
    ~AudioBackend(){};

    virtual
    AudioBackendType
    getBackendType() = 0;

    uint32
    getBitsPerSample(){return bits_per_sample_;};

    uint32
    getChannels(){return channels_;};

    virtual
    std::string
    getError() = 0;

    virtual
    std::string
    getInfo() = 0;

    uint32
    getSampleRate(){return sample_rate_;};

    State
    getState(){return state_;};

    //! Returns the backend state.
    std::string
    getStateString();

    //! Returns the backend state string.
    static
    std::string
    getStateString(const AudioBackend::State & state);

    virtual
    void
    initialize() = 0;

    virtual
    void
    play(void * data, uint32 n_bytes) = 0;

    virtual
    void
    scanDevices(AudioPlayback & pb, const AudioStream & test_clip) = 0;

    virtual
    void
    setOption(const std::string & key, const std::string & value) = 0;

    virtual
    void
    shutdown() = 0;

    protected:

    uint32 sample_rate_;
    uint32 channels_;
    uint32 bits_per_sample_;
    State state_;
};

} // namespace Nsound

// :mode=c++: jEdit modeline
#endif
