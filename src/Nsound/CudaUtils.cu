///////////////////////////////////////////////////////////////////////////////
//
//  $Id: CudaUtils.cu 699 2011-10-03 20:56:04Z weegreenblobbie $
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
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
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
///////////////////////////////////////////////////////////////////////////////

#include <cuda.h>
#include <Nsound/CudaUtils.h>
#include <string.h>
#include <stdio.h>

// The nvcc compiler doesn't handle namespaces the same way GCC does, as of
// version: Cuda compilation tools, release 3.2, V0.2.1221

Nsound::Context::State
Nsound::Context::state_ = Nsound::Context::BOOTING_;

///////////////////////////////////////////////////////////////////////////////
void
Nsound::
Context::
init(int flags)
{
    if(state_ == BOOTING_)
    {
        if(CUDA_SUCCESS == cuInit(flags))
        {
            state_ = INITALIZED_;
        }
        else
        {
            checkForCudaError();
            state_ = ERROR_;
        }
    }
}

// :mode=c++: jEdit modeline
