//-----------------------------------------------------------------------------
//
//  $Id: CudaUtils.h 874 2014-09-08 02:21:29Z weegreenblobbie $
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
#ifndef _NSOUND_CUDA_UTILS_H_
#define _NSOUND_CUDA_UTILS_H_

namespace Nsound
{

//-----------------------------------------------------------------------------
// Magic macro to check for cuda errors.
#define checkForCudaError()                     \
{                                               \
    cudaError_t ecode = cudaGetLastError();     \
    if(ecode)                                   \
    {                                           \
        printf("%s:%3d: Error: %s\n",           \
            __FILE__,                           \
            __LINE__,                           \
            cudaGetErrorString(ecode));         \
    }                                           \
}

struct Context
{
    enum State
    {
        BOOTING_    = 1,
        INITALIZED_ = 10,
        UNLOADED_   = 20,
        ERROR_      = 30,
    };

    static State state_;

    //! Initialize the card once.
    void
    init(int flags = 0);
};

} // namespace

#endif
// :mode=c++: jEdit modeline
