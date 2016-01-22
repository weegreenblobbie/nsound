//-----------------------------------------------------------------------------
//
//  $Id: Kernel.cc 874 2014-09-08 02:21:29Z weegreenblobbie $
//
//  Copyright (c) 2009 to Present Nick Hilton
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

#include <Nsound/Buffer.h>
#include <Nsound/Kernel.h>
#include <Nsound/RngTausworthe.h>

#include <string.h>
#include <cmath>
#include <iomanip>
#include <iostream>

using namespace Nsound;

using std::cerr;
using std::endl;
using std::setw;
using std::setprecision;
using std::showpoint;

//-----------------------------------------------------------------------------
Kernel::
Kernel(uint32 b_length, uint32 a_length,
    int32 f1,
    int32 f2,
    int32 f3,
    int32 f4)
    :
    b_length_(b_length),
    a_length_(a_length),
    f1_(f1),
    f2_(f2),
    f3_(f3),
    f4_(f4),
    b_(NULL),
    a_(NULL)
{
    b_ = new float64 [b_length];
    a_ = new float64 [a_length];
}

//-----------------------------------------------------------------------------
Kernel::
Kernel(const Kernel & copy)
    :
    b_length_(copy.b_length_),
    a_length_(copy.a_length_),
    f1_(copy.f1_),
    f2_(copy.f2_),
    f3_(copy.f3_),
    f4_(copy.f4_),
    b_(NULL),
    a_(NULL)
{
    b_ = new float64 [b_length_];
    a_ = new float64 [a_length_];

    *this = copy;
}

//-----------------------------------------------------------------------------
Kernel::
~Kernel()
{
    delete [] b_;
    delete [] a_;
}

//-----------------------------------------------------------------------------
Kernel &
Kernel::
operator=(const Kernel & rhs)
{
    if(this == & rhs)
    {
        return *this;
    }

    setLength(rhs.b_length_, rhs.a_length_);

    memcpy(b_, rhs.b_, sizeof(float64) * b_length_);
    memcpy(a_, rhs.a_, sizeof(float64) * a_length_);

    f1_ = rhs.f1_;
    f2_ = rhs.f2_;
    f3_ = rhs.f3_;
    f4_ = rhs.f4_;

    return *this;
}

boolean
Kernel::
operator<(const Kernel & rhs) const
{
    return (b_length_ < rhs.b_length_)  ||

           (b_length_ == rhs.b_length_  &&
            a_length_  < rhs.a_length_) ||

           (b_length_ == rhs.b_length_  &&
            a_length_ == rhs.a_length_  &&
            f1_        < rhs.f1_)       ||

           (b_length_ == rhs.b_length_  &&
            a_length_ == rhs.a_length_  &&
            f1_       == rhs.f1_        &&
            f2_        < rhs.f2_)       ||

           (b_length_ == rhs.b_length_  &&
            a_length_ == rhs.a_length_  &&
            f1_       == rhs.f1_        &&
            f2_       == rhs.f2_        &&
            f3_        < rhs.f3_)       ||

           (b_length_ == rhs.b_length_  &&
            a_length_ == rhs.a_length_  &&
            f1_       == rhs.f1_        &&
            f2_       == rhs.f2_        &&
            f3_       == rhs.f3_        &&
            f4_        < rhs.f4_)       ||

           false;

}

float64
Kernel::
getSum() const
{
    float64 sum = 0.0;

    for(uint32 i = 0; i < b_length_; ++i)
    {
        sum += std::fabs(b_[i]);
    }

    for(uint32 i = 0; i < a_length_; ++i)
    {
        sum += std::fabs(a_[i]);
    }

    return sum;
}

//-----------------------------------------------------------------------------
std::ostream &
Nsound::
operator<<(std::ostream & out, const Kernel & rhs)
{
    #define FORMAT setw(12) << setprecision(9) << std::setiosflags(std::ios::fixed)

    for(uint32 i = 0; i < rhs.b_length_; ++i)
    {
        out << "b[" << i << "] = " << FORMAT << rhs.b_[i];

        if(i < rhs.a_length_)
        {
            out << "        a[" << i << "] = " << FORMAT << rhs.a_[i];
        }
        out << std::endl;
    }

    return out;
}

void
Kernel::
randomize(const float64 & min, const float64 & max)
{
    RngTausworthe rng;

    for(uint32 i = 0; i < b_length_; ++i)
    {
        b_[i] = rng.get(min, max);
    }

    for(uint32 i = 0; i < a_length_; ++i)
    {
        a_[i] = rng.get(min, max);
    }
}

void
Kernel::
setA(const float64 * a)
{
    memcpy(a_, a, sizeof(float64) * a_length_);
}

void
Kernel::
setA(const Buffer & a)
{
    uint32 a_len = a.getLength();

    for(uint32 i = 0; i < a_length_; ++i)
    {
        if(i < a_len)
        {
            a_[i] = a[i];
        }
    }
}

void
Kernel::
setB(const float64 * b)
{
    memcpy(b_, b, sizeof(float64) * b_length_);
}

void
Kernel::
setB(const Buffer & b)
{
    uint32 b_len = b.getLength();

    for(uint32 i = 0; i < b_length_; ++i)
    {
        if(i < b_len)
        {
            b_[i] = b[i];
        }
    }
}

void
Kernel::
setLength(uint32 b_length, uint32 a_length)
{
    if(b_length_ != b_length)
    {
        b_length_ = b_length;
        delete [] b_;
        b_ = new float64 [b_length_];
    }

    if(a_length_ != a_length)
    {
        a_length_ = a_length;
        delete [] a_;
        a_ = new float64 [a_length_];
    }

}

void
Kernel::
ga_swap_ab(Kernel & rhs)
{
    Kernel temp(rhs);

    rhs.setA(getB());
    rhs.setB(getA());

    setA(temp.getB());
    setB(temp.getA());
}

void
swap_(float64 & a, float64 & b)
{
    float64 temp = b;
    b = a;
    a = temp;
}

void
Kernel::
ga_interleave(Kernel & rhs)
{
    Kernel temp(rhs);

    for(uint32 i = 0; i < b_length_; ++i)
    {
        if(i % 2)
        {
            swap_(b_[i], rhs.b_[i]);
        }
    }

    for(uint32 i = 0; i < a_length_; ++i)
    {
        if(i % 2)
        {
            swap_(a_[i], rhs.a_[i]);
        }
    }

}

void
Kernel::
ga_interleave()
{
    for(uint32 i = 0; i < b_length_; ++i)
    {
        if(i % 2 && i < a_length_)
        {
            swap_(b_[i], a_[i]);
        }
    }

}



