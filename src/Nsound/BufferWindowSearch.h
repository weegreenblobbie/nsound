//-----------------------------------------------------------------------------
//
//  $Id: BufferWindowSearch.h 874 2014-09-08 02:21:29Z weegreenblobbie $
//
//  Nsound is a C++ library and Python module for audio synthesis featuring
//  dynamic digital filters. Nsound lets you easily shape waveforms and write
//  to disk or plot them. Nsound aims to be as powerful as Csound but easy to
//  use.
//
//  Copyright (c) 2009-Present Nick Hilton
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
#ifndef _NSOUND_BUFFER_WINDOW_SEARCH_H_
#define _NSOUND_BUFFER_WINDOW_SEARCH_H_

#include <Nsound/Nsound.h>

namespace Nsound
{

// Forward
class Buffer;

//-----------------------------------------------------------------------------
//! Searches the target Buffer for zero crossings at or after the window_size position.
class BufferWindowSearch
{
    public:

    BufferWindowSearch(const Buffer & buffer, uint32 window_size = 2048);

    BufferWindowSearch(const BufferWindowSearch & copy);

    ~BufferWindowSearch(){};

    BufferWindowSearch &
    operator=(const BufferWindowSearch & rhs);

    //! Searches the target Buffer for a zero crossing at or after the window_size position.
    Buffer
    getNextWindow();

    //! Returns how many samples are left in the target Buffer.
    uint32
    getSamplesLeft() const;

    //! Resets the search.
    void
    reset()
    {position_ = 0;};

    //! Search a different Buffer.
    void
    setBuffer(const Buffer & buffer)
    {
        target_buffer_ = &buffer;
        reset();
    }

    protected:

    const Buffer * target_buffer_;
    uint32 window_size_;
    uint32 position_;
};

};

#endif
