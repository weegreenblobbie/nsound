//-----------------------------------------------------------------------------
//
//  $Id: BufferSelection.cc 912 2015-07-26 00:50:29Z weegreenblobbie $
//
//  Nsound is a C++ library and Python module for audio synthesis featuring
//  dynamic digital filters. Nsound lets you easily shape waveforms and write
//  to disk or plot them. Nsound aims to be as powerful as Csound but easy to
//  use.
//
//  Copyright (c) 2009-Present Nick Hilton
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

#include <Nsound/Buffer.h>

using namespace Nsound;

Nsound::
BufferSelection::
BufferSelection(Buffer & buffer, const BooleanVector & bv)
    :
    target_buffer_(&buffer),
    bv_(bv)
{
};

Nsound::
BufferSelection::
BufferSelection(const BufferSelection & copy)
    :
    target_buffer_(copy.target_buffer_),
    bv_(copy.bv_)
{
};

BufferSelection &
Nsound::
BufferSelection::
operator=(const BufferSelection & rhs)
{
    if(this == & rhs)
    {
        return *this;
    }

    target_buffer_ = rhs.target_buffer_;
    bv_ = rhs.bv_;
    return *this;
};

BufferSelection &
Nsound::
BufferSelection::
operator+=(const float64 & rhs)
{
    Buffer::iterator b = target_buffer_->begin();
    Buffer::iterator b_end = target_buffer_->end();

    BooleanVector::iterator bv     = bv_.begin();
    BooleanVector::iterator bv_end = bv_.end();

    while(b != b_end && bv != bv_end)
    {
        if(*bv) *b += rhs;
        ++b;
        ++bv;
    }

    return *this;
}

BufferSelection &
Nsound::
BufferSelection::
operator-=(const float64 & rhs)
{
    Buffer::iterator b = target_buffer_->begin();
    Buffer::iterator b_end = target_buffer_->end();

    BooleanVector::iterator bv     = bv_.begin();
    BooleanVector::iterator bv_end = bv_.end();

    while(b != b_end && bv != bv_end)
    {
        if(*bv) *b -= rhs;
        ++b;
        ++bv;
    }

    return *this;
}

BufferSelection &
Nsound::
BufferSelection::
operator*=(const float64 & rhs)
{
    Buffer::iterator b = this->target_buffer_->begin();
    Buffer::iterator b_end = this->target_buffer_->end();

    BooleanVector::iterator bv     = this->bv_.begin();
    BooleanVector::iterator bv_end = this->bv_.end();

    while(b != b_end && bv != bv_end)
    {
        if(*bv) *b *= rhs;
        ++b;
        ++bv;
    }

    return *this;
}

BufferSelection &
Nsound::
BufferSelection::
operator/=(const float64 & rhs)
{
    Buffer::iterator b = this->target_buffer_->begin();
    Buffer::iterator b_end = this->target_buffer_->end();

    BooleanVector::iterator bv     = this->bv_.begin();
    BooleanVector::iterator bv_end = this->bv_.end();

    while(b != b_end && bv != bv_end)
    {
        if(*bv) *b /= rhs;
        ++b;
        ++bv;
    }

    return *this;
}

BufferSelection &
Nsound::
BufferSelection::
operator^=(const float64 & rhs)
{
    Buffer::iterator b = this->target_buffer_->begin();
    Buffer::iterator b_end = this->target_buffer_->end();

    BooleanVector::iterator bv     = this->bv_.begin();
    BooleanVector::iterator bv_end = this->bv_.end();

    while(b != b_end && bv != bv_end)
    {
        if(*bv) *b = std::pow(*b, rhs);
        ++b;
        ++bv;
    }

    return *this;
}

BufferSelection &
Nsound::
BufferSelection::
operator=(const float64 & rhs)
{
    Buffer::iterator b = this->target_buffer_->begin();
    Buffer::iterator b_end = this->target_buffer_->end();

    BooleanVector::iterator bv     = this->bv_.begin();
    BooleanVector::iterator bv_end = this->bv_.end();

    while(b != b_end && bv != bv_end)
    {
        if(*bv) *b = rhs;
        ++b;
        ++bv;
    }

    return *this;
}

