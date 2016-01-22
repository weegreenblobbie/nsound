//-----------------------------------------------------------------------------
//
//  $Id: Filter.h 911 2015-07-10 03:04:24Z weegreenblobbie $
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

#ifndef _NSOUND_INTERFACES_HPP_
#define _NSOUND_INTERFACES_HPP_

#include <iostream>

#include <Nsound/Nsound.h>


namespace nsound
{


//-----------------------------------------------------------------------------
// Render mode

enum RenderMode { OFFLINE, REALTIME };


std::string to_string(const RenderMode rm);
std::ostream & operator << (std::ostream &, const RenderMode & rm);


class RenderModal
{
public:
    virtual RenderMode render_mode() = 0;
    virtual void render_mode(RenderMode) = 0;
};


//-----------------------------------------------------------------------------
// Producers / generators

template <class T>
class Callable
{
public:
    virtual T operator() = 0;
    virtual bool finished() const = 0;
};


//-----------------------------------------------------------------------------
// Inline implementation

std::string to_string(const RenderMode & rm)
{
    switch(rm)
    {
        case OFFLINE: return "offline";
        case REALTIME: return "realtime";
    }

    return "/*error*/";
}


std::ostream & operator << (std::ostream & out, const RenderMode & rm)
{
    return out << to_string(rm);
}


} // namespace

#endif

// :mode=c++: jEdit modeline