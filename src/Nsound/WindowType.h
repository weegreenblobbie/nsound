//-----------------------------------------------------------------------------
//
//  $Id: WindowType.h 874 2014-09-08 02:21:29Z weegreenblobbie $
//
//  Nsound is a C++ library and Python module for audio synthesis featuring
//  dynamic digital filters. Nsound lets you easily shape waveforms and write
//  to disk or plot them. Nsound aims to be as powerful as Csound but easy to
//  use.
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
#ifndef _NSOUND_WINDOW_TYPE_H_
#define _NSOUND_WINDOW_TYPE_H_

namespace Nsound
{

    enum WindowType
    {
        BARTLETT,
        BLACKMAN,
        BLACKMAN_HARRIS,
        GAUSSIAN,
        GAUSSIAN_05,
        GAUSSIAN_10,
        GAUSSIAN_15,
        GAUSSIAN_20,
        GAUSSIAN_25,
        GAUSSIAN_30,
        GAUSSIAN_33,
        GAUSSIAN_35,
        GAUSSIAN_40,
        GAUSSIAN_45,
        GAUSSIAN_50,
        GAUSSIAN_55,
        GAUSSIAN_60,
        GAUSSIAN_65,
        GAUSSIAN_66,
        GAUSSIAN_70,
        GAUSSIAN_75,
        GAUSSIAN_80,
        GAUSSIAN_85,
        GAUSSIAN_90,
        GAUSSIAN_95,
        GAUSSIAN_99,
        HAMMING,
        HANNING,
        KAISER,
        NUTTALL,
        PARZEN,
        RECTANGULAR,
    };

};

#endif

// :mode=c++: jEdit modeline
