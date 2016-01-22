//-----------------------------------------------------------------------------
//
//  $Id: Kernel.h 874 2014-09-08 02:21:29Z weegreenblobbie $
//
//  Nsound is a C++ library and Python module for audio synthesis featuring
//  dynamic digital filters. Nsound lets you easily shape waveforms and write
//  to disk or plot them. Nsound aims to be as powerful as Csound but easy to
//  use.
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
#ifndef _NSOUND_KERNEL_H_
#define _NSOUND_KERNEL_H_

#include <Nsound/Nsound.h>

#include <iostream>
#include <set>

namespace Nsound
{

// Forward declarations
class Buffer;

//-----------------------------------------------------------------------------
//! WARNING: This is Experimental, you should not use this class as it may not
//! be working or will change in future releases of Nsound.
class Kernel
{
    public:

    Kernel(uint32 b_length, uint32 a_length,
        int32 f1 = 0,
        int32 f2 = 0,
        int32 f3 = 0,
        int32 f4 = 0);

    Kernel(const Kernel & copy);

    virtual
    ~Kernel();

    boolean
    operator<(const Kernel & rhs) const;

    //! Returns kernel 'a[i]' coef.
    float64
    getA(uint32 i)
    { if(i < a_length_) return a_[i]; return 0.0; };

    //! Returns Kernel 'a' array, the length is returned by getALegnth()
    float64 *
    getA()
    { return a_; };

    //! Returns Kernel 'a' array, the length is returned by getALegnth()
    const
    float64 *
    getA() const
    { return a_; };

    //! Returns kernel 'b[i]' coef.
    float64
    getB(uint32 i)
    { if(i < b_length_) return b_[i]; return 0.0; };

    //! Returns kernel 'b' array, the length is returned by getBLegnth()
    float64 *
    getB()
    { return b_; };

    //! Returns kernel 'b' array, the length is returned by getBLegnth()
    const
    float64 *
    getB() const
    { return b_; };

    uint32
    getALength() const { return a_length_; };

    uint32
    getBLength() const { return b_length_; };

    float64
    getSum() const;

    Kernel &
    operator=(const Kernel & rhs);

    //! Prints the coeffents.
    friend
    std::ostream &
    operator<<(std::ostream & out, const Kernel & rhs);

    //! Sets all coefs to random float64 values between min & max.
    void
    randomize(const float64 & min = -1.0, const float64 & max = 1.0);

    //! Copy getALength() values from the array a into the Kernel.  Use setLength() to set the number of values to copy.
    void
    setA(const float64 * a);

    //! Set a_[i] = d
    void
    setA(const float64 & d, uint32 i)
    { if(i < a_length_) a_[i] = d; };

    //! Copy getALength() values from the array a into the Kernel.  Use setLength() to set the number of values to copy.
    void
    setA(const Buffer & a);

    //! Copy getBLength() values from the array a into the Kernel.  Use setLength() to set the number of values to copy.
    void
    setB(const float64 * b);

    //! Set b_[i] = d
    void
    setB(const float64 & d, uint32 i)
    { if(i < b_length_) b_[i] = d; };

    //! Copy getALength() values from the array a into the Kernel.  Use setLength() to set the number of values to copy.
    void
    setB(const Buffer & b);

    void
    setLength(uint32 b_length, uint32 a_length);

    //! Genitic Algorithm Helper Function: swaps the A & B coefs.
    void
    ga_swap_ab(Kernel & rhs);

    //! Genitic Algorithm Helper Function: interleave B coefs with B, A with A
    void
    ga_interleave(Kernel & rhs);

    //! Genitic Algorithm Helper Function: interleave B coefs with A
    void
    ga_interleave();

    protected:

    uint32  b_length_;
    uint32  a_length_;

    int32 f1_;
    int32 f2_;
    int32 f3_;
    int32 f4_;

    float64 * b_;
    float64 * a_;

};

std::ostream &
operator<<(std::ostream & out, const Kernel & rhs);

typedef std::set<Kernel> KernelCache;

};

// :mode=c++: jEdit modeline

#endif
