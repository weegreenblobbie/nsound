//-----------------------------------------------------------------------------
//
//  $Id: StretcherCuda.h 874 2014-09-08 02:21:29Z weegreenblobbie $
//
//  Nsound is a C++ library and Python module for audio synthesis featuring
//  dynamic digital filters. Nsound lets you easily shape waveforms and write
//  to disk or plot them. Nsound aims to be as powerful as Csound but easy to
//  use.
//
//  Copyright (c) 2008 to Present Nick Hilton
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
#ifndef _NSOUND_STRETCHER_CUDA_H_
#define _NSOUND_STRETCHER_CUDA_H_

namespace Nsound
{

void
searchForBestMatch_f64(
    double *           rss,
    const double *     source,
    const unsigned int source_index,
    const unsigned int search_index,
    const unsigned int window_length,
    const unsigned int max_delta);

void
searchForBestMatch_f32(
    float *           rss,
    const float *     source,
    const unsigned int source_index,
    const unsigned int search_index,
    const unsigned int window_length,
    const unsigned int max_delta);

} // namespace

#endif
// :mode=c++: jEdit modeline
