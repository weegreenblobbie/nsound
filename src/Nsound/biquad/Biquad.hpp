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
#ifndef _NSOUND_BIQUAD_BIQUAD_HPP_
#define _NSOUND_BIQUAD_BIQUAD_HPP_

#include <Nsound/Nsound.h>

#include <Nsound/Callable.hpp>
#include <Nsound/Interfaces.hpp>

#include <Nsound/biquad/Design.hpp>
#include <Nsound/biquad/Kernel.hpp>


// https://github.com/kazuho/picojson
#define PICOJSON_USE_INT64
#include <picojson.h>



namespace Nsound
{
namespace biquad
{

class Biquad : public RenderModal
{

public:

    static Biquad from_json(const std::string & in);
    static Biquad from_json(const picojson::value & in);

//~    Biquad(const Biquad &) = default;  // is this needed?

    Biquad(const BiquadKernel & bk);

    Biquad(
        float64 sample_rate,
        float64 freq_center,
        float64 bandwidth,
        float64 gain_db_at_fc,         // g0
        float64 gain_db_at_band_width, // g1
        float64 gain_db_baseline,      // g2
        uint32  order);

    std::string to_json() const;

    void to_json(picojson::value &) const; // writes to object

    void update_design();

    // QT style getters

    BiquadKernel kernel() const { return _kernel; }

    float64 sr() const    { M_ASSERT_VALUE(_design_mode, ==, OPEN); return _sample_rate; }
    float64 bw() const    { M_ASSERT_VALUE(_design_mode, ==, OPEN); return _band_width; }
    float64 fc() const    { M_ASSERT_VALUE(_design_mode, ==, OPEN); return _freq_center; }
    float64 lo() const    { M_ASSERT_VALUE(_design_mode, ==, OPEN); return _band_edge._lo_hz; }
    float64 hi() const    { M_ASSERT_VALUE(_design_mode, ==, OPEN); return _band_edge._hi_hz; }
    float64 g0() const    { M_ASSERT_VALUE(_design_mode, ==, OPEN); return _gain_db_at_fc; }
    float64 g1() const    { M_ASSERT_VALUE(_design_mode, ==, OPEN); return _gain_db_at_band_width; }
    float64 g2() const    { M_ASSERT_VALUE(_design_mode, ==, OPEN); return _gain_db_baseline; }
    uint32  order() const { M_ASSERT_VALUE(_design_mode, ==, OPEN); return _order; }

    // QT style setters

    void bw(float64 v);
    void fc(float64 v);
    void g0(float64 v);
    void g1(float64 v);
    void g2(float64 v);
    void order(uint32 v);

    // interface methods

    RenderMode render_mode()        { return _render_mode; }
    void render_mode(RenderMode rm) { _render_mode = rm; }

    // filter methods

    float64 operator()(float64 in);
    float64 operator()(float64 in, float64 fc_);
    float64 operator()(float64 in, float64 fc_, float64 bw_);

    Buffer operator()(const Iterate<float64> & in);
    Buffer operator()(const Iterate<float64> & in, const Callable<float64> & fc_);
    Buffer operator()(const Iterate<float64> & in, const Callable<float64> & fc_, const Callable<float64> & bw_);

    void plot(boolean show_phase = false) const;
    void plot(float64 sample_rate, boolean show_phase = false) const;

private:

    // plot support functions

    Buffer _get_freq_axis(float64 sample_rate, float64 size_sec) const;
    Buffer _get_freq_response(float64 sample_rate, float64 size_sec) const;
    Buffer _get_impulse_response(float64 sample_rate, float64 size_sec) const;
    Buffer _get_phase_response(float64 sample_rate, float64 size_sec) const;
    uint32 _get_nfft(float64 sample_rate, float64 size_sec) const;

    // all filter calls eventually call this one:

    float64 _filter(float64 in, float64 fc, float64 bw);

    void _reset();

    float64 _sample_rate;
    float64 _freq_center;
    float64 _band_width;
    BandEdge _band_edge;
    float64 _gain_db_at_fc;
    float64 _gain_db_at_band_width;
    float64 _gain_db_baseline;
    uint32  _order;

    // Design Mode
    //    closed = constructed with kernel, can not update the desgin
    //    open = constructed from parameters, kernel can be redesigned
    enum DesignMode { CLOSED, OPEN };

    DesignMode _design_mode;

    RenderMode _render_mode;

    BiquadKernel _kernel;

    // history buffers
    std::vector<float64>_x_buf;
    int32 _x_ptr;

    std::vector<float64> _y_buf;
    int32 _y_ptr;
};


//-----------------------------------------------------------------------------
// inline implementation

inline
void Biquad::bw(float64 v)
{
    M_ASSERT_VALUE(_design_mode, ==, OPEN);
    M_ASSERT_MSG(v > 0.0, "bandwidth must be > 0 (" << v << " <= 0)");
    _band_width = v;
}


inline
void Biquad::fc(float64 v)
{
    M_ASSERT_VALUE(_design_mode, ==, OPEN);
    _freq_center = v;
}


inline
void Biquad::g0(float64 v)
{
    M_ASSERT_VALUE(_design_mode, ==, OPEN);
    _gain_db_at_fc = v;
}


inline
void Biquad::g1(float64 v)
{
    M_ASSERT_VALUE(_design_mode, ==, OPEN);
    _gain_db_at_band_width = v;
}


inline
void Biquad::g2(float64 v)
{
    M_ASSERT_VALUE(_design_mode, ==, OPEN);
    _gain_db_baseline = v;
}


inline
void Biquad::order(uint32 v)
{
    M_ASSERT_VALUE(_design_mode, ==, OPEN);
    M_ASSERT_MSG(v > 0, "order must be > 0 (" << v << " <= 0)");
    _order = v;
}


inline
float64 Biquad::operator()(float64 in)
{
    return (*this)(in, _freq_center, _band_width);
}


inline
float64 Biquad::operator()(float64 in, float64 fc_)
{
    M_ASSERT_MSG(
        _design_mode == OPEN,
        "Can't change freq center or bandwidth with 'CLOSED' design");

    return (*this)(in, fc_, _band_width);
}


inline
Buffer
Biquad::
operator()(const Iterate<float64> & in)
{
    return (*this)(in, Constant<float64>(_freq_center), Constant<float64>(_band_width));
}


inline
Buffer
Biquad::
operator()(const Iterate<float64> & in, const Callable<float64> & fc_)
{
    return (*this)(in, fc_, Constant<float64>(_band_width));
}


} // namespace
} // namespace


#endif