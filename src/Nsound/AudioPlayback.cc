//-----------------------------------------------------------------------------
//
//  $Id: AudioPlayback.cc 912 2015-07-26 00:50:29Z weegreenblobbie $
//
//  Nsound is a C++ library and Python module for audio synthesis featuring
//  dynamic digital filters. Nsound lets you easily shape waveforms and write
//  to disk or plot them. Nsound aims to be as powerful as Csound but easy to
//  use.
//
//  Copyright (c) 2011-Present Nick Hilton
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

#include <Nsound/Nsound.h>
#include <Nsound/AudioBackend.h>
#include <Nsound/AudioBackendLibao.h>
#include <Nsound/AudioBackendLibportaudio.h>
#include <Nsound/AudioBackendType.h>
#include <Nsound/AudioPlayback.h>
#include <Nsound/AudioStream.h>
#include <Nsound/Buffer.h>
#include <Nsound/GuitarBass.h>

#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>

using namespace Nsound;
using std::cout;
using std::cerr;
using std::endl;

#ifdef NSOUND_LIBPORTAUDIO
    AudioBackendType AudioPlayback::backend_type_ = BACKEND_TYPE_LIBPORTAUDIO;
#elif defined(NSOUND_LIBAO)
    AudioBackendType AudioPlayback::backend_type_ = BACKEND_TYPE_LIBAO;
#else
    AudioBackendType AudioPlayback::backend_type_ = BACKEND_TYPE_NONE;
#endif

// A common function to create the backend pointer, but doesn't call the
// backend initialize() function.
AudioBackend *
allocate_backend(
    const AudioBackendType & type,
    const uint32 sample_rate,
    const uint32 channels,
    const uint32 bits_per_sample)
{
    AudioBackend * backend = NULL;

    switch(type)
    {
        case BACKEND_TYPE_LIBAO:
            #ifdef NSOUND_LIBAO
                backend = new AudioBackendLibao(
                    sample_rate,
                    channels,
                    bits_per_sample);
            #endif
            break;

        case BACKEND_TYPE_LIBPORTAUDIO:
            #ifdef NSOUND_LIBPORTAUDIO
                backend = new AudioBackendLibportaudio(
                    sample_rate,
                    channels,
                    bits_per_sample);
            #endif
            break;

        case BACKEND_TYPE_NONE:
            break;
    }

    return backend;
}

void
AudioPlayback::
setBackendType(const AudioBackendType ab)
{
    backend_type_ = ab;
}

AudioBackendType
AudioPlayback::
getBackendType()
{
    return backend_type_;
}

AudioPlayback::
AudioPlayback(
    const float64 & sample_rate,
    uint32 channels,
    uint32 bits_per_sample)
    :
    sample_rate_(static_cast<uint32>(sample_rate)),
    channels_(channels),
    bits_per_sample_(bits_per_sample),
    backend_(NULL)
{
    backend_ = allocate_backend(
        backend_type_,
        sample_rate_,
        channels_,
        bits_per_sample_);

    if(BACKEND_TYPE_NONE != backend_type_ && backend_ == NULL)
    {
        M_THROW("Nsound::AudioPlayback::AudioPlayback():"
            << ": failed to create AudioBackend!");
    }
};

AudioPlayback::
~AudioPlayback()
{
    if(backend_ != NULL)
    {
        backend_->shutdown();
        delete backend_;
    }
}

uint32
AudioPlayback::
getBitsPerSample()
{
    if(backend_ != NULL)
    {
        return backend_->getBitsPerSample();
    }

    return bits_per_sample_;
}

uint32
AudioPlayback::
getChannels()
{
    if(backend_ != NULL)
    {
        return backend_->getChannels();
    }

    return channels_;
}

std::string
AudioPlayback::
getError()
{
    if(backend_ != NULL)
    {
        return backend_->getError();
    }

    std::stringstream ss;

    ss << "Nsound::AudioPlayback::getError():"
        << __LINE__
        << ": backend not initialized"
        << endl;

    return ss.str();
}

std::string
AudioPlayback::
getInfo()
{
    if(backend_ != NULL)
    {
        return backend_->getInfo();
    }

    std::stringstream ss;

    ss << "Nsound::AudioPlayback::getInfo():"
        << __LINE__
        << ": backend not initialized"
        << endl;

    return ss.str();
}

uint32
AudioPlayback::
getSampleRate()
{
    if(backend_ != NULL)
    {
        return backend_->getSampleRate();
    }

    return sample_rate_;
}

AudioBackend::State
AudioPlayback::
getState()
{
    if(backend_ != NULL)
    {
        return backend_->getState();
    }

    return AudioBackend::BACKEND_NOT_INITIALIZED;
}

std::string
AudioPlayback::
getStateString()
{
    if(backend_ != NULL)
    {
        return backend_->getStateString();
    }

    return AudioBackend::getStateString(AudioBackend::BACKEND_NOT_INITIALIZED);
}

std::string
AudioPlayback::
getStateString(const AudioBackend::State & state)
{
    return AudioBackend::getStateString(state);
}

void
AudioPlayback::
initialize()
{
    if(backend_type_ == BACKEND_TYPE_NONE)
    {
        M_THROW("Nsound::AudioPlayback::initialize():"
            << ": no backend selected or available");
        return;
    }

    // If the backend is currently NULL, recreate it with current settings
    if(backend_ == NULL)
    {
        backend_ = allocate_backend(
            backend_type_,
            sample_rate_,
            channels_,
            bits_per_sample_);
    }

    if(backend_ == NULL)
    {
        M_THROW("Nsound::AudioPlayback::initialize():"
            << ": failed to initialize AudioBackend!");

        return;
    }

    backend_->initialize();

    if(AudioBackend::BACKEND_READY != backend_->getState())
    {
        M_THROW("Nsound::AudioPlayback::initialize():"
            << ": backend faild to initialize\n"
            << "Backend error: "
            << backend_->getError());
    }
}

// Templates, these are declared here not in a header, nobody else needs access
// to these templates, they are also not class members.  I did this to avoid
// including <iostream> in the header and keep the header a little cleaner.

// forward delcare

template <typename T>
void
play_int(
    AudioBackend * backend,
    const AudioStream & a,
    const float64 & scale);

// forward delcare

template <typename T>
void
play_int(
    AudioBackend * backend,
    const Buffer & b,
    const float64 & scale);

template <typename T>
void
play_int(
    AudioBackend * backend,
    const AudioStream & a,
    const float64 & scale)
{
    uint32 n_channels = a.getNChannels();

    // If the AudioStream is mono, just call play() for the buffer method and
    // return.
    if(n_channels == 1)
    {
        play_int<T>(backend, a[0], scale);
        return;
    }

    // If the number of channels in the AudioStream doesn't agree with the
    // number of channels of the backend, print error.
    if(n_channels != backend->getChannels())
    {
        M_THROW("Nsound::AudioPlayback::play():"
            << ": AudioStream channels do not match backend channels ("
            << n_channels
            << " != "
            << backend->getChannels()
            << ")");
        return;
    }

    T * array = NULL;

    uint32 n_samples = a.getLength();

    array = new T [n_samples * n_channels];

    if(array == NULL)
    {
        M_THROW("Nsound::AudioPlayback::play():"
            << ": failed allocate memory");
        return;
    }

    uint32 k = 0;
    for(uint32 i = 0; i < n_samples; ++i)
    {
        for(uint32 j = 0; j < n_channels; ++j)
        {
            array[k] = static_cast<T>(a[j][i] * scale);
            ++k;
        }
    }

    backend->play(
        reinterpret_cast<void *>(array),
        n_samples * n_channels * sizeof(T));

    delete array;
}

template <typename T>
void
play_int(
    AudioBackend * backend,
    const Buffer & b,
    const float64 & scale)
{
    uint32 n_samples = b.getLength();
    uint32 n_channels = backend->getChannels();

    std::vector<T> array;

    array.reserve(n_samples * n_channels);

    uint32 k = 0;
    for(uint32 i = 0; i < n_samples; ++i)
    {
        for(uint32 j = 0; j < n_channels; ++j)
        {
            array.push_back(static_cast<T>(b[i] * scale));
            ++k;
        }
    }

    backend->play(
        reinterpret_cast<void *>(array.data()),
        n_samples * n_channels * sizeof(T));
}


template <typename T>
void
_play(AudioPlayback * pb, AudioBackend ** be, const T & audio)
{
    pb->initialize();

    if(*be == NULL) return;

    uint32 bits = (*be)->getBitsPerSample();

    switch(bits)
    {
        case 16:
            play_int<int16>(*be, audio, 32768.0);
            break;
        case 32:
            play_int<int32>(*be, audio, 2147483648.0);
            break;

        default:
            M_THROW("Nsound::AudioPlayback::play():"
                << ": Support for "
                << bits
                << "-bit playback not yet implemented");
            break;
    }
}

void
AudioPlayback::
play(const AudioStream & a)
{
    _play<AudioStream>(this, &backend_, a);
}

void
AudioPlayback::
play(const Buffer & b)
{
    _play<Buffer>(this, &backend_, b);
}

void
AudioPlayback::
scanDevices()
{
    GuitarBass guitar(sample_rate_);

    AudioStream test_clip(sample_rate_, channels_);

    test_clip << guitar.play();

    initialize();

    std::vector< std::string > names = Nsound::getBackends();
    std::vector< AudioBackendType > types = Nsound::getBackendTypes();

    cout << "Nsound::AudioPlayback::scanDevices(): starting\n";
    cout.flush();

    if(types.size() == 0)
    {
        cout << "No backends available\n";
        cout.flush();
    }

    for(uint32 i = 0; i < types.size(); ++i)
    {
        cout << "Selecting backend '"
             << names[i]
             << "'\n";
        cout.flush();

        AudioBackend * orig = backend_;

        backend_ = allocate_backend(
            types[i],
            sample_rate_,
            channels_,
            bits_per_sample_);

        if(backend_->getState() == AudioBackend::BACKEND_READY)
        {
            backend_->scanDevices(*this, test_clip);
        }
        else
        {
            cout << "Backend '"
                 << names[i]
                 << "' failed to initialize\n"
                 << backend_->getError();
            cout.flush();
        }

        delete backend_;

        backend_ = orig;
    }

      cout << "Nsound::AudioPlayback::scanDevices(): finished\n";
      cout.flush();
}

void
AudioPlayback::
setOption(const std::string & key, const std::string & value)
{
    if(backend_ == NULL)
    {
        M_THROW("Nsound::AudioPlayback::setOption():"
            << ": backend is NULL");
        return;
    }
    else if(AudioBackend::BACKEND_NOT_INITIALIZED != backend_->getState())
    {
        M_THROW("Nsound::AudioPlayback::setOption():"
            << ": backend already initialized");
        return;
    }

    backend_->setOption(key, value);
}

void
AudioPlayback::
shutdown()
{
    if(backend_ != NULL)
    {
        backend_->shutdown();
        delete backend_;
        backend_ = NULL;
    }
}

void
Nsound::
operator>>(const AudioStream & lhs, AudioPlayback & ap)
{
    ap.play(lhs);
}

void
Nsound::
operator>>(const Buffer & lhs, AudioPlayback & ap)
{
    ap.play(lhs);
}

std::string
mylower(const std::string & x)
{
    std::string y = x;
    std::transform(y.begin(), y.end(), y.begin(), ::tolower);
    return y;
}

void
Nsound::
use(const std::string & backend)
{
    std::string be = mylower(backend);

    if(be == "ao" || be == "libao")
    {
        AudioPlayback::setBackendType(BACKEND_TYPE_LIBAO);
    }
    else
    if(be == "portaudio" || be == "libportaudio")
    {
        AudioPlayback::setBackendType(BACKEND_TYPE_LIBPORTAUDIO);
    }
    else
    {
        std::stringstream ss_buffer;
        ss_buffer << "Nsound::use(): "
             << "Unrecognized AudioBackend '"
             << backend
             << "'"
             << endl;

        std::vector< std::string > vec = getBackends();

        ss_buffer << "Available backends are:" << endl;

        for(uint32 i = 0; i < vec.size(); ++i)
        {
            ss_buffer << "    "
                 << vec[i]
                 << endl;
        }

        M_THROW(ss_buffer.str());
    }
}

void
Nsound::
use(const AudioBackendType & type)
{
    switch(type)
    {
        case BACKEND_TYPE_LIBAO:
            use("ao");
            break;

        case BACKEND_TYPE_LIBPORTAUDIO:
            use("portaudio");
            break;

        default:
            M_THROW("Nsound::use(): "
                 << "Unrecognized AudioBackendType "
                 << static_cast<uint32>(type));
    }
}

std::vector< std::string >
Nsound::
getBackends()
{
    std::vector< std::string > vec;

    #if defined(NSOUND_LIBPORTAUDIO)
        vec.push_back("portaudio");
    #endif

    #if defined(NSOUND_LIBAO)
        vec.push_back("ao");
    #endif

    return vec;
}

std::vector< AudioBackendType >
Nsound::
getBackendTypes()
{
    std::vector< AudioBackendType > vec;

    #if defined(NSOUND_LIBPORTAUDIO)
        vec.push_back(BACKEND_TYPE_LIBPORTAUDIO);
    #endif

    #if defined(NSOUND_LIBAO)
        vec.push_back(BACKEND_TYPE_LIBAO);
    #endif

    return vec;
}

// :mode=c++:
