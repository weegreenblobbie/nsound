//////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2023 Nick Hilton
//
//  weegreenblobbie2_gmail_com (replace '_' with '@' and '.')
//
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
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
//////////////////////////////////////////////////////////////////////////////

#include <Nsound/LookupTable.h>

#include <algorithm>

#include <cassert> // while debugging
#include <iostream>

using namespace Nsound;

void
LookupTable::
add_point(const float64 x, const float64 y)
{
    if (_x.empty())
    {
        _x.push_back(x);
        _y.push_back(y);
        return;
    }

    auto location = std::lower_bound(_x.begin(), _x.end(), x);
    if (location == _x.end())
    {
        _x.push_back(x);
        _y.push_back(y);
    }
    else
    {
        auto offset = std::distance(location, _x.begin());
        _x.insert(_x.begin() + offset, x);
        _y.insert(_y.begin() + offset, y);
    }

    assert(std::is_sorted(_x.begin(), _x.end())); // "x should be sorted"
}


float64
LookupTable::
interpolate(const float64 x) const
{
    assert(not _x.empty()); // LookupTable should not be empty.

    // Limit checks.
    if (x < *_x.begin()) return *_y.begin();
    if (x > _x.back()) return _y.back();

    auto location = std::lower_bound(_x.begin(), _x.end(), x,
        [](const float64 & lhs, const float64 & rhs){ return lhs <= rhs; }
    );
    auto index = std::distance(_x.begin(), location);
    if (index > 0) --index;

    auto x0 = _x[index];
    auto x1 = _x[index + 1];

    auto total_distance = x1 - x0;

    auto weight = (x - x0) / total_distance;

    return _y[index] * (1.0 - weight) + _y[index + 1] * weight;
}
