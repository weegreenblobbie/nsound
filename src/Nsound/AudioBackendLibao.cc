//-----------------------------------------------------------------------------
//
//  $Id: AudioBackendLibao.cc 874 2014-09-08 02:21:29Z weegreenblobbie $
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

#include <Nsound/Nsound.h>
#include <Nsound/AudioBackendLibao.h>
#include <Nsound/AudioPlayback.h>

#include <algorithm>
#include <iostream>
#include <string>

#include <string.h> // for memset
#include <stdio.h>

#include <ao/ao.h>

using namespace Nsound;

using std::cerr;
using std::cout;
using std::endl;

//-----------------------------------------------------------------------------
// Stupid windows
extern "C"
{
	const char * KSDATAFORMAT_SUBTYPE_PCM = "00000001-0000-0010-8000-00aa00389b71";
}

const uint32 N_DRIVER_TYPES = 13;

std::string driver_types[N_DRIVER_TYPES] =
{
    "alsa",
    "arts",
    "axis",
    "esd",
    "irix",
    "macosx",
    "nas",
    "oss",
    "pulse",
    "roar",
    "sndio",
    "sun",
    "wmm",
};

AudioBackendLibao::
AudioBackendLibao(
    uint32 sample_rate,
    uint32 channels,
    uint32 bits_per_sample)
    :
    AudioBackend(sample_rate, channels, bits_per_sample),
    options_(),
    error_buffer_(""),
    driver_id_(-1),
    device_ptr_(NULL)
{
    initialize();
};

AudioBackendLibao::
~AudioBackendLibao()
{
    shutdown();
}

AudioBackendType
AudioBackendLibao::
getBackendType()
{
    return BACKEND_TYPE_LIBAO;
}

std::string
AudioBackendLibao::
getError()
{
    return error_buffer_.str();
}

std::string
print_info(ao_info * info)
{
    std::stringstream ss;

    ss << "libao Driver Info:" << endl
        << "    type:       "  << info->type       << endl
        << "    name:       "  << info->name       << endl
        << "    short_name: "  << info->short_name << endl
        << "    comment:    "  << info->comment    << endl
        << "    preferred_byte_format: ";

    switch(info->preferred_byte_format)
    {
        case AO_FMT_LITTLE:
            ss << "Little Endian" << endl;
            break;

        case AO_FMT_BIG:
            ss << "Big Endian" << endl;
            break;

        case AO_FMT_NATIVE:

            if(ao_is_big_endian())
            {
                ss << "Native Big Endian" << endl;
            }
            else
            {
                ss << "Native Little Endian" << endl;
            }
            break;

        default:
            ss << "Unknown!" << endl;
            break;
    }

    ss << "    priority:   "   << info->priority     << endl
       << "    option count: " << info->option_count << endl;

    for(int32 i = 0; i < info->option_count; ++i)
    {
        ss << "    options[" << i << "]: " << info->options[i] << endl;
    }

    return ss.str();
}

std::string
AudioBackendLibao::
getInfo()
{
    #ifndef NSOUND_LIBAO
        state_ = BACKEND_ERROR;
        M_THROW("Nsound::AudioBackendLibao::getInfo():"
            << ": Nsound was not compiled with libao support.\n");
        return;
    #else
        if(state_ == BACKEND_NOT_INITIALIZED)
        {
            return "Nsound::AudioBackendLibao::getInfo(): "
                "Backend not initialized yet";
        }
        else if(state_ == BACKEND_ERROR)
        {
            return getError();
        }

        ao_info * info = ao_driver_info(driver_id_);

        if(info == NULL)
        {
            return "Nsound::AudioBackendLibao::getInfo(): "
                "ao_driver_info() failed";
        }

        return print_info(info);
    #endif
}

void
AudioBackendLibao::
initialize()
{
    #ifndef NSOUND_LIBAO
        state_ = BACKEND_ERROR;
        M_THROW("Nsound::AudioBackendLibao::initialize():"
            << ": Nsound was not compiled with libao support.\n");
        return;
    #else

        // Only initialize if the back end has not been initialized yet.
        if(state_ != BACKEND_NOT_INITIALIZED)
        {
            return;
        }

        ao_initialize();

        if(driver_id_ < 0)
        {
            driver_id_ = ao_default_driver_id();
        }

        if(driver_id_ < 0)
        {
            ao_shutdown();

            error_buffer_ << "ao_default_driver_id() failed" << endl;

            state_ = BACKEND_ERROR;

            return;
        }

        // Setup the sample format.
        ao_sample_format format;

        memset(&format, 0, sizeof(ao_sample_format));

        format.bits = bits_per_sample_;
        format.rate = sample_rate_;
        format.channels = channels_;

        #ifdef NSOUND_LITTLE_ENDIAN
            format.byte_format = AO_FMT_LITTLE;
        #else
            format.byte_format = AO_FMT_BIG;
        #endif

        // Need to add config check for the matrix field, for now, just
        // disable.

//~        char matrix[16];
//~        format.matrix = matrix;

//~        switch(channels_)
//~        {
//~            case 1:
//~                sprintf(matrix, "M");
//~                break;

//~            case 2:
//~                sprintf(matrix, "L,R");
//~                break;

//~            default:
//~                error_buffer_
//~                    << "Nsound::AudioBackendLibao::initialize(): "
//~                    << "Don't know how to map "
//~                    << channels_
//~                    << " channels to the ao_sample_format.matrix"
//~                    << endl;

//~               shutdown();
//~               state_ = BACKEND_ERROR;

//~               return;
//~        }

        // Setup any options.

        ao_option * options = NULL;

        uint32 n_options = static_cast<uint32>(options_.size());

        int32 ecode = 0;

        if(n_options >= 2)
        {
            for(uint32 i = 0; i < n_options; i += 2)
            {
                ecode = ao_append_option(
                    &options,
                    options_[i    ].c_str(),
                    options_[i + 1].c_str());

                if(ecode != 1)
                {
                    ao_shutdown();

                    error_buffer_
                        << "Nsound::AudioBackendLibao::initialize():"
                        << __LINE__
                        << ": error appending libao '"
                        << options_[i]
                        << "' : '"
                        << options_[i + 1]
                        << "' device option"
                        << endl;

                    state_ = BACKEND_ERROR;

                    return;
                }
            }
        }

        device_ptr_ = ao_open_live(driver_id_, &format, options);

        ao_free_options(options);

        if(device_ptr_ == NULL)
        {
            ao_shutdown();

            error_buffer_
                << "Nsound::AudioBackendLibao::initialize():"
                << __LINE__
                << ": ao_open_live() failed:"
                << endl;

            switch(errno)
            {
                case AO_ENODRIVER:
                    error_buffer_
                        << "No driver corresponds to driver_id ("
                        << driver_id_
                        << ")"
                        << endl;
                    break;

                case AO_ENOTLIVE:
                    error_buffer_
                        << "This driver is not a live output device"
                        << endl;
                    break;

                case AO_EBADOPTION:
                    error_buffer_
                        << "A valid option key has an invalid value"
                        << endl;
                    break;

                case AO_EOPENDEVICE:
                    error_buffer_
                        << "Cannot open the device (for example, if "
                        << "/dev/dsp cannot be opened for writing)"
                        << endl;
                    break;

                case AO_EFAIL:
                    error_buffer_
                        << "Any other cause of failure"
                        << endl;
                    break;

                default:
                    error_buffer_
                        << "Reason unknown"
                        << endl;
                    break;
            }

            state_ = BACKEND_ERROR;

            return;
        }

        state_ = BACKEND_READY;
    #endif
}

void
AudioBackendLibao::
play(void * data, uint32 n_bytes)
{
    #ifndef NSOUND_LIBAO
        state_ = BACKEND_ERROR;
        M_THROW("Nsound::AudioBackendLibao::play():"
            << ": Nsound was not compiled with libao support.\n");
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
                << "AudioBackendLibao::play():"
                << __LINE__
                << ": data == NULL"
                << endl;

            state_ = BACKEND_ERROR;

            return;
        }

        int32 ecode = 0;

        ecode = ao_play(
            device_ptr_,
            reinterpret_cast<char *>(data),
            n_bytes);

        if(ecode == 0)
        {
            state_ = BACKEND_ERROR;

            ao_close(device_ptr_);
            ao_shutdown();

            error_buffer_
                << "AudioBackendLibao::play():"
                << __LINE__
                << ": ao_play() failed"
                << endl;

            return;
        }

    #endif
}

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
        M_THROW("Nsound::AudioBackendLibao::setOption():"
            << ": could not convert '"
            << x
            << "' to an integer.\n");
        return -1;
    }

    return i;
}

std::string
static
lower(const std::string & x)
{
    std::string y = x;
    std::transform(y.begin(), y.end(), y.begin(), ::tolower);
    return y;
}

void
AudioBackendLibao::
scanDevices(AudioPlayback & pb, const AudioStream & test_clip)
{
    for(uint32 i = 0; i < N_DRIVER_TYPES; ++i)
    {
        int32 id = ao_driver_id(driver_types[i].c_str());

        if(id >= 0)
        {
            shutdown();
            driver_id_ = id;
            initialize();

            cout << "Libao: found driver '"
                 << driver_types[i].c_str()
                 << "', id = "
                 << id
                 << "\nPLAYBACK STARTING ...";
            cout.flush();

            pb.play(test_clip);

            cout << " STOPPED\n";
            cout.flush();

            shutdown();
        }
    }
}

void
AudioBackendLibao::
setOption(const std::string & key, const std::string & value)
{
    #ifndef NSOUND_LIBAO
        state_ = BACKEND_ERROR;
        M_THROW("Nsound::AudioBackendLibao::setOption():"
            << ": Nsound was not compiled with libao support.\n");
        return;
    #else

        std::string k = lower(key);

        // Special handeling if key == "id"
        if(k == "id")
        {
            int32 id = integer(value, &state_);
            if(id >= 0) driver_id_ = id;
        }
        else
        if(k == "driver")
        {
            int32 id = ao_driver_id(value.c_str());

            cerr << "key = 'driver', value = '"
                 << value.c_str()
                 << "', id = "
                 << id
                 << "\n";
            cerr.flush();

            if(id >= 0)
            {
                driver_id_ = id;
            }
            else
            {
                state_ = BACKEND_ERROR;
                M_THROW("Nsound::AudioBackendLibao::setOption():"
                    << ": failed to select driver '"
                    << value
                    << "'\n");
                return;
            }
        }
        else
        {
            options_.push_back(key);
            options_.push_back(value);
        }
    #endif
}

void
AudioBackendLibao::
shutdown()
{
    #ifndef NSOUND_LIBAO
        state_ = BACKEND_ERROR;
        M_THROW("Nsound::AudioBackendLibao::shutdown():"
            << ": Nsound was not compiled with libao support.\n");
        return;
    #else
        if(device_ptr_ != NULL)
        {
            ao_close(device_ptr_);
            device_ptr_ = NULL;
            driver_id_ = -1;
        }

        ao_shutdown();
        state_ = BACKEND_NOT_INITIALIZED;
    #endif
}
