//-----------------------------------------------------------------------------
//
//  $Id: CircularBuffer.h 878 2014-11-23 04:51:23Z weegreenblobbie $
//
//  Nsound is a C++ library and Python module for audio synthesis featuring
//  dynamic digital filters. Nsound lets you easily shape waveforms and write
//  to disk or plot them. Nsound aims to be as powerful as Csound but easy to
//  use.
//
//  Copyright (c) 2008-Present Nick Hilton
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
#ifndef _NSOUND_CIRCULAR_BUFFER_H_
#define _NSOUND_CIRCULAR_BUFFER_H_

namespace Nsound
{

class Buffer;

class CircularBuffer
{
    public:

    CircularBuffer(uint32 n_samples);
    ~CircularBuffer();

    Buffer read() const;

    void write(float64 d);
    void write(const AudioStream & as);
    void write(const Buffer & b);

    protected:

    Buffer * buffer_;
    Buffer::circular_iterator * itor_;

    private:

    // FIXME: currently Buffer::circular_iterator isn't copyable

    CircularBuffer(const CircularBuffer & copy);
    CircularBuffer & operator=(const CircularBuffer & rhs);

};

} // namespace

// :mode=c++: jEdit modeline
#endif
