//-----------------------------------------------------------------------------
//
//  $Id: CircularBuffer.cc 878 2014-11-23 04:51:23Z weegreenblobbie $
//
//  Copyright (c) 2008 Nick Hilton
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

#include <Nsound/AudioStream.h>
#include <Nsound/Buffer.h>
#include <Nsound/CircularBuffer.h>

using namespace Nsound;

using std::cerr;
using std::endl;

CircularBuffer::
CircularBuffer(uint32 n_samples)
    :
    buffer_(std::make_shared<Buffer>(Buffer::zeros(n_samples))),
    itor_(std::make_shared<Buffer::circular_iterator>(buffer_->cbegin()))
{
}


Buffer
CircularBuffer::
read() const
{
    const uint32 N = buffer_->getLength();

    Buffer::circular_iterator ci(*itor_);

    Buffer b(N);

    for(uint32 i = 0; i < N; ++i)
    {
        b << *ci;
        ++ci;
    }

    return b;
}


void
CircularBuffer::
write(const AudioStream & as)
{
    write(as.getMono()[0]);
}


void
CircularBuffer::
write(const Buffer & src)
{
    for(auto x : src)
    {
        write(x);
    }
}
