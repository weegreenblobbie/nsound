//-----------------------------------------------------------------------------
//
//  $Id: Vocoder.cc 878 2014-11-23 04:51:23Z weegreenblobbie $
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

#include <Nsound/AudioStream.h>
#include <Nsound/Buffer.h>
#include <Nsound/FilterBandPassIIR.h>
#include <Nsound/FilterBandPassVocoder.h>
#include <Nsound/FilterMovingAverage.h>
#include <Nsound/Plotter.h>
#include <Nsound/Vocoder.h>

using namespace Nsound;

using std::cerr;
using std::cout;
using std::endl;

#define CERR_HEADER __FILE__ << ":" << __LINE__ << ": "

// Convert Hz to mel scale.
//
// mels = 1127 * ln(1 + hz/700)
//
// http://en.wikipedia.org/wiki/Mel_scale
//

static float64 f_to_mel(float64 f)
{
    M_ASSERT_VALUE(f, >, 0.0);

    return 2595.0 * log10(1.0 + f / 700.0);
}

static float64 mel_to_f(float64 m)
{
    M_ASSERT_VALUE(m, >, 0.0);
    m = pow(10.0, m / 2595.0);
    return (m - 1.0) * 700.0;
}


//-----------------------------------------------------------------------------
Vocoder::
Vocoder(
    const float64 & sample_rate,
    const float64 & window_length,
    const uint32  & n_bands,
    const float64 & freq_max,
    const bool use_mel_scale,
    const bool plot_filter_bank)
    :
    sample_rate_(sample_rate),
    window_size_(static_cast<uint32>(window_length * sample_rate_)),
    n_bands_(n_bands),
    freq_max_(freq_max),
    filters_input_(),
    filters_output_(),
    moving_average_()
{
    M_ASSERT_VALUE(sample_rate_, >, 0.0);

    uint32 min_window_size = static_cast<uint32>(sample_rate_ * 0.005);

    M_ASSERT_VALUE(window_size_, >=, min_window_size);

    uint32 min_n_bands = 4;

    M_ASSERT_VALUE(n_bands_, >=, min_n_bands);

    float64 min_freq_max = 0.05 * sample_rate_;

    M_ASSERT_VALUE(freq_max_, >=, min_freq_max);

    float64 hi = freq_max_;

    // convert to mel scale

    if(use_mel_scale)
    {
        hi = f_to_mel(hi);
    }

    // linear space from lo to hi.

    float64 band_width = hi / static_cast<float64>(n_bands_);

    Buffer freq_centers(n_bands_);

    for(uint32 i = 0; i <= n_bands_; ++i)
    {
        freq_centers << (i + 1) * band_width;
    }

    // Shift to the center of each window.
    freq_centers -= band_width / 2.0;

    // convert back to Hz scale.
    if(use_mel_scale)
    {
        for(uint32 i = 0; i < freq_centers.getLength(); ++i)
        {
            freq_centers[i] = mel_to_f(freq_centers[i]);
        }
    }

    uint32 start = 0;
    uint32 order = 6;
    float64 ripple = 0.01;

    for(uint32 i = start; i < n_bands_; ++i)
    {
        float64 freq = freq_centers[i];

        // Cut offs are halfway between the filters.

        float64 f_lo = 0.0;

        if(i > 0)
        {
            f_lo = (freq + freq_centers[i - 1]) / 2.0;
        }

        float64 f_hi = freq_max_;

        if(i + 1 < n_bands_)
        {
            f_hi = (freq + freq_centers[i + 1]) / 2.0;
        }

        FilterBandPassIIR * bp = new FilterBandPassIIR(
            sample_rate_,
            order,
            f_lo,
            f_hi,
            ripple);

        M_CHECK_PTR(bp);

        filters_input_.push_back(bp);

        bp = new FilterBandPassIIR(
            sample_rate_,
            order,
            f_lo,
            f_hi,
            ripple);

        M_CHECK_PTR(bp);

        filters_output_.push_back(bp);

        FilterMovingAverage * ma = new FilterMovingAverage(window_size_);

        M_CHECK_PTR(ma);

        moving_average_.push_back(ma);
    }

    if(plot_filter_bank)
    {
        Plotter pylab;

        pylab.figure();
        pylab.hold(true);

        for(size_t i = 0; i < filters_input_.size(); ++i)
        {
            Buffer x = filters_input_[i]->getFrequencyAxis();
            Buffer y = filters_input_[i]->getFrequencyResponse().getdB();

            pylab.plot(x, y);
        }

        pylab.xlim(0.0, freq_max_);
        pylab.ylim(-40.0, 10.0);
        pylab.xlabel("Frequency (Hz)");
        pylab.ylabel("Filter Frequency Response (dB)");
        pylab.grid(true);

//~        pylab.show();
    }

}

//-----------------------------------------------------------------------------
Vocoder::
~Vocoder()
{
    FilterVector::iterator fi = filters_input_.begin();
    FilterVector::iterator fo = filters_output_.begin();
    FilterVector::iterator ma = moving_average_.begin();

    FilterVector::iterator end = filters_input_.end();

    while(fi != end)
    {
        delete *fi;
        delete *fo;
        delete *ma;

        ++fi;
        ++fo;
        ++ma;
    }
}

Buffer
Vocoder::
filter(const Buffer & voice, const Buffer & x)
{
    Buffer y;

    Buffer::const_circular_iterator v = voice.cbegin();

    uint32 n_samples = x.getLength();

    for(uint32 n = 0; n < n_samples; ++n, ++v)
    {
        y << Vocoder::filter(*v, x[n]);
    }

    return y;
}

float64
Vocoder::
filter(const float64 & voice, const float64 & x)
{
    FilterVector::iterator fi = filters_input_.begin();
    FilterVector::iterator fo = filters_output_.begin();
    FilterVector::iterator ma = moving_average_.begin();

    FilterVector::iterator end = filters_input_.end();

    float64 y = 0.0;

    while(fi != end)
    {
        // Filter the voice to use as an envelope..
        float64 envelope = (*fi)->filter(voice);

        // Use the abs of the envelope.
        envelope = std::fabs(envelope);

        // Smooth the filtered envelope.
        envelope = (*ma)->filter(envelope);

        // Shape the filtered input signal.
        y += envelope * (*fo)->filter(x);

        ++fi;
        ++fo;
        ++ma;
    }

    return y;
}

void
Vocoder::
reset()
{
    FilterVector::iterator fi = filters_input_.begin();
    FilterVector::iterator fo = filters_output_.begin();
    FilterVector::iterator ma = moving_average_.begin();

    FilterVector::iterator end = filters_input_.end();

    while(fi != end)
    {
        (*fi)->reset();
        (*fo)->reset();
        (*ma)->reset();

        ++fi;
        ++fo;
        ++ma;
    }
}

