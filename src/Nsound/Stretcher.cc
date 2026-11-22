//-----------------------------------------------------------------------------
//
//  $Id: Stretcher.cc 878 2014-11-23 04:51:23Z weegreenblobbie $
//
//  Nsound is a C++ library and Python module for audio synthesis featuring
//  dynamic digital filters. Nsound lets you easily shape waveforms and write
//  to disk or plot them. Nsound aims to be as powerful as Csound but easy to
//  use.
//
//  Copyright (c) 2008 to Present Nick Hilton
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

#include <Nsound/AudioStream.h>
#include <Nsound/Buffer.h>
#include <Nsound/Sine.h>
#include <Nsound/Stretcher.h>

#include <cstdio>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <deque>
#include <set>

using namespace Nsound;

using std::cerr;
using std::cout;
using std::endl;

//-----------------------------------------------------------------------------
Stretcher::
Stretcher(
    const float64 & sample_rate,
    const float64 & window_size_seconds,
    const float64 & max_delta_window)
    :
    sample_rate_(sample_rate)
{
    Sine sin(sample_rate);
    window_ = sin.drawWindowHanning(window_size_seconds);
    window_length_ = window_.getLength();
    max_delta_ = uint32(float64(window_length_) * max_delta_window);
}

void
Stretcher::
analyize(const Buffer & input, const float64 & f)
{
    Buffer factor(1);
    factor << f;

    analyize(input, factor);
}

void
Stretcher::
analyize(const Buffer & input, const Buffer & factor)
{
    // The frames step through the input by window / 2 / factor until a window
    // no longer fits, so both have to make progress.
    M_ASSERT_MSG(
        input.getLength() > window_length_,
        "The input (" << input.getLength() << " samples) must be longer than "
        "the window (" << window_length_ << " samples)!");

    M_ASSERT_MSG(
        factor.getLength() > 0 && factor.getAbs().getMin() > 0.0,
        "The factors must not be 0!");

    // Always calculate time shift.

    Buffer::const_circular_iterator ifactor = factor.cbegin();

    uint32 tau = static_cast<uint32>(window_length_ / 2.0 / ::fabs(*ifactor));

    uint32 input_length = input.getLength();

    frames_ = Buffer(256);

    uint32 i = 0;
    while(i < input_length - window_length_)
    {
        frames_ << i;
        i += tau;

        ifactor += tau;

        tau = static_cast<uint32>(window_length_ / 2.0 / ::fabs(*ifactor));
    }

    uint32 n_frames = frames_.getLength();

    uint32 mod_frames = n_frames / 100;

    if(n_frames < 100) mod_frames = n_frames;

    if(show_progress_)
    {
        #ifdef NSOUND_CUDA

            printf("Using CUDA\n");

        #else

            #ifdef NSOUND_OPENMP
                #pragma omp parallel
                {
                    if(0 == omp_get_thread_num())
                    {
                        printf("Using OpenMP with %d threads\n",
                            omp_get_num_threads());
                        fflush(stdout);
                    }
                }
            #endif

        #endif

        printf("Analyizing %3.0f%%", 0.0);
        fflush(stdout);
    }

    for(uint32 j = 0; j < n_frames-1; ++j)
    {
        if(show_progress_ && j % mod_frames == 0)
        {
            printf("\b\b\b\b");
            printf("%3.0f%%", 100.0 * float64(j) / float64(n_frames));
            fflush(stdout);
        }

        // WSOLA: frame j + 1 is the candidate near its own position that
        // best continues frame j, half a window on from it.
        frames_[j + 1] = searchForBestMatch(
            input,
            uint32(frames_[j]) + window_length_ / 2,
            uint32(frames_[j + 1]));
    }

    if(show_progress_)
    {
        printf("\b\b\b\b");
        printf("%3.0f%%\n", 100.0);
        fflush(stdout);
    }
}

uint32
Stretcher::
searchForBestMatch(
    const Buffer & input,
    uint32 source_index,
    uint32 search_index) const
{
    // Nick's original score function
    #if 1
        Buffer b_source = input.subbuffer(source_index, window_length_);

        uint32 t = 0;
        float64 min_score = 1.0e100;

        // Windows OpenMP (MSVC /openmp) requires a signed loop counter.
        const int32 n_deltas = static_cast<int32>(max_delta_);

        #ifdef NSOUND_OPENMP
            #pragma omp parallel
        #endif
        {
            // Each thread finds its own best match, then the threads merge
            // them one at a time.  Ties go to the lowest offset, so the
            // result is the same as running the loop on one thread.
            uint32 thread_t = 0;
            float64 thread_min_score = 1.0e100;

            #ifdef NSOUND_OPENMP
                #pragma omp for
            #endif
            for(int32 i = 0; i < n_deltas; ++i)
            {
                Buffer b_test = input.subbuffer(search_index + i, window_length_)
                              - b_source;

                b_test *= b_test;

                float64 score = b_test.getSum();

                if(score < thread_min_score)
                {
                    thread_min_score = score;
                    thread_t = i;
                }
            }

            #ifdef NSOUND_OPENMP
                #pragma omp critical
            #endif
            if(thread_min_score < min_score
                || (thread_min_score == min_score && thread_t < t))
            {
                min_score = thread_min_score;
                t = thread_t;
            }
        }
    #endif

    #if 0
        // Maximizing fft phase information.

        FFTransform transform(100.0); // Sample rate here won't be used.

        Buffer b_source = input.subbuffer(source_index, window_length_);

        FFTChunkVector vec = transform.fft(b_source, window_length_);

        Buffer source_fft = vec[0].getPhase();

        uint32 t = 0;
        float64 min_score = 1.0e100;
        for(uint32 i = 0; i < max_delta_; ++i)
        {
            Buffer b_test = input.subbuffer(search_index + i, window_length_);

            FFTChunkVector vec = transform.fft(b_test, window_length_);

            Buffer test_fft = vec[0].getPhase();

            Buffer delta = source_fft - test_fft;

            delta *= delta;

            float64 score = delta.getSum();

            if(score < min_score)
            {
                min_score = score;
                t = i;
            }
        }
    #endif

    return t + search_index;
}

Buffer
Stretcher::
overlapAdd(const Buffer & input) const
{
    Sine sin(sample_rate_);

    float64 half_window_seconds = (window_length_ / 2) / sample_rate_;

    uint32 half_length = window_length_ / 2;

    Buffer window_start = sin.drawLine(half_window_seconds, 1.0, 1.0)
                       << window_.subbuffer(half_length, half_length);

    Buffer window_finish = window_start.getReverse();

    Buffer output = input.subbuffer(0, window_length_) * window_;

    uint32 n_frames = frames_.getLength();

    uint32 mod_frames = n_frames / 100;

    if(n_frames < 100) mod_frames = n_frames;

    if(show_progress_)
    {
        printf("Creating output %3.0f%%", 0.0);
        fflush(stdout);
    }

    for(uint32 i = 1; i < n_frames; ++i)
    {
        if(show_progress_ && i % mod_frames == 0)
        {
            printf("\b\b\b\b");
            printf("%3.0f%%", 100.0 * float64(i) / float64(n_frames));
            fflush(stdout);
        }

        uint32 frame = uint32(frames_[i]);

        Buffer next_window = input.subbuffer(frame, window_length_)
                           * window_;

        // overlap & add
        output.add(next_window, i * half_length);
    }

    if(show_progress_)
    {
        printf("\b\b\b\b");
        printf("%3.0f%%\n", 100.0);
        fflush(stdout);
    }

    return output;
}

AudioStream
Stretcher::
pitchShift(const AudioStream & x, const float64 & factor)
{
    // Prepare for time shift.
    analyize(x.getMono()[0], factor);

    AudioStream y(x.getSampleRate(), x.getNChannels());

    for(uint32 i = 0; i < x.getNChannels(); ++i)
    {
        y[i] = overlapAdd(x[i]);
    }

    return y.getResample(1.0/factor);
}

AudioStream
Stretcher::
pitchShift(const AudioStream & x, const Buffer & factor)
{
    // Prepare for time shift.
    analyize(x.getMono()[0], factor);

    AudioStream y(x.getSampleRate(), x.getNChannels());

    for(uint32 i = 0; i < x.getNChannels(); ++i)
    {
        y[i] = overlapAdd(x[i]);
    }

    return y.getSpeedUp(factor);
}

Buffer
Stretcher::
pitchShift(const Buffer & x, const float64 & factor)
{
    // Prepare for time shift.
    analyize(x, factor);

    Buffer stretched = overlapAdd(x);

    return stretched.getResample(1.0/factor);
}

Buffer
Stretcher::
pitchShift(const Buffer & x, const Buffer & factor)
{
    // Prepare for time shift.
    analyize(x, factor);

    Buffer stretched = overlapAdd(x);

    return stretched.getSpeedUp(factor);
}

AudioStream
Stretcher::
timeShift(const AudioStream & x, const float64 & factor)
{
    // Prepare for time shift.
    analyize(x.getMono()[0], factor);

    AudioStream y(x.getSampleRate(), x.getNChannels());

    for(uint32 i = 0; i < x.getNChannels(); ++i)
    {
        y[i] = overlapAdd(x[i]);
    }

    return y;
}

AudioStream
Stretcher::
timeShift(const AudioStream & x, const Buffer & factor)
{
    // Prepare for time shift.
    analyize(x.getMono()[0], factor);

    AudioStream y(x.getSampleRate(), x.getNChannels());

    for(uint32 i = 0; i < x.getNChannels(); ++i)
    {
        y[i] = overlapAdd(x[i]);
    }

    return y;
}

Buffer
Stretcher::
timeShift(const Buffer & x, const float64 & factor)
{
    // Prepare for time shift.
    analyize(x, factor);

    Buffer stretched = overlapAdd(x);

    return stretched;
}

Buffer
Stretcher::
timeShift(const Buffer & x, const Buffer & factor)
{
    // Prepare for time shift.
    analyize(x, factor);

    Buffer stretched = overlapAdd(x);

    return stretched;
}

