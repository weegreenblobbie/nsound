//-----------------------------------------------------------------------------
//
//  $Id: Test.h 874 2014-09-08 02:21:29Z weegreenblobbie $
//
//  Nsound is a C++ library and Python module for audio synthesis featuring
//  dynamic digital filters. Nsound lets you easily shape waveforms and write
//  to disk or plot them. Nsound aims to be as powerful as Csound but easy to
//  use.
//
//  Copyright (c) 2006 Nick Hilton
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

#ifndef _NSOUND_TEST_H_
#define _NSOUND_TEST_H_

#define TEST_HEADER2(line) \
    THIS_FILE << ":"; cout.width(4); cout << line \
    << ": "; cout.width(0); cout

#define TEST_ERROR_HEADER2(line) \
    endl << THIS_FILE << ":"; cerr.width(4); cerr << line \
    << ": ***Error!  "; cerr.width(0); cout

#define TEST_HEADER \
    THIS_FILE << ":"; cout.width(4); cout << __LINE__ \
    << ": "; cout.width(0); cout

#define TEST_ERROR_HEADER \
    endl << THIS_FILE << ":"; cerr.width(4); cerr << __LINE__ \
    << ": ***Error!  "; cerr.width(0); cout


#endif
