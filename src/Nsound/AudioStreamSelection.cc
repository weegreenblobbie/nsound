//-----------------------------------------------------------------------------
//
//  $Id: AudioStreamSelection.cc 912 2015-07-26 00:50:29Z weegreenblobbie $
//
//  Nsound is a C++ library and Python module for audio synthesis featuring
//  dynamic digital filters. Nsound lets you easily shape waveforms and write
//  to disk or plot them. Nsound aims to be as powerful as Csound but easy to
//  use.
//
//  Copyright (c) 2004-2007 Nick Hilton
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

using namespace Nsound;

Nsound::
AudioStreamSelection::
AudioStreamSelection(AudioStream & as, const BooleanVectorVector & bv)
    :
    target_as_(&as),
    bv_(bv)
{
}

Nsound::
AudioStreamSelection::
AudioStreamSelection(const AudioStreamSelection & copy)
    :
    target_as_(copy.target_as_),
    bv_(copy.bv_)
{
}

AudioStreamSelection &
Nsound::
AudioStreamSelection::
operator=(const AudioStreamSelection & rhs)
{
    if(this == & rhs)
    {
        return *this;
    }

    target_as_ = rhs.target_as_;
    bv_ = rhs.bv_;
    return *this;
}


AudioStreamSelection &
Nsound::
AudioStreamSelection::
operator+=(const float64 & rhs)
{
    uint32 n_channels = target_as_->getNChannels();
    uint32 n_bv = static_cast<uint32>(bv_.size());

    for(uint32 i = 0; i < n_channels && i < n_bv; ++i)
    {
        (*target_as_)[i](bv_[i]) += rhs;
    }

    return *this;
}

AudioStreamSelection &
Nsound::
AudioStreamSelection::
operator-=(const float64 & rhs)
{
    uint32 n_channels = target_as_->getNChannels();
    uint32 n_bv = static_cast<uint32>(bv_.size());

    for(uint32 i = 0; i < n_channels && i < n_bv; ++i)
    {
        (*target_as_)[i](bv_[i]) -= rhs;
    }

    return *this;
}

AudioStreamSelection &
Nsound::
AudioStreamSelection::
operator*=(const float64 & rhs)
{
    uint32 n_channels = target_as_->getNChannels();
    uint32 n_bv = static_cast<uint32>(bv_.size());

    for(uint32 i = 0; i < n_channels && i < n_bv; ++i)
    {
        (*target_as_)[i](bv_[i]) *= rhs;
    }

    return *this;
}

AudioStreamSelection &
Nsound::
AudioStreamSelection::
operator/=(const float64 & rhs)
{
    uint32 n_channels = target_as_->getNChannels();
    uint32 n_bv = static_cast<uint32>(bv_.size());

    for(uint32 i = 0; i < n_channels && i < n_bv; ++i)
    {
        (*target_as_)[i](bv_[i]) /= rhs;
    }

    return *this;
}

AudioStreamSelection &
Nsound::
AudioStreamSelection::
operator^=(const float64 & rhs)
{
    uint32 n_channels = target_as_->getNChannels();
    uint32 n_bv = static_cast<uint32>(bv_.size());

    for(uint32 i = 0; i < n_channels && i < n_bv; ++i)
    {
        (*target_as_)[i](bv_[i]) ^= rhs;
    }

    return *this;
}

AudioStreamSelection &
Nsound::
AudioStreamSelection::
operator=(const float64 & rhs)
{
    uint32 n_channels = target_as_->getNChannels();
    uint32 n_bv = static_cast<uint32>(bv_.size());

    for(uint32 i = 0; i < n_channels && i < n_bv; ++i)
    {
        (*target_as_)[i](bv_[i]) = rhs;
    }

    return *this;
}

