//-----------------------------------------------------------------------------
//
//  $Id: BufferSelection.h 912 2015-07-26 00:50:29Z weegreenblobbie $
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
#ifndef _NSOUND_BUFFER_SELECTION_H_
#define _NSOUND_BUFFER_SELECTION_H_

#include <Nsound/Nsound.h>

#include <vector>

namespace Nsound
{

// Forward
class Buffer;

typedef std::vector< boolean > BooleanVector;

//-----------------------------------------------------------------------------
//! A helper class for advance operators.
//
//! The intent for this class is to allow some "Matlab" like expressions.
//!
//! \par Example:
//! \code
//! // C++
//! Buffer b1;
//! b1 << "california.wav";
//!
//! // limit anthing outsize of +/- 0.3
//! b1(b1 > 0.3) = 0.3;
//! b1(b1 < -0.3) = -0.3;
//!
//! // Python
//! b1 = Buffer()
//! b1 << "california.wav"
//! b1(b1 > 0.3).set(0.3)
//! b1(b1 < -0.3).set(-0.3)
//! \endcode
class BufferSelection
{

public:

    BufferSelection(Buffer & buffer, const BooleanVector & bv);
    BufferSelection(const BufferSelection & copy);
    BufferSelection & operator=(const BufferSelection & rhs);
    ~BufferSelection(){};

    BufferSelection & operator+=(const float64 & rhs);
    BufferSelection & operator-=(const float64 & rhs);
    BufferSelection & operator*=(const float64 & rhs);
    BufferSelection & operator/=(const float64 & rhs);
    BufferSelection & operator^=(const float64 & rhs);
    BufferSelection & operator=(const float64 & rhs);

    // For SWIG
    void set(const float64 & rhs){this->operator=(rhs);};

protected:

    Buffer * target_buffer_;
    BooleanVector bv_;
};

};

#endif
