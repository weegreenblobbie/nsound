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


FilterId
FilterBank::
add(const Biquad & bq)
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
        if(ptr)
        {
            bk = cas2dir(bk, ptr->kernel());
        }
    }

    return bk;
}


void
FilterBank::
render_mode(RenderMode rm)
{
    for(auto ptr : _filters)
    {
        if(ptr)
        {
            ptr->render_mode(rm);
        }
    }
}



} // namespace
} // namespace
