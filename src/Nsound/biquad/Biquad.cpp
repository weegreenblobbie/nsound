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
#include <Nsound/biquad/Design.hpp>

#define PICOJSON_USE_INT64

#include <picojson.h>               // https://github.com/kazuho/picojson

#include <cmath>



namespace Nsound
{
namespace biquad
{


Biquad::
Biquad(const BiquadKernel & bk)
    :
    _sample_rate(0),
    _freq_center(0),
    _band_width(0),
    _band_edge(),
    _gain_db_at_fc(0),
    _gain_db_at_band_width(0),
    _gain_db_baseline(0),
    _order(static_cast<uint32>(bk._b.size())),
    _design_mode(CLOSED),
    _render_mode(OFFLINE),
    _kernel(bk),
    _x_buf(_order, 0.0),
    _x_ptr(0),
    _y_buf(_order, 0.0),
    _y_ptr(0)
{
}


Biquad::
Biquad(
    float64 sample_rate,
    float64 freq_center,
    float64 bandwidth,
    float64 gain_db_at_fc,         // g0
    float64 gain_db_at_band_width, // g1
    float64 gain_db_baseline,      // g2
    uint32  order_)
    :
    _sample_rate(sample_rate),
    _freq_center(freq_center),
    _band_width(bandwidth),
    _band_edge(),
    _gain_db_at_fc(gain_db_at_fc),
    _gain_db_at_band_width(gain_db_at_band_width),
    _gain_db_baseline(gain_db_baseline),
    _order(order_),
    _design_mode(OPEN),
    _render_mode(OFFLINE),
    _kernel({std::vector<float64>(_order, 0.0), std::vector<float64>(_order, 0.0)}),
    _x_buf(_order, 0.0),
    _x_ptr(0),
    _y_buf(_order, 0.0),
    _y_ptr(0)
{
    M_ASSERT_VALUE(sample_rate, >, 0);

    // range check some of these
    bw(_band_width);
    g0(_gain_db_at_fc);
    g1(_gain_db_at_band_width);
    g2(_gain_db_baseline);
    order(_order);
    update_design();
}


void
Biquad::
_reset()
{
    std::fill(_x_buf.begin(), _x_buf.end(), 0.0);
    std::fill(_y_buf.begin(), _y_buf.end(), 0.0);

    _x_ptr = 0;
    _y_ptr = 0;
}


void
Biquad::
update_design()
{
    M_ASSERT_MSG(
        _design_mode == OPEN,
        "Can not update the design for CLOSED kernels"
    );

    _band_edge = BandEdge(_sample_rate, _freq_center, _band_width);

    _kernel = hpeq_design(
        _sample_rate,
        _order,
        _freq_center,
        _band_width,
        _gain_db_baseline,
        _gain_db_at_fc,
        _gain_db_at_band_width,
        BUTTERWORTH
    );

    // Reallocate buffers if necessary

    if(_x_buf.size() == _kernel._b.size()) return;

    _x_buf.clear();
    _x_buf.resize(_kernel._b.size(), 0.0);

    _y_buf.clear();
    _y_buf.resize(_kernel._a.size(), 0.0);
}


// helper object to iterate over a vector, skipping the first element

template<int32 offset, typename T>
class SkipN
{

private:

    T & _vector;

public:

    SkipN(T & vec) : _vector( vec ) {}

    auto begin() const -> decltype( _vector.begin() )
    { return _vector.begin() + offset; }

    auto end() const -> decltype( _vector.end() )
    { return _vector.end(); }
};


float64
Biquad::
_filter(float64 x, float64 fc_, float64 bw_)
{
    // update the kernel?

    if(
        std::abs(_freq_center - fc_) >= 1.0 ||
        std::abs(_band_width - bw_) >= 2.0)
    {
        fc(fc_);
        bw(bw_);
        update_design();
    }

    // IIR filter, difference equation:
    //
    //     https://en.wikipedia.org/wiki/Infinite_impulse_response

    const int32 size_b = static_cast<int32>(_kernel._b.size());
    const int32 size_a = static_cast<int32>(_kernel._a.size());

    // write x to x history buffer

    _x_buf[_x_ptr] = x;
    ++_x_ptr;

    if(_x_ptr >= size_b) _x_ptr = 0;

    // sum over: b * x[n]

    float64 y = 0.0;

    auto xhist = _x_ptr;

    for(auto b : _kernel._b)
    {
        --xhist;

        if(xhist < 0) xhist = size_b - 1;

        y += b * _x_buf[xhist];
    }

    // sum over: a * y[n]

    auto yhist = _y_ptr;

    for(auto a : SkipN<1, std::vector<float64>>(_kernel._a))
    {
        --yhist;

        if(yhist < 0) yhist = size_a - 1;

        y -= a * _y_buf[yhist];
    }

    // write y to y history buffer

    _y_buf[_y_ptr] = y;

    ++_y_ptr;

    if(_y_ptr >= size_a) _y_ptr = 0;

    return y;
}


float64
Biquad::
operator()(float64 in, float64 fc_, float64 bw_)
{
    return _filter(in, fc_, bw_);
}


Buffer
Biquad::
operator()(const Iterate<float64> & in, const Callable<float64> & fc_, const Callable<float64> & bw_)
{
    M_ASSERT_MSG(_design_mode == OPEN, "Can't change freq center or bandwidth with 'CLOSED' design");

    if(_render_mode == OFFLINE) _reset();

    Buffer out;

    for(auto x : in)
    {
        out << _filter(x, fc_(), bw_());
    }

    return out;
}


Biquad
Biquad::
from_json(const std::string & in)
{
    typedef picojson::value::array Array;
//~    typedef picojson::value::object Object;
    typedef picojson::value Value;

    Value v;

    std::string err;

    picojson::parse(v, in.begin(), in.end(), &err);

    if (! err.empty())
    {
        M_THROW("JSON parser error: " << err);
    }

    // open design

    if(v.contains("samplerate"))
    {
        auto samplerate = v.get("samplerate").get<float64>();
        auto freq_center = v.get("freq_center_hz").get<float64>();
        auto band_width = v.get("band_width_hz").get<float64>();
        auto gain_db_at_fc = v.get("gain_db_at_fc").get<float64>();
        auto gain_db_at_band_width = v.get("gain_db_at_band_width").get<float64>();
        auto gain_db_baseline = v.get("gain_db_baseline").get<float64>();
        auto o = v.get("order").get<int64_t>();

        M_ASSERT_MSG(o > 0, "order must be > 0 (" << o << " <= 0)");

        uint32 order = static_cast<uint32>(o);

        return Biquad(
            samplerate,
            freq_center,
            band_width,
            gain_db_at_fc,
            gain_db_at_band_width,
            gain_db_baseline,
            order);
    }

    M_ASSERT_MSG(v.contains("b"), "Error, expecting JSON to contian key 'b'");
    M_ASSERT_MSG(v.contains("a"), "Error, expecting JSON to contian key 'a'");

    Array b = v.get("b").get<Array>();
    Array a = v.get("a").get<Array>();

    BiquadKernel bk;

    for(auto & bb : b)
    {
        bk._b.push_back(bb.get<float64>());
    }

    for(auto & aa : a)
    {
        bk._a.push_back(aa.get<float64>());
    }

    return BiquadKernel(bk);
}


std::string
Biquad::
to_json() const
{
    typedef picojson::value::array Array;
    typedef picojson::value::object Object;
    typedef picojson::value Value;

    Object obj;

    switch(_design_mode)
    {
        case OPEN:
        {
            obj["samplerate"] = Value(_sample_rate);
            obj["freq_center_hz"] = Value(_freq_center);
            obj["band_width_hz"] = Value(_band_width);
            obj["gain_db_at_fc"] = Value(_gain_db_at_fc);
            obj["gain_db_at_band_width"] = Value(_gain_db_at_band_width);
            obj["gain_db_baseline"] = Value(_gain_db_baseline);
            obj["order"] = Value(static_cast<int64_t>(_order));
            break;
        }

        case CLOSED:
        {
            Array b;
            Array a;

            for(const auto bb : _kernel._b)
            {
                b.push_back(Value(bb));
            }

            for(const auto aa : _kernel._a)
            {
                a.push_back(Value(aa));
            }

            obj["b"] = Value(b);
            obj["a"] = Value(a);

            break;
        }
    }

    return Value(obj).serialize(true);
}


void
Biquad::
plot(boolean show_phase) const
{
    M_ASSERT_VALUE(_design_mode, ==, OPEN);
    plot(sr(), show_phase);
}


void
Biquad::
plot(float64 sample_rate, boolean show_phase) const
{
    float64 window_size = 0.080;

    Biquad bq(kernel());

    Buffer f_axis = bq._get_freq_axis(sample_rate, window_size);
    Buffer resp   = bq._get_freq_response(sample_rate, window_size);

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

    float64 y0 = -60.0;
    float64 y1 = resp.getMax();
    float64 yrange = y1 - y0;

    y1 += 0.05 * yrange;

//~    pylab.ylim(y0, y1);

    pylab.xlabel("Frequency (Hz)");
    pylab.ylabel("Frequency Response (dB)");

    // plot design parameters

    if(_design_mode == OPEN)
    {
        pylab.plot(lo(), g1(), "r+", "ms=10,mew=2");
        pylab.plot(hi(), g1(), "r+", "ms=10,mew=2");
        pylab.plot(fc(), g0(), "r+", "ms=10,mew=2");
    }

    // Phase response
    if(show_phase)
    {
        pylab.subplot(n_rows, n_cols, 2);

        Buffer phase = bq._get_phase_response(sample_rate, window_size).getdB();

        pylab.plot(f_axis, phase);

        pylab.xlabel("Frequency (Hz)");
        pylab.ylabel("Phase Response (dB)");
    }

    pylab.xlim(f_axis[0], f_axis[f_axis.getLength()-1]);
}


uint32
Biquad::
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
Biquad::
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
Biquad::
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
Biquad::
_get_impulse_response(float64 sample_rate, float64 size_sec) const
{
    Biquad bq(kernel());

    uint32 n_fft = _get_nfft(sample_rate, size_sec);

    Buffer resp(n_fft);

    resp << bq(1.0);

    for(uint32 i = 1; i < n_fft; ++i)
    {
        resp << bq(0.0);
    }

    return resp;
}


Buffer
Biquad::
_get_phase_response(float64 sample_rate, float64 size_sec) const
{
    uint32 n_fft = _get_nfft(sample_rate, size_sec);

    FFTransform fft(n_fft);

    FFTChunkVector vec = fft.fft(
        _get_impulse_response(sample_rate, size_sec),
        n_fft);

    return vec[0].getPhase();
}



} // namespace
} // namespace
