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
#ifndef _NSOUND_BIQUAD_BIQUAD_H_
#define _NSOUND_BIQUAD_BIQUAD_H_

#include <Nsound/Nsound.h>

#include <Nsound/CircularIterators.h>
#include <Nsound/Interfaces.hpp>

#include <Nsound/biquad/Kernel.h>


namespace Nsound
{
namespace biquad
{


class Biquad : public RenderModal
{

public:

    static Biquad from_json(const std::string & in);

    Biquad(const BiquadKernel & bk);

    Biquad(
        float64 sample_rate,
        float64 freq_center,
        float64 bandwidth,
        float64 gain_db_at_fc,        // g0
        float64 gain_db_at_bandwidth, // g1
        float64 gain_db_baseline,     // g2
        uint32  order);

    std::string to_json() const;

    void update_design();

    // QT style getters

    float64 bw() const { return _band_width; }
    float64 fc() const { return _freq_center; }
    float64 lo() const { return _freq_center - 0.5 * _band_width; }
    float64 hi() const { return _freq_center + 0.5 * _band_width; }
    float64 g0() const { return _gain_db_at_fc; }
    float64 g1() const { return _gain_db_at_bandwidth; }
    float64 g2() const { return _gain_db_baseline; }
    uint32  order() const { return _order; }

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

    FloatVector operator()(Callable<float64> & in);
    FloatVector operator()(Callable<float64> & in, Callable<float64> & fc_);
    FloatVector operator()(Callable<float64> & in, Callable<float64> & fc_, Callable<float64> & bw_);

private:

    float64 _filter(float64 in, float64 fc, float64 bw);

    float64 _sample_rate;
    float64 _freq_center;
    float64 _band_width;
    float64 _gain_db_at_fc;
    float64 _gain_db_at_bandwidth;
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
    std::vector<float64>                 _x_buf;
    std::vector<float64>::iterator       _x_ptr;
    std::vector<float64>::const_iterator _x_begin;
    std::vector<float64>::const_iterator _x_end;

    std::vector<float64>                 _y_buf;
    std::vector<float64>::iterator       _y_ptr;
    std::vector<float64>::const_iterator _y_begin;
    std::vector<float64>::const_iterator _y_end;
};


//-----------------------------------------------------------------------------
// inline implementation


void Biquad::bw(float64 v)
{
    M_ASSERT_MSG(v > 0.0, "bandwidth must be > 0 (" << v << " <= 0)");
    _band_width = v;
}


void Biquad::fc(float64 v)
{
    _freq_center = v;
}


void Biquad::g0(float64 v)
{
    _gain_db_at_fc = v;
}


void Biquad::g1(float64 v)
{
    _gain_db_at_bandwidth = v;
}


void Biquad::g2(float64 v)
{
    _gain_db_baseline = v;
}


void Biquad::order(uint32 v)
{
    M_ASSERT_MSG(v > 0, "order must be > 0 (" << v << " <= 0)");
    _order = v;
}








} // namespace
} // namespace