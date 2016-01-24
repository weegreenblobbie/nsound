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
#include <Nsound/biquad/Biquad.hpp>
#include <Nsound/biquad/Design.h>

#include <picojson.h>               // https://github.com/kazuho/picojson


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
    _gain_db_at_fc(0),
    _gain_db_at_band_width(0),
    _gain_db_baseline(0),
    _order(static_cast<uint32>(bk._b.size())),
    _design_mode(CLOSED),
    _render_mode(OFFLINE),
    _kernel(bk),
    _x_buf(_order, 0.0),
    _x_ptr(_x_buf.begin()),
    _x_begin(_x_buf.begin()),
    _x_end(_x_buf.end()),
    _y_buf(_order, 0.0),
    _y_ptr(_y_buf.begin()),
    _y_begin(_y_buf.begin()),
    _y_end(_y_buf.end())
{}


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
    _gain_db_at_fc(gain_db_at_fc),
    _gain_db_at_band_width(gain_db_at_band_width),
    _gain_db_baseline(gain_db_baseline),
    _order(order_),
    _design_mode(OPEN),
    _render_mode(OFFLINE),
    _kernel({std::vector<float64>(_order, 0.0), std::vector<float64>(_order, 0.0)}),
    _x_buf(_order, 0.0),
    _x_ptr(_x_buf.begin()),
    _x_begin(_x_buf.begin()),
    _x_end(_x_buf.end()),
    _y_buf(_order, 0.0),
    _y_ptr(_y_buf.begin()),
    _y_begin(_y_buf.begin()),
    _y_end(_y_buf.end())
{
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
update_design()
{
    M_ASSERT_MSG(
        _design_mode == OPEN,
        "Can not update the design for CLOSED kernels"
    );

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
    _x_ptr = _x_buf.begin();
    _x_end = _x_buf.end();

    _y_buf.clear();
    _y_buf.resize(_kernel._b.size(), 0.0);
    _y_ptr = _y_buf.begin();
    _y_end = _y_buf.end();
}


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

    // write x to x history buffer

    *_x_ptr = x;
    ++_x_ptr;

    if(_x_ptr == _x_end) _x_ptr = _x_begin;

    // sum over: b * x[n]

    float64 y = 0.0;

    auto xptr = _x_ptr;

    for(auto b : _kernel._b)
    {
        if(xptr == _x_begin) xptr = _x_end - 1;
        else                 --xptr;

        y += b * (*xptr);
    }

    // sum over: a * y[n]

    auto yptr = _y_ptr;

    for(auto a : _kernel._a)
    {
        if(yptr == _y_begin) yptr = _y_end - 1;
        else                 --yptr;

        y -= a * (*yptr);
    }

    // write y to y history buffer

    *_y_ptr = y;
    ++_y_ptr;

    if(_y_ptr == _y_end) _y_ptr = _y_begin;

    return y;
}


float64
Biquad::
operator()(float64 in)
{
    return _filter(in, _freq_center, _band_width);
}


float64
Biquad::
operator()(float64 in, float64 fc_)
{
    M_ASSERT_MSG(_design_mode == OPEN, "Can't change freq center with 'CLOSED' design");

    return _filter(in, fc_, _band_width);
}


float64
Biquad::
operator()(float64 in, float64 fc_, float64 bw_)
{
    M_ASSERT_MSG(_design_mode == OPEN, "Can't change freq center or bandwidth with 'CLOSED' design");

    return _filter(in, fc_, bw_);
}


FloatVector
Biquad::
operator()(Callable<float64> & in)
{
    FloatVector out;

    while(!in.finished())
    {
        out.push_back(_filter(in(), _freq_center, _band_width));
    }

    return out;
}


FloatVector
Biquad::
operator()(Callable<float64> & in, Callable<float64> & fc_)
{
    M_ASSERT_MSG(_design_mode == OPEN, "Can't change freq center with 'CLOSED' design");

    FloatVector out;

    while(!in.finished())
    {
        out.push_back(_filter(in(), fc_(), _band_width));
    }

    return out;
}


FloatVector
Biquad::
operator()(Callable<float64> & in, Callable<float64> & fc_, Callable<float64> & bw_)
{
    M_ASSERT_MSG(_design_mode == OPEN, "Can't change freq center or bandwidth with 'CLOSED' design");

    FloatVector out;

    while(!in.finished())
    {
        out.push_back(_filter(in(), fc_(), bw_()));
    }

    return out;
}












} // namespace
} // namespace
