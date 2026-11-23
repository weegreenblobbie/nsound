//-----------------------------------------------------------------------------
//
//  $Id: AudioPlaybackRt.cc 897 2015-06-07 17:43:34Z weegreenblobbie $
//
//  Copyright (c) 2005-2006 Nick Hilton
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
//
// Some PortAudio notes:
//
// A "frame" consists of all the samples for a given time step.  If you have
// stereo audio, then frames 0 and 1 will contain:
//     frame 0 [left_sample_0, right_sample0]
//     frame 1 [left_sample_1, right_sample1]
//     ...
//
// The data above might contain 4 samples, but only 2 frames, or 2 samples in
// time.
//
//-----------------------------------------------------------------------------

#include <Nsound/Nsound.h>

#include <Nsound/AudioPlaybackRt.h>
#include <Nsound/AudioStream.h>
#include <Nsound/Buffer.h>
#include <Nsound/RandomNumberGenerator.h>
#include <Nsound/Sine.h>
#include <Nsound/Wavefile.h>

#if defined(NSOUND_LIBPORTAUDIO) && defined(NSOUND_PLATFORM_OS_WINDOWS)
    #include <pa_win_wasapi.h>
#endif

#include <algorithm>
#include <atomic>
#include <cstring> // for memset
#include <limits>
#include <chrono>
#include <thread>

using std::cerr;
using std::cout;
using std::endl;

namespace Nsound
{

bool AudioPlaybackRt::use_jack_ = false;
std::string AudioPlaybackRt::host_api_name_;

//-----------------------------------------------------------------------------
// stub out class if we don't have portaudio

#if !defined(NSOUND_LIBPORTAUDIO)

    AudioPlaybackRt::AudioPlaybackRt(float64, uint32, uint32, float64)
    {
        M_THROW("Nsound was not compiled with portaudio.");
    }

    AudioPlaybackRt::~AudioPlaybackRt() {}
    void AudioPlaybackRt::setBufferUnderrunMode(BufferUnderrunMode) {}
    std::string AudioPlaybackRt::getInfo() { return "nsound was compiled without rt playback"; }
    void AudioPlaybackRt::play(const AudioStream &) {}
    void AudioPlaybackRt::play(const Buffer &) {}
    void AudioPlaybackRt::play(float64) {}
    void AudioPlaybackRt::play(float64, float64) {}
    void AudioPlaybackRt::stop() {}
    std::string AudioPlaybackRt::debug_print() { return "nsound was compiled without rt playback"; }

    AudioPlaybackRtDebug AudioPlaybackRt::get_debug_info() const { return AudioPlaybackRtDebug(); }
    std::vector<std::string> AudioPlaybackRt::getHostApis() { return {}; }


//-----------------------------------------------------------------------------
// Real class implementation with portaudio

#else

//! Returns s in lower case.
static
std::string
lower(std::string s)
{
    std::transform(s.begin(), s.end(), s.begin(), ::tolower);
    return s;
}

std::vector<std::string>
AudioPlaybackRt::
getHostApis()
{
    std::vector<std::string> names;

    if(Pa_Initialize() != paNoError) return names;

    for(PaHostApiIndex i = 0; i < Pa_GetHostApiCount(); ++i)
    {
        const PaHostApiInfo * info = Pa_GetHostApiInfo(i);

        if(info != nullptr) names.push_back(info->name);
    }

    Pa_Terminate();

    return names;
}

void my_atomic_init(std::atomic_uint & a, unsigned int v)
{
    // Windows
    #ifdef __WIN32__
        std::atomic_init(&a, v);
    #else
        a = v;
    #endif
}

AudioPlaybackRt::
AudioPlaybackRt(
    float64 sample_rate,
    uint32  channels,
    uint32  n_buffers,
    float64 buffer_size_sec)
    :
    sample_rate_(sample_rate),
    channels_(channels),
    underrun_mode_(BUM_SILENCE),
    underrun_count_(0),
    overrun_count_(0),
    unknown_error_count_(0),
    stop_error_count_(0),
    pa_underrun_count_(0),
    pa_overrun_count_(0),
    n_history_(),
    sine_(std::make_unique<Sine>(sample_rate)),
    pool_(),
    pool_size_(n_buffers),
    pool_begin_(),
    pool_end_(),
    n_ready_(),
    rd_ptr_(),
    wr_ptr_(),
    wr_index_(0),
    driver_(),
    actual_latency_sec_(0)
{
    M_ASSERT_VALUE(channels_, >,  0);
    M_ASSERT_VALUE(channels_, <=, 2);
    M_ASSERT_VALUE(n_buffers, >=, 2);

    my_atomic_init(n_ready_, 0u);

    n_history_.reserve(16);

//~    std::cerr << "lock free? " << n_ready_.is_lock_free() << "\n";

    PaError ecode = Pa_Initialize();

    if(ecode != paNoError)
    {
        M_THROW(
            "Nsound::AudioPlaybackRt"
            << ": Pa_Initialize() failed"
            << endl
            << Pa_GetErrorText(ecode));
    }

    // Allocate params object
    // Zeroed.
    driver_.out_params_ = std::make_unique<PaStreamParameters>();

    // Try to get the JACK Audio Connection Ket Host API if available.

    PaHostApiIndex api_index = -1;

    if(use_jack_)
    {
        api_index = Pa_HostApiTypeIdToHostApiIndex(paJACK);
    }
    else if(! host_api_name_.empty())
    {
        for(PaHostApiIndex i = 0; i < Pa_GetHostApiCount(); ++i)
        {
            const PaHostApiInfo * api = Pa_GetHostApiInfo(i);

            if(api != nullptr && lower(api->name) == lower(host_api_name_))
            {
                api_index = i;
            }
        }
    }
    else
    {
        api_index = Pa_GetDefaultHostApi();
    }

    if(api_index < 0)
    {
        Pa_Terminate();

        if(use_jack_)
        {
            M_THROW(
                "Nsound::AudioPlaybackRt"
                << ": could not find JACK host api!  "
                "Is the JACK server running?");
        }
        else if(! host_api_name_.empty())
        {
            std::string names;

            for(const std::string & name : getHostApis())
            {
                names += " '" + name + "'";
            }

            M_THROW(
                "Nsound::AudioPlaybackRt"
                << ": no host api '" << host_api_name_
                << "', the ones compiled in:" << names);
        }
        else
        {
            M_THROW(
                "Nsound::AudioPlaybackRt"
                << ": could not find host api to use!");
        }
    }

    driver_.host_api_ = api_index;

    const PaHostApiInfo * info = Pa_GetHostApiInfo(driver_.host_api_);

    if(info == nullptr)
    {
        Pa_Terminate();

        M_THROW(
            "Nsound::AudioPlaybackRt"
            << ": Pa_GetHostApiInfo() failed"
            << endl);
    }

    driver_.host_api_str_ = std::string(info->name);

    // Get the default ouput device

    driver_.out_params_->device = info->defaultOutputDevice;

    if(driver_.out_params_->device == paNoDevice)
    {
        Pa_Terminate();
        driver_.out_params_.reset();

        M_THROW(
            "Nsound::AudioPlaybackRt"
            << ": failed to get default output device!");
    }

    const PaDeviceInfo * dinfo = Pa_GetDeviceInfo(
        driver_.out_params_->device);

    if(dinfo == nullptr)
    {
        Pa_Terminate();

        M_THROW(
            "Nsound::AudioPlaybackRt"
            << ": Pa_GetDeviceInfo() failed\n");
    }

    // use default low output latency if unspecified by user.
    if(buffer_size_sec <= 0.0)
    {
        buffer_size_sec = dinfo->defaultLowOutputLatency;
    }

    driver_.out_params_->channelCount = channels_;
    driver_.out_params_->sampleFormat = paInt16;

    // Never less latency than the device's lowest: below it, a Windows MME
    // device distorts without reporting underruns.  The buffers stay the
    // size asked for.
    driver_.out_params_->suggestedLatency = std::max(
        buffer_size_sec,
        dinfo->defaultLowOutputLatency);
    driver_.out_params_->hostApiSpecificStreamInfo = nullptr;

    #ifdef NSOUND_PLATFORM_OS_WINDOWS

        // WASAPI plays in the format the device is set to in Windows, e.g.
        // 48000 Hz stereo, unless it is asked to convert.  Used while the
        // stream opens.
        PaWasapiStreamInfo wasapi_info{};

        if(info->type == paWASAPI)
        {
            wasapi_info.size = sizeof(PaWasapiStreamInfo);
            wasapi_info.hostApiType = paWASAPI;
            wasapi_info.version = 1;
            wasapi_info.flags = paWinWasapiAutoConvert;

            driver_.out_params_->hostApiSpecificStreamInfo = &wasapi_info;
        }

    #endif

    //-------------------------------------------------------------------------
    // Allocate circular buffers.

    driver_.n_frames_per_buffer_ = static_cast<uint32>(
        buffer_size_sec * sample_rate_);

    driver_.n_samples_per_buffer_ = driver_.n_frames_per_buffer_ * channels_;

    if(driver_.n_frames_per_buffer_ < 16)
    {
        Pa_Terminate();

        M_THROW(
            "Nsound::AudioPlaybackRt"
            << ": n_frames_per_buffer_ < 16"
            << endl);
    }

    for(uint32 i = 0; i < n_buffers; ++i)
    {
        pool_.emplace_back(driver_.n_samples_per_buffer_, 0);
    }

    pool_begin_ = pool_.begin();
    pool_end_   = pool_.end();

    rd_ptr_ = pool_begin_;
    wr_ptr_ = pool_begin_;

    //-------------------------------------------------------------------------
    // Check if format is supported.

    ecode = Pa_IsFormatSupported(nullptr, driver_.out_params_.get(), sample_rate_);

    if(ecode != paNoError)
    {
        Pa_Terminate();

        M_THROW(
            "Nsound::AudioPlaybackRt"
            << ": Pa_IsFormatSupported() failed ("
            << sample_rate_
            << " sample rate, paInt16, "
            << channels_ << " channel(s))"
            << endl
            << Pa_GetErrorText(ecode));
    }

    //-------------------------------------------------------------------------
    // Open the stream.

    ecode = Pa_OpenStream(
        &driver_.stream_,
        nullptr, // no input channel
        driver_.out_params_.get(),
        sample_rate_,
        driver_.n_frames_per_buffer_,
        paClipOff | paDitherOff,
        &AudioPlaybackRt::_pa_callback,
        reinterpret_cast<void *>(this));

    if(ecode != paNoError)
    {
        Pa_Terminate();

        M_THROW(
            "Nsound::AudioPlaybackRt:"
            << __LINE__
            << ": Pa_OpenStream() failed"
            << endl
            << Pa_GetErrorText(ecode));
    }

    // The WASAPI settings were for opening only.
    driver_.out_params_->hostApiSpecificStreamInfo = nullptr;

    // Grab output latencey.
    const PaStreamInfo * sinfo = Pa_GetStreamInfo(driver_.stream_);

    if(sinfo == nullptr)
    {
        Pa_Terminate();

        M_THROW(
            "Nsound::AudioPlaybackRt"
            << ": Pa_GetStreamInfo() failed"
            << endl
            << Pa_GetErrorText(ecode));
    }

    actual_latency_sec_ = sinfo->outputLatency;
}

AudioPlaybackRt::
~AudioPlaybackRt()
{
    // Play what was written before stopping: a program that plays and ends
    // would cut the last buffers off.  Failing, it stops all the same.
    try
    {
        _play_written();
    }
    catch(...)
    {
    }

    stop();

    PaError ecode = paNoError;

    if(driver_.stream_)
    {
        ecode = Pa_CloseStream(driver_.stream_);

        driver_.stream_ = nullptr;

        driver_.out_params_.reset();
    }


    if(ecode != paNoError)
    {
        Pa_Terminate();

        M_THROW(
            "Nsound::AudioPlaybackRt"
            << ": Pa_CloseStream() failed"
            << endl
            << Pa_GetErrorText(ecode));
    }

    ecode = Pa_Terminate();

    if(ecode != paNoError)
    {
        M_THROW(
            "Nsound::AudioPlaybackRt"
            << ": Pa_Terminate() failed"
            << endl
            << Pa_GetErrorText(ecode));
    }
}

int
AudioPlaybackRt::
_pa_callback(
    const void *                     input,
    void *                           output,
    unsigned long                    frame_count,
    const PaStreamCallbackTimeInfo * time_info,
    PaStreamCallbackFlags            status_flags,
    void *                           ap_obj )
{
    return reinterpret_cast<AudioPlaybackRt *>(ap_obj)->_callback(
        input, output, frame_count, time_info, status_flags);
}

#define SCALE static_cast<float64>(std::numeric_limits<int16>::max())

//! Returns the sample as 16 bits, full scale, 1.0, is 32767, clipped there.
static
int16
to_int16(float64 sample)
{
    return static_cast<int16>(std::clamp(SCALE * sample, -SCALE, SCALE));
}

int
AudioPlaybackRt::
_callback(
    const void *                     input,
    void *                           output,
    const uint32                     frames_per_buffer,
    const PaStreamCallbackTimeInfo * time_info,
    PaStreamCallbackFlags            status_flags)
{
    if(frames_per_buffer != driver_.n_frames_per_buffer_) ++unknown_error_count_;

    int16 * dst_ptr = reinterpret_cast<int16 *>(output);

    uint32 n = n_ready_.load();

//~    DEBUG
//~    if(n_history_.size() < n_history_.capacity()) n_history_.push_back(n);

    BufferUnderrunMode bum = underrun_mode_;

    int16 * src = rd_ptr_->data();

    // No underrun
    if(n > 0)
    {
        for(uint32 i = 0; i < driver_.n_samples_per_buffer_; ++i)
        {
            dst_ptr[i] = src[i];
        }

        n_ready_.fetch_sub(1);

        ++rd_ptr_;

        if(rd_ptr_ == pool_end_)
        {
            rd_ptr_ = pool_begin_;
        }
    }

    // Oops, underrun!
    else
    {
        ++underrun_count_;

        switch(bum)
        {
            case BUM_SILENCE:
            {
                for(uint32 i = 0; i < driver_.n_samples_per_buffer_; ++i)
                {
                    dst_ptr[i] = 0;
                }

                break;
            }

            case BUM_NOISE:
            {
                RandomNumberGenerator & rng = sine_->getRandomNumberGenerator();

                for(uint32 i = 0; i < driver_.n_samples_per_buffer_; ++i)
                {
                    float64 sample = SCALE * rng.get(-0.666, 0.666);
                    dst_ptr[i] = static_cast<int16>(sample);
                }

                break;
            }

            case BUM_TONE:
            {
                float64 tone = 440.0;

                uint32 i = 0;

                while(i < driver_.n_frames_per_buffer_)
                {
                    float64 s = SCALE * 0.666 * sine_->generate(tone);

                    int16 sample = static_cast<int16>(s);

                    for(uint32 j = 0; j < channels_; ++j)
                    {
                        dst_ptr[i++] = sample;
                    }
                }
            }
        }
    }

    if(status_flags & paOutputUnderflow)
    {
        ++pa_underrun_count_;
    }

    if(status_flags & paOutputOverflow)
    {
        ++pa_overrun_count_;
    }

    return paContinue;
}

void
AudioPlaybackRt::
_start()
{
    if(0 == Pa_IsStreamStopped(driver_.stream_)) return;

    n_history_.clear();

    PaError ecode  = Pa_StartStream( driver_.stream_ );

    if(ecode != paNoError)
    {
        Pa_Terminate();

        M_THROW(
            "Nsound::AudioPlaybackRt"
            << ": Pa_StartStream() failed"
            << endl
            << Pa_GetErrorText(ecode));
    }
}

void
AudioPlaybackRt::
stop()
{
    if(Pa_IsStreamStopped(driver_.stream_)) return;

    PaError ecode = Pa_StopStream(driver_.stream_);

    if(ecode != paNoError)
    {
        ++stop_error_count_;
        M_THROW(
            "Nsound::AudioPlaybackRt"
            << ": Pa_StopStream() failed"
            << endl
            << Pa_GetErrorText(ecode));
    }

    // zero out buffers, reset read/write pointers.

    n_ready_ = wr_index_ = 0;
    rd_ptr_ = wr_ptr_ = pool_begin_;

    for(Int16Vector & vec : pool_)
    {
        std::fill(vec.begin(), vec.end(), 0);
    }
}

void
AudioPlaybackRt::
setBufferUnderrunMode(BufferUnderrunMode bum)
{
    M_ASSERT_VALUE(1, ==, Pa_IsStreamStopped(driver_.stream_));
    underrun_mode_ = bum;
}

void
AudioPlaybackRt::
play(const AudioStream & as)
{
    M_ASSERT_VALUE(as.getNChannels(), <=, 2);

    uint32 sr1 = static_cast<uint32>(as.getSampleRate());
    uint32 sr2 = static_cast<uint32>(sample_rate_);

    if(sr1 != sr2)
    {
        std::cerr
            << "\nWARNING: AudioStream sample_rate != AudioPlaybackRt "
            << "sample_rate ("
            << sr1
            << " != "
            << sr2
            << ")\n";
    }

    if(as.getNChannels() == 1)
    {
        play(as[0]);
        return;
    }

    M_ASSERT_VALUE(as[0].getLength(), ==, as[1].getLength());

    for(uint32 i = 0; i < as.getLength(); ++i)
    {
        play(as[0][i], as[1][i]);
    }

}

void
AudioPlaybackRt::
play(const Buffer & b)
{
    for(auto & s : b)
    {
        play(s);
    }
}

void
AudioPlaybackRt::
_play_written()
{
    if(driver_.stream_ == nullptr) return;

    // Silence fills the last buffer, then it can be played.
    while(wr_index_ != 0)
    {
        if(channels_ == 2) play(0.0, 0.0);
        else play(0.0);
    }

    if(n_ready_.load() == 0) return;

    _start();

    // Twice the time the buffers take, the device may be slower.
    float64 buffer_sec = driver_.n_frames_per_buffer_ / sample_rate_;

    auto deadline = std::chrono::steady_clock::now()
        + std::chrono::duration<float64>(2.0 * pool_size_ * buffer_sec + 1.0);

    while(n_ready_.load() > 0 && std::chrono::steady_clock::now() < deadline)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void
AudioPlaybackRt::
_wait_for_free_buffer()
{
    uint32 n = n_ready_.load();

    if(n == pool_size_)
    {
        _start();
    }

    while(n == pool_size_)
    {
        ++overrun_count_;
        std::this_thread::yield();
        n = n_ready_.load();
    }
}

void
AudioPlaybackRt::
play(float64 sample)
{
    if(channels_ == 2)
    {
        return play(sample, sample);
    }

    M_ASSERT_VALUE(wr_index_, <, driver_.n_samples_per_buffer_);

    if(wr_index_ == 0) _wait_for_free_buffer();

    (*wr_ptr_)[wr_index_++] = to_int16(sample);

    // The buffer is full, it's ready to play.
    if(wr_index_ == driver_.n_samples_per_buffer_)
    {
        n_ready_.fetch_add(1);

        wr_index_ = 0;

        ++wr_ptr_;

        if(wr_ptr_ == pool_end_)
        {
            wr_ptr_ = pool_begin_;
        }
    }
}

void
AudioPlaybackRt::
play(float64 left, float64 right)
{
    M_ASSERT_VALUE(wr_index_, <, driver_.n_samples_per_buffer_);

    if(wr_index_ == 0) _wait_for_free_buffer();

    (*wr_ptr_)[wr_index_++] = to_int16(left);
    (*wr_ptr_)[wr_index_++] = to_int16(right);

    // The buffer is full, it's ready to play.
    if(wr_index_ == driver_.n_samples_per_buffer_)
    {
        n_ready_.fetch_add(1);

        wr_index_ = 0;

        ++wr_ptr_;

        if(wr_ptr_ == pool_end_)
        {
            wr_ptr_ = pool_begin_;
        }
    }
}

std::string
AudioPlaybackRt::
getInfo()
{
    const PaDeviceInfo * info = nullptr;

    info = Pa_GetDeviceInfo(driver_.out_params_->device);

    if(info == nullptr)
    {
        M_THROW("Nsound::AudioPlaybackRt::getInfo():"
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

    const PaHostApiInfo * api_info = nullptr;

    api_info = Pa_GetHostApiInfo(info->hostApi);

    if(info == nullptr)
    {
        M_THROW("Nsound::AudioPlaybackRt::getInfo():"
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
}

std::string
AudioPlaybackRt::
debug_print()
{
    std::stringstream ss;

    ss
        << "AudioPlaybackRt:\n"
        << "    sample_rate_ = " << sample_rate_ << "\n"
        << "    channels_ = " << channels_ << "\n"
        << "    pool_size_ = " << pool_size_ << "\n"
        << "    n_ready_   = " << n_ready_.load() << "\n"
        << "    wr_index_  = " << wr_index_ << "\n"
        << "    underrun_count_ = " << underrun_count_ << "\n"
        << "    overrun_count_  = " << overrun_count_ << "\n"
        << "    unknown_error_count_ = " << unknown_error_count_ << "\n"
        << "    stop_error_count_ = " << stop_error_count_ << "\n"
        << "    pa_underrun_count_ = " << pa_underrun_count_ << "\n"
        << "    pa_overrun_count_  = " << pa_overrun_count_ << "\n"
        << "    driver_.n_frames_per_buffer_ = "
        << driver_.n_frames_per_buffer_ << "\n"
        << "    driver_.n_samples_per_buffer_ = "
        << driver_.n_samples_per_buffer_ << "\n";

    if(driver_.out_params_)
    {
        ss
            << "    driver_.out_params_->suggestedLatency = "
            << driver_.out_params_->suggestedLatency << "\n";
    }

    if(driver_.stream_)
    {
        const PaStreamInfo * sinfo = Pa_GetStreamInfo(driver_.stream_);

        M_CHECK_PTR(sinfo);

        ss
            << "    actual samplerate = " << sinfo->sampleRate << "\n"
            << "    actual output latency = " << sinfo->outputLatency << "\n";
    }

    ss
        << "    PortAudio Device Driver Info:\n"
        << "        Host API: " << driver_.host_api_str_ << "\n";

    const PaDeviceInfo * info = Pa_GetDeviceInfo(driver_.out_params_->device);

    if(info == nullptr)
    {
        ss << "        Pa_GetDeviceInfo() failed!\n";
    }
    else
    {
        ss
            << "        Name:              " << info->name << "\n"
            << "        maxOutputChannels: " << info->maxOutputChannels << "\n"
            << "        defaultSampleRate: " << info->defaultSampleRate << "\n"
            << "        defaultLowOutputLatency:  " << info->defaultLowOutputLatency << "\n"
            << "        defaultHighOutputLatency: " << info->defaultHighOutputLatency << "\n";
    }

    return ss.str();
}


AudioPlaybackRtDebug
AudioPlaybackRt::
get_debug_info() const
{
    AudioPlaybackRtDebug info;

    info.unknown_error_count = unknown_error_count_;
    info.overrun_count = overrun_count_;
    info.underrun_count = underrun_count_;
    info.pa_overrun_count = pa_overrun_count_;
    info.pa_underrun_count = pa_underrun_count_;
    info.pool_size = static_cast<uint32>(pool_.size());
    info.n_ready = n_ready_.load();
    info.wr_index = wr_index_;
    info.n_history = n_history_;

    // The buffers' indices in the pool.
    info.wr_ptr = static_cast<uint32>(wr_ptr_ - pool_begin_);
    info.rd_ptr = static_cast<uint32>(rd_ptr_ - pool_begin_);

    const PaStreamInfo * sinfo = Pa_GetStreamInfo(driver_.stream_);

    if(sinfo)
    {
        info.samplerate = sinfo->sampleRate;
    }
    else
    {
        info.samplerate = -1.0;
    }

    info.samples_per_buffer = driver_.n_samples_per_buffer_;

    info.is_streaming = 1 == Pa_IsStreamActive(driver_.stream_);

    return info;
}

#endif
//
//-----------------------------------------------------------------------------

void
operator>>(const AudioStream & lhs, AudioPlaybackRt & rhs)
{
    rhs.play(lhs);
}

void
operator>>(const Buffer & lhs, AudioPlaybackRt & rhs)
{
    rhs.play(lhs);
}

void
operator>>(float64 sample, AudioPlaybackRt & rhs)
{
    rhs.play(sample);
}

std::string
AudioPlaybackRtDebug::
__str__() const
{
    std::stringstream ss;

    ss
        << "AudioPlaybackRtDebug:\n"
        << "    unknown_error_count = " << unknown_error_count << "\n"
        << "    overrun_count      = " << overrun_count << "\n"
        << "    underrun_count     = " << underrun_count << "\n"
        << "    pa_overrun_count   = " << pa_overrun_count << "\n"
        << "    pa_underrun_count  = " << pa_underrun_count << "\n"
        << "    pool_size          = " << pool_size << "\n"
        << "    n_ready            = " << n_ready << "\n"
        << "    wr_index           = " << wr_index << "\n"
        << "    wr_ptr             = " << wr_ptr << "\n"
        << "    rd_ptr             = " << rd_ptr << "\n"
        << "    sampleRate         = " << samplerate << "\n"
        << "    samples_per_buffer = " << samples_per_buffer << "\n"
        << "    is_streaming       = ";

    if(is_streaming) ss << "yes\n";
    else             ss << "no\n";

    ss << "    n_history:\n";

    for(uint32 n : n_history)
    {
        ss << "        " << n << "\n";
    }

    return ss.str();
}


} // namespace
