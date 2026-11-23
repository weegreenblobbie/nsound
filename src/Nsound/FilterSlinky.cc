//-----------------------------------------------------------------------------
//
//  $Id: FilterSlinky.cc 874 2014-09-08 02:21:29Z weegreenblobbie $
//
//  Copyright (c) 2008 to Present Nick Hilton
//
//  weegreenblobbie2_gmail_com (replace '_' with '@' and '.')
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
//-----------------------------------------------------------------------------////

#include <Nsound/AudioStream.h>
#include <Nsound/Buffer.h>
#include <Nsound/FilterBandPassIIR.h>
#include <Nsound/FilterDelay.h>
#include <Nsound/FilterSlinky.h>

using namespace Nsound;

using std::cerr;
using std::cout;
using std::endl;

#define CERR_HEADER __FILE__ << ":" << __LINE__ << ": "

//-----------------------------------------------------------------------------
FilterSlinky::
FilterSlinky(
    const float64 & sample_rate,
    const float64 & delay_octave,
    const float64 & frequency_octave)
    :
    Filter(sample_rate)
{
    // Checked before anything is allocated. A 0 Hz octave would never
    // reach Nyquist below.
    M_ASSERT_VALUE(delay_octave, >, 0.0);
    M_ASSERT_VALUE(frequency_octave, >, 0.0);

    float64 f = 0.0;

    while( f < sample_rate_ / 2.0)
    {
        if(f == 0.0)
        {
            f = frequency_octave;
        }
        else
        {
            f += frequency_octave;
        }
    }

    // Create the highest frequency pass band filter.

    float64 f_high = f;
    f -= frequency_octave;
    float64 f_low  = f;

    filters_.push_back(
        std::make_unique<FilterBandPassIIR>(
            sample_rate_,
            6,
            f_low,
            f_high,
            0.01));

    // Create pass band filters for the remaing frequency octaves.
    float64 dt = delay_octave;
    while(f >= frequency_octave)
    {
        f_high = f;
        f -= frequency_octave;
        f_low = f;

        if(f_high == frequency_octave)
        {
            f_low = 0.0;
        }

        filters_.push_back(
        std::make_unique<FilterBandPassIIR>(
            sample_rate_,
            6,
            f_low,
            f_high,
            0.01));

        delay_lines_.emplace_back(sample_rate_, dt);

        dt += delay_octave;
    }

//~    cout << "n_filters = " << filters_.size() << endl
//~         << "n_delays  = " << delay_lines_.size() << endl;
}

//-----------------------------------------------------------------------------
// Defined here, where FilterBandPassIIR is complete, for the std::unique_ptrs.
FilterSlinky::
~FilterSlinky() = default;


AudioStream
FilterSlinky::
filter(const AudioStream & x)
{
    return Filter::filter(x);
}

AudioStream
FilterSlinky::
filter(const AudioStream & x, const Buffer & /* frequencies */)
{
    return FilterSlinky::filter(x);
}

Buffer
FilterSlinky::
filter(const Buffer & x)
{
    return Filter::filter(x);
}

Buffer
FilterSlinky::
filter(const Buffer & x, const Buffer & /* frequencies */)
{
    return FilterSlinky::filter(x);
}

float64
FilterSlinky::
filter(const float64 & x)
{
    float64 y = 0.0;

    FilterVector::iterator filter = filters_.begin();
    FilterVector::iterator end    = filters_.end();

    DelayVector::iterator delay = delay_lines_.begin();

    // There is always one more filter than delay lines, this is because the
    // highest frequency have zero delay.

    y += (*filter)->filter(x);

    ++filter;

    while(filter != end)
    {
        y += delay->filter((*filter)->filter(x));

        ++filter;
        ++delay;
    }

    return y;
}

float64
FilterSlinky::
filter(const float64 & x, const float64 & /* frequency */)
{
    return FilterSlinky::filter(x);
}

void
FilterSlinky::
reset()
{
    FilterVector::iterator filter = filters_.begin();
    FilterVector::iterator end  = filters_.end();

    DelayVector::iterator delay = delay_lines_.begin();

    (*filter)->reset();
    ++filter;

    while(filter != end)
    {
        (*filter)->reset();
        delay->reset();

        ++filter;
        ++delay;
    }

}

