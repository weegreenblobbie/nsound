//-----------------------------------------------------------------------------
//
//  $Id: TicToc.cc 874 2014-09-08 02:21:29Z weegreenblobbie $
//
//  Copyright (c) 2004-2006 Nick Hilton
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

#include <Nsound/TicToc.h>

#include <ctime>

namespace Nsound
{
    static double start_time = 0.0;

    void Tic()
    {
        start_time = clock();
    }

    Nsound::float64 Toc()
    {
        return ((clock() - start_time) / static_cast<float64>(CLOCKS_PER_SEC));
    }
};
