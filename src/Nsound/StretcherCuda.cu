///////////////////////////////////////////////////////////////////////////////
//
//  $Id$
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
#include <Nsound/StretcherCuda.h>

#include <stdio.h>
#include <string.h>

using namespace Nsound;

template<typename T>
__global__
void
calculateRSS(
    T *                rss,
    const T *          A,
    const T *          B,
    const unsigned int window_length,
    const unsigned int max_delta,
    const unsigned int thread_offset)
{
    const unsigned int offset = blockIdx.x + thread_offset;

    // Is this a valid thread?
    if(offset >= max_delta) return;

    T sum = 0.0;

    for(unsigned int i = 0; i < window_length; ++i)
    {
        T diff = A[i] - B[i+offset];

        sum += diff * diff;
    }

    rss[offset] = sum;
}


///////////////////////////////////////////////////////////////////////////////
//
// Calculate the residual sum square for all max_delta test cases in parallel
// on the cuda card.
void
Nsound::
searchForBestMatch_f64(
    double *           rss,
    const double *     source,
    const unsigned int source_index,
    const unsigned int search_index,
    const unsigned int window_length,
    const unsigned int max_delta)
{
    Nsound::Context context;
    context.init(0);

    if(context.state_ != Nsound::Context::INITALIZED_)
    {
        rss[0] = -1*context.state_;
        rss[1] = -1*context.state_;
        rss[2] = -1*context.state_;
        return;
    }

    // Copy vectors to device
    // Copy source & rss_array to device
    double * gpuA   = NULL;
    double * gpuB   = NULL;
    double * gpuRSS = NULL;

    const unsigned int ABSIZE = sizeof(double) * (max_delta + window_length);
    const unsigned int RSSIZE = sizeof(double) * (max_delta);

    // Allocate GPU memory
    cudaMalloc((void**)&gpuA,   ABSIZE); checkForCudaError();
    cudaMalloc((void**)&gpuB,   ABSIZE); checkForCudaError();
    cudaMalloc((void**)&gpuRSS, RSSIZE); checkForCudaError();

    // Copy the data to the device
    cudaMemcpy(
        gpuA,
        source + source_index,
        ABSIZE,
        cudaMemcpyHostToDevice); checkForCudaError();

    cudaMemcpy(
        gpuB,
        source + search_index,
        ABSIZE,
        cudaMemcpyHostToDevice); checkForCudaError();

    unsigned int threads_per_block = 65536 / max_delta;

//~    for(
//~        unsigned int thread_offset = 0;
//~        thread_offset < max_delta;
//~        thread_offset += threads_per_block)
//~    {

        calculateRSS<double> <<<max_delta, 1>>>(
            gpuRSS,
            gpuA,
            gpuB,
            window_length,
            max_delta,
            0); checkForCudaError();

        // Copy the data back to the host
        cudaMemcpy(
            rss,
            gpuRSS,
            RSSIZE,
            cudaMemcpyDeviceToHost); checkForCudaError();
//~    }
}

void
Nsound::
searchForBestMatch_f32(
    float *           rss,
    const float *     source,
    const unsigned int source_index,
    const unsigned int search_index,
    const unsigned int window_length,
    const unsigned int max_delta)
{
    Nsound::Context context;
    context.init(0);

    if(context.state_ != Nsound::Context::INITALIZED_)
    {
        rss[0] = -1*context.state_;
        rss[1] = -1*context.state_;
        rss[2] = -1*context.state_;
        return;
    }

    // Copy vectors to device
    // Copy source & rss_array to device
    float * gpuA   = NULL;
    float * gpuB   = NULL;
    float * gpuRSS = NULL;

    const unsigned int ABSIZE = sizeof(float) * (max_delta + window_length);
    const unsigned int RSSIZE = sizeof(float) * (max_delta);

    // Allocate GPU memory
    cudaMalloc((void**)&gpuA,   ABSIZE); checkForCudaError();
    cudaMalloc((void**)&gpuB,   ABSIZE); checkForCudaError();
    cudaMalloc((void**)&gpuRSS, RSSIZE); checkForCudaError();

    // Copy the data to the device
    cudaMemcpy(
        gpuA,
        source + source_index,
        ABSIZE,
        cudaMemcpyHostToDevice); checkForCudaError();

    cudaMemcpy(
        gpuB,
        source + search_index,
        ABSIZE,
        cudaMemcpyHostToDevice); checkForCudaError();

    // Magic Number, applies to nick's cuda card
    unsigned int threads_per_block = 512;

    for(
        unsigned int thread_offset = 0;
        thread_offset < max_delta;
        thread_offset += threads_per_block)
    {

        calculateRSS<float> <<<threads_per_block, 1>>>(
            gpuRSS,
            gpuA,
            gpuB,
            window_length,
            max_delta,
            thread_offset); checkForCudaError();
    }

    // Copy the data back to the host
    cudaMemcpy(
        rss,
        gpuRSS,
        RSSIZE,
        cudaMemcpyDeviceToHost); checkForCudaError();
}

// :mode=c++: jEdit modeline
