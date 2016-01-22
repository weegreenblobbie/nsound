//-----------------------------------------------------------------------------
//
//  $Id: StreamOperators.h 879 2014-11-29 20:48:10Z weegreenblobbie $
//
//  Nsound is a C++ library and Python module for audio synthesis featuring
//  dynamic digital filters. Nsound lets you easily shape waveforms and write
//  to disk or plot them. Nsound aims to be as powerful as Csound but easy to
//  use.
//
//  Copyright (c) 2007 Nick Hilton
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

#ifndef _NSOUND_STREAM_OPERATORS_H_
#define _NSOUND_STREAM_OPERATORS_H_

#include <Nsound/Nsound.h>

#include <iostream>

namespace Nsound
{

// Read/write binary to/from streams.

std::ostream & operator&(std::ostream & out, char    value);
std::ostream & operator&(std::ostream & out, uint32  value);
std::ostream & operator&(std::ostream & out, uint64  value);
std::ostream & operator&(std::ostream & out, float32 value);
std::ostream & operator&(std::ostream & out, float64 value);

std::istream & operator&(std::istream & out, char    & value);
std::istream & operator&(std::istream & out, uint32  & value);
std::istream & operator&(std::istream & out, uint64  & value);
std::istream & operator&(std::istream & out, float32 & value);
std::istream & operator&(std::istream & out, float64 & value);

template <class T>
T peek(std::istream & in)
{
    T tmp = 0;

    std::streampos pos = in.tellg();

    in.read(static_cast<char *>(&tmp), sizeof(T));

    in.seekg(pos);

    return tmp;
}

} // namespace

// :mode=c++: jEdit modeline
#endif
