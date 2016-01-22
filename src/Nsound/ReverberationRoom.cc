//-----------------------------------------------------------------------------
//
//  $Id: ReverberationRoom.cc 874 2014-09-08 02:21:29Z weegreenblobbie $
//
//  Copyright (c) 2008 to Present Nick Hilton
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
//-----------------------------------------------------------------------------////

#include <Nsound/AudioStream.h>
#include <Nsound/Buffer.h>
#include <Nsound/FilterAllPass.h>
#include <Nsound/FilterCombLowPassFeedback.h>
#include <Nsound/ReverberationRoom.h>

#include <ctime>
#include <cstdlib>

using namespace Nsound;

using std::cerr;
using std::endl;

#define CERR_HEADER __FILE__ << ":" << __LINE__ << ": "

const
float64
ReverberationRoom::
COMB_DELAY_TIME_SECONDS_[N_COMB_FILTERS_] =
{
    1116.0 / 44100.0,
    1188.0 / 44100.0,
    1277.0 / 44100.0,
    1356.0 / 44100.0,
    1422.0 / 44100.0,
    1491.0 / 44100.0,
    1557.0 / 44100.0,
    1617.0 / 44100.0
};

const
float64
ReverberationRoom::
ALL_PASS_DELAY_TIME_SECONDS_[N_ALL_PASS_FILTERS_] =
{
    556.0 / 44100.0,
    441.0 / 44100.0,
    341.0 / 44100.0,
    225.0 / 44100.0
};

const
float64
ReverberationRoom::
ROOM_FEEDBACK_SCALE_ = 0.28;

const
float64
ReverberationRoom::
ROOM_FEEDBACK_OFFSET_ = 0.7;


//-----------------------------------------------------------------------------
float64
getRandom()
{
    return static_cast<float64>(rand() / (RAND_MAX + 1.0));
}

//-----------------------------------------------------------------------------
ReverberationRoom::
ReverberationRoom(
    const float64 & sample_rate,
    const float64 & room_feedback,
    const float64 & wet_percent,
    const float64 & dry_percent,
    const float64 & low_pass_frequency_Hz,
    const float64 & stereo_spread_seconds)
    :
    sample_rate_(sample_rate),
    wet_percent_(wet_percent),
    dry_percent_(dry_percent),
    comb_left_(NULL),
    comb_right_(NULL),
    allpass_left_(NULL),
    allpass_right_(NULL)
{
    // Bounds checks:
    if(wet_percent_ < 0.0)
    {
        wet_percent_ = 0.0;
    }

    if(wet_percent_ > 1.0)
    {
        wet_percent_ = 1.0;
    }

    if(dry_percent_ < 0.0)
    {
        dry_percent_ = 0.0;
    }

    if(dry_percent_ > 1.0)
    {
        dry_percent_ = 1.0;
    }

    float64 lpf = low_pass_frequency_Hz;

    if(lpf < 0.0)
    {
        lpf = 0.0;
    }

    float64 stereo_spread = stereo_spread_seconds;

    if(stereo_spread < 0.0)
    {
        stereo_spread = 0.0;
    }

    // Convert the room size to a delay time.
    float64 comb_feedback = ROOM_FEEDBACK_SCALE_ * room_feedback
                          + ROOM_FEEDBACK_OFFSET_;

    // Allocate filter pointers
    comb_left_  = new FilterCombLowPassFeedback * [N_COMB_FILTERS_];
    comb_right_ = new FilterCombLowPassFeedback * [N_COMB_FILTERS_];

    for(uint32 i = 0; i < N_COMB_FILTERS_; ++i)
    {
        comb_left_[i] = new FilterCombLowPassFeedback(
            sample_rate_,
            COMB_DELAY_TIME_SECONDS_[i],
            comb_feedback,
            lpf);

        comb_right_[i] = new FilterCombLowPassFeedback(
            sample_rate_,
            COMB_DELAY_TIME_SECONDS_[i] + stereo_spread,
            comb_feedback,
            lpf);
    }

    // Allocate filter pointers
    allpass_left_  = new FilterAllPass * [N_ALL_PASS_FILTERS_];
    allpass_right_ = new FilterAllPass * [N_ALL_PASS_FILTERS_];

    for(uint32 i = 0; i < N_ALL_PASS_FILTERS_; ++i)
    {
        allpass_left_[i] = new FilterAllPass(
            sample_rate_,
            ALL_PASS_DELAY_TIME_SECONDS_[i],
            0.5);

        allpass_right_[i] = new FilterAllPass(
            sample_rate_,
            ALL_PASS_DELAY_TIME_SECONDS_[i] + stereo_spread,
            0.5);
    }

    ReverberationRoom::reset();

    // Init random number generator.
    std::srand(static_cast<unsigned int>(std::time(0)));
}

//-----------------------------------------------------------------------------
ReverberationRoom::
ReverberationRoom(const ReverberationRoom & copy)
    :
    sample_rate_(copy.sample_rate_),
    wet_percent_(copy.wet_percent_),
    dry_percent_(copy.dry_percent_),
    comb_left_(NULL),
    comb_right_(NULL),
    allpass_left_(NULL),
    allpass_right_(NULL)
{
    *this = copy;
}

//-----------------------------------------------------------------------------
ReverberationRoom::
~ReverberationRoom()
{
    for(uint32 i = 0; i < N_COMB_FILTERS_; ++i)
    {
        delete comb_left_[i];
        delete comb_right_[i];
    }
    delete [] comb_left_;
    delete [] comb_right_;

    for(uint32 i = 0; i < N_ALL_PASS_FILTERS_; ++i)
    {
        delete allpass_left_[i];
        delete allpass_right_[i];
    }
    delete [] allpass_left_;
    delete [] allpass_right_;
}

AudioStream
ReverberationRoom::
filter(const AudioStream & x)
{
    if(x.getNChannels() == 1)
    {
        return filter(x[0]);
    }
    else if(x.getNChannels() < 2)
    {
        return x;
    }

    reset();

    AudioStream y(sample_rate_, 2);

    uint32 n_samples = x.getLength();

    float64 left;
    float64 right;

    for(uint32 n = 0; n < n_samples; ++n)
    {
        filter(left, right, x[0][n], x[1][n]);

        y[0] << left;
        y[1] << right;
    }

    return y;
}

AudioStream
ReverberationRoom::
filter(const Buffer & x)
{
    reset();
    AudioStream y(sample_rate_, 2);

    uint32 n_samples = x.getLength();

    float64 left;
    float64 right;

    for(uint32 n = 0; n < n_samples; ++n)
    {
        filter(left, right, x[n], x[n]);

        y[0] << left;
        y[1] << right;
    }

    return y;
}

void
ReverberationRoom::
filter(
    float64 & out_left,
    float64 & out_right,
    const float64 & in_left,
    const float64 & in_right)
{
    out_left  = 0.0 + getRandom() * 1e-23;
    out_right = 0.0 + getRandom() * 1e-23;

    // Sum the comb filters in parallel
    for(uint32 i = 0; i < N_COMB_FILTERS_; ++i)
    {
        out_left  += comb_left_[i]->filter(in_left * 0.15);
        out_right += comb_right_[i]->filter(in_right * 0.15);
    }

    // Serilalize the all pass fitlers
    for(uint32 i = 0; i < N_ALL_PASS_FILTERS_; ++i)
    {
        out_left  = allpass_left_[i]->filter(out_left);
        out_right = allpass_right_[i]->filter(out_left);
    }

    out_left  =  wet_percent_ * out_left  + dry_percent_ * in_left;
    out_right =  wet_percent_ * out_right + dry_percent_ * in_right;
}

//-----------------------------------------------------------------------------
ReverberationRoom &
ReverberationRoom::
operator=(const ReverberationRoom & rhs)
{
    if(this == & rhs)
    {
        return *this;
    }

    sample_rate_ = rhs.sample_rate_;
    wet_percent_ = rhs.wet_percent_;
    dry_percent_ = rhs.dry_percent_;

    for(uint32 i = 0; i < N_COMB_FILTERS_; ++i)
    {
        *comb_left_[i] = *(rhs.comb_left_[i]);
        *comb_right_[i] = *(rhs.comb_right_[i]);
    }

    for(uint32 i = 0; i < N_ALL_PASS_FILTERS_; ++i)
    {
        *allpass_left_[i] = *(rhs.allpass_left_[i]);
        *allpass_right_[i] = *(rhs.allpass_right_[i]);
    }

    return *this;
}

void
ReverberationRoom::
reset()
{
     for(uint32 i = 0; i < N_COMB_FILTERS_; ++i)
    {
        comb_left_[i]->reset();
        comb_right_[i]->reset();
    }

    for(uint32 i = 0; i < N_ALL_PASS_FILTERS_; ++i)
    {
        allpass_left_[i]->reset();
        allpass_right_[i]->reset();
    }
}

