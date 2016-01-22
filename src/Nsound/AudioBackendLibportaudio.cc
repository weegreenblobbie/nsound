//-----------------------------------------------------------------------------
//
//  $Id: AudioBackendLibportaudio.cc 875 2014-09-27 22:25:13Z weegreenblobbie $
//
//  Copyright (c) 2005-2006 Nick Hilton
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
// Some PortAudio notes:
//
// A "frame" consists of all the samples for a given time step.  If you have
// stereo audio, then frames 1 and 2 will contain:
//     frame 0 [left_sample_0, right_sample0]
//     frame 1 [left_sample_1, right_sample1]
//     ...
//
// The data above might contain 4 samples, but only 2 frames, or 2 samples in
// time.
//
//-----------------------------------------------------------------------------


#include <Nsound/Nsound.h>
#include <Nsound/AudioBackendLibportaudio.h>
#include <Nsound/AudioPlayback.h>

#include <algorithm>
#include <iostream>
#include <string>

#include <time.h>

#ifdef NSOUND_LIBPORTAUDIO
    #include <portaudio.h>
#endif

using namespace Nsound;

using std::cerr;
using std::cout;
using std::endl;

#ifndef NSOUND_PLATFORM_OS_WINDOWS
	#pragma GCC diagnostic ignored "-Wold-style-cast"
#endif

#ifdef NSOUND_PLATFORM_OS_WINDOWS
    extern "C"
    {
        void ___chkstk_ms(){}
        void __chkstk_ms(){}
    }
#endif

//-----------------------------------------------------------------------------
AudioBackendLibportaudio::
AudioBackendLibportaudio(
    uint32 sample_rate,
    uint32 channels,
    uint32 bits_per_sample)
    :
    AudioBackend(sample_rate, channels, bits_per_sample),
    options_(),
    error_buffer_(""),
    out_params_(NULL),
    stream_(NULL),
//~    pos_(0),
    n_frames_per_buffer_(64),
    driver_id_(0)
{
    initialize();
};

//-----------------------------------------------------------------------------
AudioBackendLibportaudio::
~AudioBackendLibportaudio()
{
    if(state_ == BACKEND_READY)
    {
        shutdown();
    }
}

AudioBackendType
AudioBackendLibportaudio::
getBackendType()
{
    return BACKEND_TYPE_LIBPORTAUDIO;
}

//-----------------------------------------------------------------------------
std::string
AudioBackendLibportaudio::
getError()
{
    return error_buffer_.str();
}

//-----------------------------------------------------------------------------
std::string
AudioBackendLibportaudio::
getInfo()
{
    #ifndef NSOUND_LIBPORTAUDIO
        state_ = BACKEND_ERROR;
        M_THROW("Nsound::AudioBackendLibportaudio::getInfo():"
            << ": Nsound was not compiled with libportaudio support.\n");
        return;
    #else
        if(state_ == BACKEND_NOT_INITIALIZED)
        {
            M_THROW("Nsound::AudioBackendLibportaudio::getInfo():"
                << ": backend not initialized yet\n");
            return "";
        }
        else if(state_ == BACKEND_ERROR)
        {
            return getError();
        }

        const PaDeviceInfo * info = NULL;

        info = Pa_GetDeviceInfo(out_params_->device);

        if(info == NULL)
        {
            M_THROW("Nsound::AudioBackendLibportaudio::getInfo():"
                << ": Pa_GetDeviceInfo() failed\n");

            return "";
        }

        std::stringstream ss;

        ss << "libportaudio Default Driver Info:"                    << endl
           << "    deviceCount:         " << Pa_GetDeviceCount()     << endl
           << "    structVersion:       " << info->structVersion     << endl
           << "    name:                " << info->name              << endl
           << "    hostApi(index):      " << info->hostApi           << endl
           << "    maxInputChannels:    " << info->maxInputChannels  << endl
           << "    maxOutputChannels:   " << info->maxOutputChannels << endl
           << "    defaultLowInputLatency:   " << info->defaultLowInputLatency   << endl
           << "    defaultHighInputLatency:  " << info->defaultHighInputLatency  << endl
           << "    defaultLowOutputLatency:  " << info->defaultLowOutputLatency  << endl
           << "    defaultHighOutputLatency: " << info->defaultHighOutputLatency << endl
           << "    defaultSampleRate:   " << info->defaultSampleRate << endl;

        const PaHostApiInfo * api_info = NULL;

        api_info = Pa_GetHostApiInfo(info->hostApi);

        if(info == NULL)
        {
            M_THROW("Nsound::AudioBackendLibportaudio::getInfo():"
                << ": Pa_GetHostApiInfo() failed\n");
            return ss.str();
        }

        ss << "libportaudio Default Host API Info:"                  << endl
           << "    structVersion:       " << api_info->structVersion << endl
           << "    name:                " << api_info->name          << endl
           << "    deviceCount:         " << api_info->deviceCount   << endl
           << "    defaultInputDevice:  " << api_info->defaultInputDevice  << endl
           << "    defaultOutputDevice: " << api_info->defaultOutputDevice << endl;

        return ss.str();
    #endif
}

void
AudioBackendLibportaudio::
initialize()
{
    #ifndef NSOUND_LIBPORTAUDIO
        state_ = BACKEND_ERROR;
        M_THROW("Nsound::AudioBackendLibportaudio::getInfo():"
            << ": Nsound was not compiled with libportaudio support.\n");
        return;
    #else

        if(state_ != BACKEND_NOT_INITIALIZED)
        {
            return;
        }

        std::stringstream ss;

        PaError ecode = Pa_Initialize();

        if(ecode != paNoError)
        {
            error_buffer_
                << "Nsound::AudioBackendLibportaudio::initialize():"
                << __LINE__
                << ": Pa_Initialize() failed"
                << endl
                << Pa_GetErrorText(ecode)
                << endl;

            state_ = BACKEND_ERROR;
            return;
        }

        // Alocate params object
        out_params_ = new PaStreamParameters();

        if(out_params_ == NULL)
        {
            error_buffer_
                << "Nsound::AudioBackendLibportaudio::initialize():"
                << __LINE__
                << ": failed to allocate output params struct."
                << endl;

            state_ = BACKEND_ERROR;

            Pa_Terminate();
            return;
        }

        // Get the default ouput device
        out_params_->device = Pa_GetDefaultOutputDevice();

        if(out_params_->device == paNoDevice)
        {
            error_buffer_
                << "Nsound::AudioBackendLibportaudio::initialize():"
                << __LINE__
                << ": Pa_GetDefaultOutputDevice() failed"
                << endl;

            state_ = BACKEND_ERROR;

            Pa_Terminate();

            delete out_params_;
            out_params_ = NULL;

            return;
        }

        out_params_->channelCount = channels_;

        switch(bits_per_sample_)
        {
            case 8:
                out_params_->sampleFormat = paInt8;
                break;

            case 16:
                out_params_->sampleFormat = paInt16;
                break;

            case 32:
                out_params_->sampleFormat = paInt32;
                break;

            default:
                error_buffer_
                    << "Nsound::AudioBackendLibportaudio::initialize():"
                    << __LINE__
                    << ": can't handle " << bits_per_sample_
                    << " bits per sample"
                    << endl;

                state_ = BACKEND_ERROR;

                Pa_Terminate();

                delete out_params_;
                out_params_ = NULL;

                return;
        }

        out_params_->suggestedLatency =
            Pa_GetDeviceInfo(out_params_->device)->defaultHighOutputLatency;

        out_params_->hostApiSpecificStreamInfo = NULL;

        // Open the stream.
        ecode = Pa_OpenStream(
            & stream_,
            NULL, // no input channel
            out_params_,
            static_cast<double>(sample_rate_),
            n_frames_per_buffer_,
            paClipOff,
            NULL,   // no callback for now
            NULL);

        if(ecode != paNoError)
        {
            error_buffer_
                << "Nsound::AudioBackendLibportaudio::initialize():"
                << __LINE__
                << ": Pa_OpenStream() failed"
                << endl
                << Pa_GetErrorText(ecode)
                << endl;

            state_ = BACKEND_ERROR;

            Pa_Terminate();

            delete out_params_;
            out_params_ = NULL;

            return;
        }

        state_ = BACKEND_READY;
    #endif
}

//-----------------------------------------------------------------------------
// A function to perform the actual writting since can't perform pointer
// arithmetic on void *
PaError
write_stream(
    PaStream * stream,
    void * data,
    uint32 position,
    uint32 n_frames_per_buffer,
    PaSampleFormat format)
{
    switch(format)
    {
        case paInt8:
        {
            int8 * data_ptr = reinterpret_cast<int8 *>(data);
            return Pa_WriteStream(
                stream,
                reinterpret_cast<void *>(data_ptr + position),
                n_frames_per_buffer);
        }

        case paInt16:
        {
            int16 * data_ptr = reinterpret_cast<int16 *>(data);
            return Pa_WriteStream(
                stream,
                reinterpret_cast<void *>(data_ptr + position),
                n_frames_per_buffer);
        }

        case paInt32:
        {
            int32 * data_ptr = reinterpret_cast<int32 *>(data);
            return Pa_WriteStream(
                stream,
                reinterpret_cast<void *>(data_ptr + position),
                n_frames_per_buffer);
        }
    }

    M_THROW("Nsound::AudioBackendLibportaudio::initialize():"
        << ": can't handle bits per sample type: "
        << format
        << "\n");

    return paSampleFormatNotSupported;
}

void
AudioBackendLibportaudio::
play(void * data, uint32 n_bytes)
{
    #ifndef NSOUND_LIBPORTAUDIO
        state_ = BACKEND_ERROR;
        M_THROW("Nsound::AudioBackendLibportaudio::getInfo():"
            << ": Nsound was not compiled with libportaudio support.\n");
        return;
    #else

        if(state_ != BACKEND_READY)
        {
            return;
        }

        if(n_bytes == 0)
        {
            return;
        }

        if(data == NULL)
        {
            error_buffer_
                << "Nsound::AudioBackendLibportaudio::play():"
                << __LINE__
                << ": data is NULL"
                << endl;

            state_ = BACKEND_ERROR;

            return;
        }

        // Start the stream
        PaError ecode = Pa_StartStream(stream_);

        if(ecode != paNoError)
        {
            error_buffer_
                << "Nsound::AudioBackendLibportaudio::play():"
                << __LINE__
                << ": Pa_StartStream() failed"
                << endl
                << Pa_GetErrorText(ecode)
                << endl;

            state_ = BACKEND_ERROR;

            Pa_CloseStream(stream_);
            Pa_Terminate();

            delete out_params_;
            out_params_ = NULL;

            return;
        }

        uint32 bits = 0;

        switch(out_params_->sampleFormat)
        {
            case paInt8:
                bits = 8;
                break;

            case paInt16:
                bits = 16;
                break;

            case paInt32:
                bits = 32;
                break;

            default:
                error_buffer_
                    << "Nsound::AudioBackendLibportaudio::initialize():"
                    << __LINE__
                    << ": can't handle bits per sample type: "
                    << out_params_->sampleFormat
                    << endl;

                state_ = BACKEND_ERROR;

                Pa_CloseStream(stream_);
                Pa_Terminate();

                delete out_params_;
                out_params_ = NULL;

                return;
        }

        // Calculate the number of samples.
        uint32 n_channels = out_params_->channelCount;
        uint32 bytes_per_sample = bits / 8;
        uint32 n_samples = n_bytes / bytes_per_sample / n_channels;

        uint32 i = 0;
        for(
            i = 0;
            i < n_samples - n_frames_per_buffer_;
            i += n_frames_per_buffer_)
        {
            ecode = write_stream(
                stream_,
                data,
                i * n_channels,
                n_frames_per_buffer_,
                out_params_->sampleFormat);

            if(ecode != paNoError)
            {
                error_buffer_
                    << "Nsound::AudioBackendLibportaudio::play():"
                    << __LINE__
                    << ": Pa_WriteStream() failed"
                    << endl
                    << Pa_GetErrorText(ecode)
                    << endl;

                state_ = BACKEND_ERROR;

                Pa_StopStream(stream_);
                Pa_CloseStream(stream_);
                Pa_Terminate();

                delete out_params_;
                out_params_ = NULL;

                return;
            }
        }

        // Write out remaining samples
        ecode = write_stream(
            stream_,
            data,
            i,
            n_samples - i,
            out_params_->sampleFormat);

        if(ecode != paNoError)
        {
            error_buffer_
                << "Nsound::AudioBackendLibportaudio::play():"
                << __LINE__
                << ": Pa_WriteStream() failed"
                << endl
                << Pa_GetErrorText(ecode)
                << endl;

            state_ = BACKEND_ERROR;

            Pa_StopStream(stream_);
            Pa_CloseStream(stream_);
            Pa_Terminate();

            delete out_params_;
            out_params_ = NULL;

            return;
        }

        // Wait 500 ms to allow the stream to finish playing.
        //
        // At least on my box the playback is always cut off, especially when
        // using Audacity, it's playback is cut off, perhaps this is releated
        // to the audio driver.  I'll leave this in for now.
        //

        // POSIX only implementation
        #if defined(NSOUND_PLATFORM_OS_LINUX)
            timespec wait_time;
            wait_time.tv_sec = 0;
            wait_time.tv_nsec = 500000000;

            nanosleep(&wait_time, NULL);
        #endif

        // Stop the stream
        ecode = Pa_StopStream(stream_);

        if(ecode != paNoError)
        {
            error_buffer_
                << "Nsound::AudioBackendLibportaudio::play():"
                << __LINE__
                << ": Pa_StopStream() failed"
                << endl
                << Pa_GetErrorText(ecode)
                << endl;

            state_ = BACKEND_ERROR;

            Pa_CloseStream(stream_);
            Pa_Terminate();

            delete out_params_;
            out_params_ = NULL;

            return;
        }

    #endif
}

//-----------------------------------------------------------------------------
std::string
static
lower(const std::string & x)
{
    std::string y = x;
    std::transform(y.begin(), y.end(), y.begin(), ::tolower);
    return y;
}

void
AudioBackendLibportaudio::
scanDevices(AudioPlayback & pb, const AudioStream & test_clip)
{
    uint32 orig_driver_id = driver_id_;

    uint32 n_host_apis = Pa_GetHostApiCount();

    for(uint32 i = 0; i < n_host_apis; ++i)
    {
        const PaHostApiInfo * info = Pa_GetHostApiInfo(i);

        if(info != NULL)
        {
            std::string short_name = lower(std::string(info->name));

            cout
                << "Libportaudio: found driver '"
                << short_name
                << "', id = "
                << i
                << ", deviceCount = "
                << info->deviceCount
                << ", defaultOutputDevice = "
                << info->defaultOutputDevice
                << "\nPLAYBACK STARTING ...";
            cout.flush();

            shutdown();

            driver_id_ = i;

            initialize();

            pb.play(test_clip);

            cout << " STOPPED\n";
            cout.flush();
        }
    }

    shutdown();
    driver_id_ = orig_driver_id;
}

//-----------------------------------------------------------------------------
int32
static
integer(const std::string & x, Nsound::AudioBackend::State * state)
{
    std::stringstream ss(x);
    int32 i = 0;

    ss >> i;

    if(ss.fail())
    {
        *state = AudioBackend::BACKEND_ERROR;
        M_THROW("Nsound::AudioBackendLibportaudio::setOption():"
            << ": could not convert '"
            << x
            << "' to an integer.\n");
        return -1;
    }

    return i;
}

void
AudioBackendLibportaudio::
setOption(const std::string & key, const std::string & value)
{
    #ifndef NSOUND_LIBPORTAUDIO
        state_ = BACKEND_ERROR;
        M_THROW("Nsound::AudioBackendLibportaudio::getInfo():"
            << ": Nsound was not compiled with libportaudio support.\n");
        return;
    #else

        std::string k = lower(key);

        if(k == "frames_per_buffer")
        {
            int32 i = integer(value, &state_);

            if(i >= 1)
            {
                n_frames_per_buffer_ = i;
            }
        }
        else
        if(k == "driver")
        {
            std::string driver_name = lower(value);

            uint32 n_host_apis = Pa_GetHostApiCount();

            bool found_driver = false;

            for(uint32 i = 0; i < n_host_apis; ++i)
            {
                const PaHostApiInfo * info = Pa_GetHostApiInfo(i);

                if(info != NULL)
                {
                    std::string short_name = lower(std::string(info->name));

                    if(driver_name == short_name)
                    {
                        driver_id_ = i;
                        found_driver = true;
                    }
                }
            }

            if(!found_driver)
            {
                state_ = BACKEND_ERROR;
                M_THROW("Nsound::AudioBackendLibportaudio::setOption():"
                    << ": failed to select driver '"
                    << key
                    << "'\n");
                return;
            }
        }
        else
        {
            state_ = BACKEND_ERROR;
            M_THROW("Nsound::AudioBackendLibportaudio::setOption():"
                << ": unrecognized key '"
                << key
                << "'\n");
            return;
        }

    #endif
}

void
AudioBackendLibportaudio::
shutdown()
{
    #ifndef NSOUND_LIBPORTAUDIO
        state_ = BACKEND_ERROR;
        M_THROW("Nsound::AudioBackendLibportaudio::getInfo():"
            << ": Nsound was not compiled with libportaudio support.\n");
        return;
    #else

        if(stream_ != NULL)
        {
            Pa_CloseStream(stream_);
            Pa_Terminate();

            stream_ = NULL;

            delete out_params_;
            out_params_ = NULL;
        }

        state_ = BACKEND_NOT_INITIALIZED;

    #endif
}
