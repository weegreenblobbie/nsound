//-----------------------------------------------------------------------------
//
//  $Id: AudioBackend.cc 874 2014-09-08 02:21:29Z weegreenblobbie $
//
//  Copyright (c) 2005-2006 Nick Hilton
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

#include <Nsound/AudioBackend.h>

using namespace Nsound;

std::string state_strings[3] =
{
    "Backend Not Initialized",
    "Backend Ready",
    "Backend Error"
};

//-----------------------------------------------------------------------------
std::string
AudioBackend::
getStateString()
{
    return state_strings[state_];
}

//-----------------------------------------------------------------------------
std::string
AudioBackend::
getStateString(const State & state)
{
    if( state >= BACKEND_NOT_INITIALIZED &&
        state <= BACKEND_ERROR)
    {
        return state_strings[state];
    }

    return "Unknown Backend State";
}


// :mode=c++:
