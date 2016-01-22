//-----------------------------------------------------------------------------
//
//  $Id: CircularIterators.h 916 2015-08-22 16:31:39Z weegreenblobbie $
//
//  Nsound is a C++ library and Python module for audio synthesis featuring
//  dynamic digital filters. Nsound lets you easily shape waveforms and write
//  to disk or plot them. Nsound aims to be as powerful as Csound but easy to
//  use.
//
//  Copyright (c) 2004-Present Nick Hilton
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
#ifndef _NSOUND_CIRCULAR_ITERATORS_H_
#define _NSOUND_CIRCULAR_ITERATORS_H_

#include <Nsound/Nsound.h>

#include <iterator>
#include <vector>

namespace Nsound
{

typedef std::vector< uint32 > Uint32Vector;
typedef std::vector< float64 > FloatVector;

//! A circulator iterator for class Buffer.
//
//! This iterator should behave like a normal std::vector::iterator, but
//! when the iterator reaches the end of the vector, it will automatically
//! wrap around back to the front.
//!
//! \par Example:
//! \code
//! // C++
//! Buffer b1("california.wav");
//! Buffer b2;
//! Buffer::circular_iterator ctor = b1.cbegin();
//! for(i = 0; i < 1e6; ++i) b2 << *ctor;
//!
//! // Python, not currently supported.
//! \endcode
class circular_iterator
{

public:

    circular_iterator(FloatVector & v)
        :
        end_(v.end()),
        itor_(v.begin()),
        data_(&v)
    {}

    circular_iterator(const circular_iterator & copy)
        :
        end_(copy.end_),
        itor_(copy.itor_),
        data_(copy.data_)
    {}

    circular_iterator & operator=(const circular_iterator & rhs)
    {
        if(this == &rhs) return *this;

        end_ = rhs.end_;
        itor_ = rhs.itor_;
        data_ = rhs.data_;

        return *this;
    }

    void reset() { itor_ = data_->begin(); }

    bool operator == (const circular_iterator & rhs) const
    {
        return
            data_ == rhs.data_ &&
            itor_ == rhs.itor_;
    }

    bool operator != (const circular_iterator & rhs) const
    {
        return
            data_ != rhs.data_ ||
            itor_ != rhs.itor_;
    }

    circular_iterator & operator++()
    {
        ++itor_;

        if(itor_ >= end_) itor_ = data_->begin();

        return *this;
    }

    circular_iterator & operator--()
    {
        if(itor_ == data_->begin()) itor_ = data_->end() - 1;
        else                      --itor_;

        return *this;
    }

    circular_iterator operator++(int)
    {
        circular_iterator tmp(*this);
        operator++();
        return tmp;
    }

    circular_iterator & operator+=(int32 i)
    {
        if(i < 0) return *this -= -i;
        auto pos = std::distance(data_->begin(), itor_);
        pos = (pos + i) % data_->size();
        itor_ = data_->begin() + pos;
        return *this;
    }

    circular_iterator & operator-=(int32 i)
    {
        if(i < 0) return *this += -i;
        auto pos = std::distance(data_->begin(), itor_);
        pos -= (i % data_->size());
        if(pos < 0) pos += data_->size();
        itor_ = data_->begin() + pos;
        return *this;
    }

    FloatVector::value_type & operator*()
    {
        return *itor_;
    }

    float64 __float__() const
    {
        return *itor_;
    }

    std::string __str__() const
    {
        std::stringstream ss;

        ss
            << "Nsound::Buffer::circular_iterator(vector at 0x0"
            << std::hex
            << reinterpret_cast<uint64>(data_)
            << ", index = ";

        uint64 pos = itor_ - data_->begin();

        ss << std::dec << pos << ")";

        return ss.str();
    }

private:

    FloatVector::const_iterator end_;
    FloatVector::iterator       itor_;
    FloatVector *               data_;
};


inline
circular_iterator operator+(const circular_iterator & lhs, int32 rhs)
{
    circular_iterator tmp(lhs);

    tmp += rhs;

    return tmp;
}


inline
circular_iterator operator-(const circular_iterator & lhs, int32 rhs)
{
    circular_iterator tmp(lhs);

    tmp -= rhs;

    return tmp;
}


// Maybe one day I'll figure out how to define only one class for both
// const and non-const.

class const_circular_iterator
{
public:

    const_circular_iterator(const FloatVector & v)
        :
        end_(v.end()),
        itor_(v.begin()),
        data_(const_cast<FloatVector*>(&v))
    {}

    const_circular_iterator(const const_circular_iterator & copy)
        :
        end_(copy.end_),
        itor_(copy.itor_),
        data_(copy.data_)
    {}

    const_circular_iterator & operator=(const const_circular_iterator & rhs)
    {
        if(this == &rhs) return *this;

        end_ = rhs.end_;
        itor_ = rhs.itor_;
        data_ = rhs.data_;

        return *this;
    }

    void reset() { itor_ = data_->begin(); }

    bool operator == (const const_circular_iterator & rhs) const
    {
        return
            data_ == rhs.data_ &&
            itor_ == rhs.itor_;
    }

    bool operator != (const const_circular_iterator & rhs) const
    {
        return
            data_ != rhs.data_ ||
            itor_ != rhs.itor_;
    }

    const_circular_iterator & operator++()
    {
        ++itor_;

        if(itor_ == end_) itor_ = data_->begin();

        return *this;
    }

    const_circular_iterator & operator--()
    {
        if(itor_ == data_->begin()) itor_ = data_->end() - 1;
        else                      --itor_;

        return *this;
    }

    const_circular_iterator operator++(int)
    {
        const_circular_iterator tmp(*this);
        operator++();
        return tmp;
    }

    const_circular_iterator & operator+=(int32 i)
    {
        if(i < 0) return *this -= -i;
        auto pos = std::distance(const_cast<const FloatVector *>(data_)->begin(), itor_);
        pos = (pos + i) % data_->size();
        itor_ = data_->begin() + pos;
        return *this;
    }

    const_circular_iterator & operator-=(int32 i)
    {
        if(i < 0) return *this += -i;
        auto pos = std::distance(const_cast<const FloatVector *>(data_)->begin(), itor_);
        pos -= (i % data_->size());
        if(pos < 0) pos += data_->size();
        itor_ = data_->begin() + pos;
        return *this;
    }

    const FloatVector::value_type & operator*()
    {
        return *itor_;
    }

    float64 __float__() const
    {
        return *itor_;
    }

private:

    FloatVector::const_iterator end_;
    FloatVector::const_iterator itor_;
    FloatVector *         data_;
};


inline
const_circular_iterator operator+(const const_circular_iterator & lhs, int32 rhs)
{
    const_circular_iterator tmp(lhs);

    tmp += rhs;

    return tmp;
}


inline
const_circular_iterator operator-(const const_circular_iterator & lhs, int32 rhs)
{
    const_circular_iterator tmp(lhs);

    tmp -= rhs;

    return tmp;
}

} // namespace

// :mode=c++: jEdit modeline
#endif
