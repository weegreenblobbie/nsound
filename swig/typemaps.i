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

// Return a bytearry from binary string.
%typemap(out) Nsound::bytearray
{
    // %typemap(out) Nsound::bytearray (C++ to Python)

    $result = PyByteArray_FromStringAndSize($1.c_str(), $1.size());
}


// Return a binary string from bytearray.
%typemap(in) (const void * data, std::size_t size)
{
    // %typemap(in) (const void * data, std::size_t size) (Python to C++)
    $1 = PyByteArray_AsString($input);
    $2 = PyByteArray_Size($input);
}

// :mode=python:
