//-----------------------------------------------------------------------------
//
//  $Id: AudioPlaybackRt.h 912 2015-07-26 00:50:29Z weegreenblobbie $
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
#ifndef _NSOUND_AUDIO_PLAYBACK_RT_H_
#define _NSOUND_AUDIO_PLAYBACK_RT_H_

#include <Nsound/Nsound.h>

#include <Nsound/AudioStream.h>

#ifdef NSOUND_LIBPORTAUDIO
    #include <portaudio.h>
#else
    struct PaStreamCallbackTimeInfo {};
    struct PaStreamCallbackFlags {};
    struct PaStreamParameters {};
    struct PaStream {};
#endif

#ifdef NSOUND_CPP11
    #include <atomic>
#endif

namespace Nsound
{

// forward declare
class AudioStream;
class Buffer;
class Sine;

enum BufferUnderrunMode
{
    // BUM = Buffer Underrun Mode
    BUM_SILENCE,
    BUM_NOISE,
    BUM_TONE
};

struct AudioPlaybackRtDebug
{
    uint32  unknown_error_count;
    uint32  overrun_count;
    uint32  underrun_count;
    uint32  pa_overrun_count;
    uint32  pa_underrun_count;
    uint32  pool_size;
    uint32  n_ready;
    uint32  wr_index;
    uint32  wr_ptr;
    uint32  rd_ptr;
    float64 samplerate;
    uint32  samples_per_buffer;
    bool    is_streaming;

    std::vector<uint32> n_history;

    std::string __str__() const;
};

class AudioPlaybackRt
{
    public:

    static void use_jack(bool flag) { use_jack_ = flag; }

    AudioPlaybackRt(
        float64 sample_rate     = 44100.0,
        uint32 channels         = 1,
        uint32 n_buffers        = 3,
        float64 buffer_size_sec = -1.0);  // negative means use default low latency time.

    ~AudioPlaybackRt();

    uint32 getSamplesPerBuffer() const { return driver_.n_samples_per_buffer_; }
    float64 getActualLatencySeconds() const { return actual_latency_sec_; }

    void setBufferUnderrunMode(BufferUnderrunMode bum);

    //! Returns information about the backend driver.
    std::string getInfo();

    //! Writes samples to the internal circular buffer to be played.
    void play(const AudioStream & a);

    //! Writes samples to the internal circular buffer to be played.
    void play(const Buffer & b);

    //! Writes sample to the internal circular buffer to be played.
    void play(float64 sample);

    //! Writes sample to the internal circular buffer to be played.
    void play(float64 left, float64 right);

    //! Stops playback
    void stop();

    std::string debug_print();

    AudioPlaybackRtDebug get_debug_info() const;

    void _swig_shadow() {}

    private:

    // disable these
    AudioPlaybackRt(const AudioPlaybackRt & copy){}
    AudioPlaybackRt & operator=(const AudioPlaybackRt & rhs){return *this;}

    int
    _callback(
        const void *                     input,
        void *                           output,
        const uint32                     frame_count,
        const PaStreamCallbackTimeInfo * time_info,
        PaStreamCallbackFlags            status_flags);

    static
    int
    _pa_callback(
        const void *                     input,
        void *                           output,
        unsigned long                    frame_count,
        const PaStreamCallbackTimeInfo * time_info,
        PaStreamCallbackFlags            status_flags,
        void *                           ap_obj );

    void _start();

    typedef std::vector< int16 > Int16Vector;
    typedef std::vector< Int16Vector * > Pool;

    //-------------------------------------------------------------------------
    // Data members

    float64 sample_rate_;
    uint32  channels_;

    BufferUnderrunMode underrun_mode_;

    uint32 underrun_count_;
    uint32 overrun_count_;
    uint32 unknown_error_count_;
    uint32 stop_error_count_;

    uint32 pa_underrun_count_;
    uint32 pa_overrun_count_;

    std::vector<uint32> n_history_;

    Sine * sine_; // used to generate noise or tones on buffer underrun

    Pool pool_;
    uint32 pool_size_;

    Pool::iterator pool_begin_;
    Pool::iterator pool_end_;

    #ifdef NSOUND_CPP11
        std::atomic_uint n_ready_;
    #else
        uint32 n_ready_;
    #endif

    Pool::iterator rd_ptr_;
    Pool::iterator wr_ptr_;

    uint32 wr_index_;

    struct Driver
    {
        Driver()
            :
            out_params_(NULL),
            stream_(NULL),
            n_frames_per_buffer_(64),
            n_samples_per_buffer_(64),
            driver_id_(0){}

        PaStreamParameters * out_params_;
        PaStream * stream_;

        uint32 n_frames_per_buffer_;
        uint32 n_samples_per_buffer_;

        std::string host_api_str_;
        uint32 host_api_;
        uint32 driver_id_;
    };

    Driver driver_;

    float64 actual_latency_sec_;

    static bool use_jack_;

}; // AudioPlaybackRt


// Must delcare friend functions here to give them proper namespace scope.
void operator>>(const AudioStream & lhs, AudioPlaybackRt & rhs);
void operator>>(const Buffer & lhs, AudioPlaybackRt & rhs);
void operator>>(float64 sample, AudioPlaybackRt & rhs);

} // namespace Nsound

#endif
// :mode=c++: jEdit modeline