//-----------------------------------------------------------------------------
//
//  $Id: RngTausworthe.cc 887 2015-04-26 02:49:19Z weegreenblobbie $
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

#include <Nsound/RngTausworthe.h>

#include <cmath>
#include <ctime>

using namespace Nsound;

using std::abs;


RngTausworthe::
RngTausworthe()
    :
    s1_(0),
    s2_(0),
    s3_(0)
{
    setSeed(static_cast<uint32>(time(NULL)));
}

uint32
RngTausworthe::
get()
{
    #define MASK 0xffffffffUL
    #define TAUSWORTHE(s,a,b,c,d) (((s & c) << d) & MASK) ^ ((((s << a) & MASK) ^ s) >> b)

    s1_ = TAUSWORTHE(s1_, 13, 19, 4294967294UL, 12);
    s2_ = TAUSWORTHE(s2_,  2, 25, 4294967288UL,  4);
    s3_ = TAUSWORTHE(s3_,  3, 11, 4294967280UL, 17);

    uint32 r = s1_ ^ s2_ ^ s3_;

    return r;
}

int32
RngTausworthe::
get(
    const int32 min,
    const int32 max)
{
    return static_cast<int32>(
        get(
            static_cast<float64>(min),
            static_cast<float64>(max)));
}

float64
RngTausworthe::
get(
    const float64 & min,
    const float64 & max)
{
    float64 mag = abs(max - min);
    float64 d = mag * (static_cast<float64>(get()) / 4294967296.0) + min;

    return d;
}


RngTausworthe &
RngTausworthe::
operator=(const RngTausworthe & rhs)
{
    if(this == &rhs)
    {
        return *this;
    }

    s1_ = rhs.s1_;
    s2_ = rhs.s2_;
    s3_ = rhs.s3_;

    return * this;
}

void
RngTausworthe::
setSeed(uint32 seed)
{
    uint32 s = seed;

    // Set default seed.
    if(s == 0)
    {
        s = 1;
    }

    #define LCG(n) ((69069 * n) & 0xffffffffUL)
    s1_ = LCG(s);
    s2_ = LCG(s1_);
    s3_ = LCG(s2_);

    // "warm it up"
    get();
    get();
    get();
    get();
    get();
    get();
}



