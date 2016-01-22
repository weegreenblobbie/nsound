//-----------------------------------------------------------------------------
//
//  $Id: MeshJunction.cc 874 2014-09-08 02:21:29Z weegreenblobbie $
//
//  Copyright (c) 2008 Nick Hilton
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

#include <Nsound/MeshJunction.h>

#include <iostream>

using namespace Nsound;

using std::cout;
using std::cerr;
using std::endl;

//-----------------------------------------------------------------------------
MeshJunction::
MeshJunction(
    const float64 & leak_gain,
    const float64 & tau,
    const float64 & delta,
    const float64 & gamma)
    :
    leak_gain_(leak_gain),
    velocity_(0.0),
    velocity_north_(0.0),
    velocity_south_(0.0),
    velocity_east_(0.0),
    velocity_west_(0.0),
    previous_velocity_(0.0),
    previous_velocity_north_(0.0),
    previous_velocity_south_(0.0),
    previous_velocity_east_(0.0),
    previous_velocity_west_(0.0),
    vc_(0.0),
    yj_(0.0),
    yc_(0.0),
    neighbor_north_(NULL),
    neighbor_south_(NULL),
    neighbor_east_(NULL),
    neighbor_west_(NULL)
{
    yj_ = 2.0 * ( (delta * delta) / (tau * tau * gamma * gamma) );
    yc_ = 2.0 * ( (delta * delta) / (tau * tau * gamma * gamma) - 2.0);

    // Ensure the leak gain is negative.
    if(leak_gain_ > 0.0)
    {
        leak_gain_ *= -1.0;
    }

//~    cout << "T = " << t << endl
//~         << "delta = " << delta << endl
//~         << "gamma = " << gamma << endl
//~         << "yj = " << yj_ << endl
//~         << "yc = " << yc_ << endl;
}

//-----------------------------------------------------------------------------
MeshJunction::
MeshJunction(const MeshJunction & copy)
    :
    leak_gain_(0.0),
    velocity_(0.0),
    velocity_north_(0.0),
    velocity_south_(0.0),
    velocity_east_(0.0),
    velocity_west_(0.0),
    previous_velocity_(0.0),
    previous_velocity_north_(0.0),
    previous_velocity_south_(0.0),
    previous_velocity_east_(0.0),
    previous_velocity_west_(0.0),
    vc_(0.0),
    yj_(0.0),
    yc_(0.0),
    neighbor_north_(NULL),
    neighbor_south_(NULL),
    neighbor_east_(NULL),
    neighbor_west_(NULL)
{
    *this = copy;
    // This operator can not assign neighbor pointers.
}

//-----------------------------------------------------------------------------
MeshJunction::
~MeshJunction()
{
}

void
MeshJunction::
clear()
{
    velocity_                =
    velocity_north_          =
    velocity_south_          =
    velocity_east_           =
    velocity_west_           =
    previous_velocity_       =
    previous_velocity_north_ =
    previous_velocity_south_ =
    previous_velocity_east_  =
    previous_velocity_west_  = 0.0;
}

float64
MeshJunction::
getVelocity() const
{
    return velocity_;
}

//-----------------------------------------------------------------------------
MeshJunction &
MeshJunction::
operator=(const MeshJunction & rhs)
{
    if(this == & rhs)
    {
        return *this;
    }

    leak_gain_ = rhs.leak_gain_;

    velocity_ = rhs.velocity_;

    velocity_north_ = rhs.velocity_north_;
    velocity_south_ = rhs.velocity_south_;
    velocity_east_  = rhs.velocity_east_;
    velocity_west_  = rhs.velocity_west_;

    previous_velocity_       = rhs.previous_velocity_;
    previous_velocity_north_ = rhs.previous_velocity_north_;
    previous_velocity_south_ = rhs.previous_velocity_south_;
    previous_velocity_east_  = rhs.previous_velocity_east_;
    previous_velocity_west_  = rhs.previous_velocity_west_;

    vc_ = rhs.vc_;
    yj_ = rhs.yj_;
    yc_ = rhs.yc_;

    // This operator can not assign neighbor pointers.

    return *this;
}

void
MeshJunction::
setNeighborNorth(MeshJunction * ptr)
{
    neighbor_north_ = ptr;
}

void
MeshJunction::
setNeighborSouth(MeshJunction * ptr)
{
    neighbor_south_ = ptr;
}

void
MeshJunction::
setNeighborEast(MeshJunction * ptr)
{
    neighbor_east_ = ptr;
}

void
MeshJunction::
setNeighborWest(MeshJunction * ptr)
{
    neighbor_west_ = ptr;
}

void
MeshJunction::
strike(const float64 & velocity)
{
    velocity_ += velocity;

    velocity_north_ += yj_ / 8.0 * velocity;
    velocity_south_ += yj_ / 8.0 * velocity;
    velocity_east_  += yj_ / 8.0 * velocity;
    velocity_west_  += yj_ / 8.0 * velocity;
}

void
MeshJunction::
saveState()
{
    previous_velocity_       = velocity_;
    previous_velocity_north_ = velocity_north_;
    previous_velocity_south_ = velocity_south_;
    previous_velocity_east_  = velocity_east_;
    previous_velocity_west_  = velocity_west_;
}

void
MeshJunction::
updateVelocity()
{
    velocity_ = 2.0 / yj_ * (  previous_velocity_north_
                             + previous_velocity_south_
                             + previous_velocity_east_
                             + previous_velocity_west_
                             + yc_ * vc_);

    if(neighbor_north_)
    {
        neighbor_north_->velocity_south_ = velocity_ - previous_velocity_north_;
    }

    if(neighbor_south_)
    {
        neighbor_south_->velocity_north_ = velocity_ - previous_velocity_south_;
    }

    if(neighbor_west_)
    {
        neighbor_west_->velocity_east_ = velocity_ - previous_velocity_west_;
    }

    if(neighbor_east_)
    {
        neighbor_east_->velocity_west_ = velocity_ - previous_velocity_east_;
    }

    vc_ = velocity_ - vc_;

    updateBoundry();

}


void
MeshJunction::
updateBoundry()
{
    if(neighbor_north_ == NULL)
    {
        float64 vtemp = velocity_north_;

        velocity_north_ = leak_gain_ * velocity_south_;

        if(neighbor_south_)
        {
            neighbor_south_->velocity_north_ = vtemp;
        }

    }

    if(neighbor_south_ == NULL)
    {
        float64 vtemp = velocity_south_;

        velocity_south_ = leak_gain_ * velocity_north_;

        if(neighbor_north_)
        {
            neighbor_north_->velocity_south_ = vtemp;
        }

    }

    if(neighbor_east_ == NULL)
    {
        float64 vtemp = velocity_east_;

        velocity_east_ = leak_gain_ * velocity_west_;

        if(neighbor_west_)
        {
            neighbor_west_->velocity_east_ = vtemp;
        }

    }

    if(neighbor_west_ == NULL)
    {
        float64 vtemp = velocity_west_;

        velocity_west_ = leak_gain_ * velocity_east_;

        if(neighbor_east_)
        {
            neighbor_east_->velocity_west_ = vtemp;
        }

    }
}

