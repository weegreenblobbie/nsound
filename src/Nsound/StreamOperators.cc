//-----------------------------------------------------------------------------
//
//  $Id: StreamOperators.cc 879 2014-11-29 20:48:10Z weegreenblobbie $
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

#include <Nsound/StreamOperators.h>

namespace Nsound
{

template <class T>
std::ostream &
_write(std::ostream & out, T value)
{
    out.write(reinterpret_cast<char *>(&value), sizeof(T));
    return out;
}

std::ostream &
operator&(std::ostream & out, char value) { return _write(out, value); }

std::ostream &
operator&(std::ostream & out, uint32 value) { return _write(out, value); }

std::ostream &
operator&(std::ostream & out, uint64 value) { return _write(out, value); }

std::ostream &
operator&(std::ostream & out, float32 value) { return _write(out, value); }

std::ostream &
operator&(std::ostream & out, float64 value) { return _write(out, value); }

template <class T>
std::istream &
_read(std::istream & in, T & value)
{
    in.read(reinterpret_cast<char *>(&value), sizeof(T));
    return in;
}

std::istream &
operator&(std::istream & in, char & value) { return _read(in, value); }

std::istream &
operator&(std::istream & in, uint32 & value) { return _read(in, value); }

std::istream &
operator&(std::istream & in, uint64 & value) { return _read(in, value); }

std::istream &
operator&(std::istream & in, float32 & value) { return _read(in, value); }

std::istream &
operator&(std::istream & in, float64 & value) { return _read(in, value); }

} // namespace