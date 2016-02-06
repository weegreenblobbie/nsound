//-----------------------------------------------------------------------------
//
//  $Id$
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

#include <Nsound/Nsound.h>

#include <Nsound/Buffer.h>
#include <Nsound/FFTransform.h>
#include <Nsound/Plotter.h>

#include <Nsound/biquad/Biquad.hpp>
#include <Nsound/biquad/FilterBank.hpp>

#define PICOJSON_USE_INT64

#include <picojson.h>               // https://github.com/kazuho/picojson


namespace Nsound
{
namespace biquad
{


FilterBank::
FilterBank(float64 sample_rate)
    :
    _sample_rate(sample_rate),
    _render_mode(OFFLINE),
    _filters()
{
    M_ASSERT_VALUE(sample_rate, >, 0);
}


FilterBank::
FilterBank(const FilterBank & copy)
    :
    _sample_rate(copy._sample_rate),
    _render_mode(copy._render_mode),
    _filters()
{
    for(auto ptr : copy._filters)
    {
        if(ptr)
        {
            _filters.push_back(std::make_shared<Biquad>(*ptr));
        }
        else
        {
            _filters.push_back(nullptr);
        }
    }
}


FilterId
FilterBank::
add(const Biquad & bq)
{
    // try to enforce filters to be designed at same samplerate.

    if(_sample_rate > 0)
    {
        float64 sr = -1;

        try
        {
            sr = bq.sr();
        }
        catch(...)
        {
            // pass
        }

        if(sr > 0 && std::abs(sr - _sample_rate) > 1.0)
        {
            M_THROW("Sample rates must agree (" << _sample_rate << " != " << sr << ")");
        }
    }

    BiquadPtr bqp = std::make_shared<Biquad>(Biquad(bq));

    bqp->render_mode(_render_mode);

    FilterId fid = _filters.size();

    _filters.push_back(bqp);

    return fid;
}


void
FilterBank::
remove(FilterId id)
{
    M_ASSERT_MSG(
        id < _filters.size(),
        "FilterId out of bounds (" << id << " >= " << _filters.size() << ")"
    );

    _filters[id].reset();
}


Biquad &
FilterBank::
get(FilterId id)
{
    M_ASSERT_MSG(
        id < _filters.size(),
        "FilterId out of bounds (" << id << " >= " << _filters.size() << ")"
    );

    M_ASSERT_MSG(
        nullptr != _filters[id],
        "FilterId " << id << " has been removed"
    );

    return *_filters[id];
}


BiquadKernel
FilterBank::
kernel() const
{
    BiquadKernel bk({{1.0},{1.0}});

    for(auto ptr : _filters)
    {
        if(!ptr) continue;

        bk = cas2dir(bk, ptr->kernel());
    }

    return bk;
}


void
FilterBank::
render_mode(RenderMode rm)
{
    for(auto ptr : _filters)
    {
        if(!ptr) continue;

        ptr->render_mode(rm);
    }
}


float64
FilterBank::
operator()(float64 x)
{
    auto y = x;

    for(auto ptr : _filters)
    {
        if(!ptr) continue;

        y = (*ptr)(y);
    }

    return y;
}


Buffer
FilterBank::
operator()(const Iterate<float64> & in)
{
    Buffer out;
    out << in;

    for(auto ptr : _filters)
    {
        if(!ptr) continue;

        out = (*ptr)(Iterate<float64>(out.data()));
    }

    return out;
}


void
FilterBank::
plot(boolean show_phase) const
{
    float64 window_size = 0.080;

    FilterBank fb(*this);

    Buffer f_axis = fb._get_freq_axis(_sample_rate, window_size);
    Buffer resp   = fb._get_freq_response(_sample_rate, window_size);

    resp.dB();

    Plotter pylab;

    pylab.figure();

    uint32 n_rows = 1;
    uint32 n_cols = 1;

    if(show_phase)
    {
        n_rows = 2;
    }

    pylab.subplot(n_rows, n_cols, 1);

    // Frequency response
    pylab.plot(f_axis, resp, "blue");
    pylab.grid(true);

    pylab.xlabel("Frequency (Hz)");
    pylab.ylabel("Frequency Response (dB)");

    // plot design parameters

    std::vector<std::string> formats = {"ro", "co", "mo", "go"};

    auto fmt = formats.begin();

    auto ymax = 0.0;

    for(auto ptr : _filters)
    {
        if(!ptr) continue;
        try
        {
            ymax = std::max(ymax, std::abs(ptr->g0()));

            pylab.plot(ptr->lo(), ptr->g1(), *fmt, "mec='none',ms=5");
            pylab.plot(ptr->hi(), ptr->g1(), *fmt, "mec='none',ms=5");
            pylab.plot(ptr->fc(), ptr->g0(), *fmt, "mec='none',ms=5");

            ++fmt;

            if(fmt == formats.end()) fmt = formats.begin();
        }
        catch(...)
        {
        }
    }

    auto yrange = 2 * ymax;
    auto margin = 0.05 * yrange;

    auto y0 = -ymax - margin;
    auto y1 = ymax + margin;

    pylab.ylim(y0, y1);

    // Phase response
    if(show_phase)
    {
        pylab.subplot(n_rows, n_cols, 2);

        Buffer phase = fb._get_phase_response(_sample_rate, window_size).getdB();

        pylab.plot(f_axis, phase);

        pylab.xlabel("Frequency (Hz)");
        pylab.ylabel("Phase Response (dB)");
    }

    pylab.xlim(f_axis[0], f_axis[f_axis.getLength()-1]);

    pylab.title("Nsound::biquad::FilterBank");
}


uint32
FilterBank::
_get_nfft(float64 sample_rate, float64 size_sec) const
{
    M_ASSERT_VALUE(sample_rate, >, 0);
    M_ASSERT_VALUE(size_sec, >, 0);

    uint32 n_fft = FFTransform::roundUp2(
        static_cast<uint32>(std::round(size_sec * sample_rate)));

    M_ASSERT_VALUE(n_fft, >, 0);

    return n_fft;
}


Buffer
FilterBank::
_get_freq_axis(float64 sample_rate, float64 size_sec) const
{
    uint32 n_fft = _get_nfft(sample_rate, size_sec);

    uint32 n_samples = n_fft / 2 + 1;

    float64 f_step =
        (1.0 / (static_cast<float64>(n_fft) / 2.0)) * (sample_rate / 2.0);

    Buffer f_axis;

    float64 f = 0.0;

    for(uint32 i = 0; i < n_samples; ++i)
    {
        f_axis << f;
        f += f_step;
    }

    return f_axis;
}


Buffer
FilterBank::
_get_freq_response(float64 sample_rate, float64 size_sec) const
{
    uint32 n_fft = _get_nfft(sample_rate, size_sec);

    FFTransform fft(sample_rate);

    FFTChunkVector vec = fft.fft(
        _get_impulse_response(sample_rate, size_sec),
        n_fft);

    return vec[0].getMagnitude();
}


Buffer
FilterBank::
_get_impulse_response(float64 sample_rate, float64 size_sec) const
{
    FilterBank fb(*this);

    uint32 n_fft = _get_nfft(sample_rate, size_sec);

    Buffer resp(n_fft);

    resp << fb(1.0);

    for(uint32 i = 1; i < n_fft; ++i)
    {
        resp << fb(0.0);
    }

    return resp;
}


Buffer
FilterBank::
_get_phase_response(float64 sample_rate, float64 size_sec) const
{
    uint32 n_fft = _get_nfft(sample_rate, size_sec);

    FFTransform fft(n_fft);

    FFTChunkVector vec = fft.fft(
        _get_impulse_response(sample_rate, size_sec),
        n_fft);

    return vec[0].getPhase();
}


std::string
FilterBank::
to_json() const
{
    picojson::value val;

    to_json(val);

    return val.serialize(true);
}


void
FilterBank::
to_json(picojson::value & val) const
{
    using Object = picojson::object;
    using Array = picojson::array;
    using Value = picojson::value;

    Object obj;

    obj["samplerate"] = Value(_sample_rate);

    Array array;

    for(auto ptr : _filters)
    {
        if(!ptr) continue;

        picojson::value v;

        ptr->to_json(v);

        array.push_back(v);
    }

    obj["filters"] = Value(array);

    val = Value(obj);
}


FilterBank
FilterBank::
from_json(const std::string & in)
{
    using Value = picojson::value;

    Value v;

    std::string err;

    picojson::parse(v, in.begin(), in.end(), &err);

    if(!err.empty())
    {
        M_THROW("JSON parser error: " << err);
    }

    return from_json(v);
}


std::string
to_type_string(const picojson::value & v)
{
    if(v.is<bool>()) return "bool";
    else
    if(v.is<picojson::null>()) return "null";
    else
    if(v.is<int64_t>()) return "int";
    else
    if(v.is<float64>()) return "float";
    else
    if(v.is<std::string>()) return "string";
    else
    if(v.is<picojson::array>()) return "array";
    else
    if(v.is<picojson::object>()) return "object";
    else
    if(v.is<picojson::object>()) return "object";

    return "unknown";
}


FilterBank
FilterBank::
from_json(const picojson::value & in)
{
    if (! in.is<picojson::object>())
    {
        M_THROW("Expecting a JSON object, got " << to_type_string(in));
    }

    M_ASSERT_MSG(in.contains("samplerate"), "Expecting key 'samplerate'");

    float64 sr = in.get("samplerate").get<float64>();

    FilterBank fb(sr);

    M_ASSERT_MSG(in.contains("filters"), "Expecting key 'filters'");

    const auto & array = in.get("filters").get<picojson::array>();

    for(auto & obj : array)
    {
        fb.add(Biquad::from_json(obj));
    }

    return fb;
}


} // namespace
} // namespace
