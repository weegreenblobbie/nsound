//-----------------------------------------------------------------------------
//
//  $Id$
//
//  Nsound is a C++ library and Python module for audio synthesis featuring
//  dynamic digital filters. Nsound lets you easily shape waveforms and write
//  to disk or plot them. Nsound aims to be as powerful as Csound but easy to
//  use.
//
//  Copyright (c) 2015 to Present Nick Hilton
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
#ifndef _NSOUND_BIQUAD_FILTER_BANK_HPP_
#define _NSOUND_BIQUAD_FILTER_BANK_HPP_

#include <Nsound/Nsound.h>

#include <Nsound/Interfaces.hpp>
#include <Nsound/biquad/Biquad.hpp>

#include <memory>


// https://github.com/kazuho/picojson
#define PICOJSON_USE_INT64
#include <picojson.h>



namespace Nsound
{
namespace biquad
{


class Biquad;

typedef uint32 FilterId;


class FilterBank : public RenderModal
{

public:

    static FilterBank from_json(const std::string & jstr);
    static FilterBank from_json(const picojson::value & jval);

    FilterBank(float64 sample_rate = -1);
    FilterBank(const FilterBank & copy);  // pointer require I do this right.

    std::string to_json() const;
    void to_json(picojson::value & obj) const;

    FilterId add(const Biquad & bq);  // copies input filter
    void     remove(FilterId id);
    Biquad & get(FilterId id);

    BiquadKernel kernel() const; // combines all filters into one kernel.

    // interface methods

    RenderMode render_mode()        { return _render_mode; }
    void render_mode(RenderMode rm);

    // filter methods

    float64 operator()(float64 x);
    Buffer operator()(const Buffer & x);
    AudioStream operator()(const AudioStream & x);

    void plot(boolean show_phase = false) const;

    Buffer get_freq_axis(float64 window_size = 0.080) const;
    Buffer get_freq_response(float64 window_size = 0.80) const;

private:

    // plot support functions

    Buffer _get_freq_axis(float64 sample_rate, float64 size_sec) const;
    Buffer _get_freq_response(float64 sample_rate, float64 size_sec) const;
    Buffer _get_impulse_response(float64 sample_rate, float64 size_sec) const;
    Buffer _get_phase_response(float64 sample_rate, float64 size_sec) const;
    uint32 _get_nfft(float64 sample_rate, float64 size_sec) const;

    // all filter calls eventually call this one:

    float64 _filter(float64 x);

    float64    _sample_rate;
    RenderMode _render_mode;

    typedef std::shared_ptr<Biquad> BiquadPtr;

    std::vector<BiquadPtr> _filters;

};


} // namespace
} // namespace


#endif