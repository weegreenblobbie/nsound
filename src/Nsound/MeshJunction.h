//-----------------------------------------------------------------------------
//
//  $Id: MeshJunction.h 874 2014-09-08 02:21:29Z weegreenblobbie $
//
//  Nsound is a C++ library and Python module for audio synthesis featuring
//  dynamic digital filters. Nsound lets you easily shape waveforms and write
//  to disk or plot them. Nsound aims to be as powerful as Csound but easy to
//  use.
//
//  Copyright (c) 2008-Present Nick Hilton
//
//  weegreenblobbie_yahoo_com (replace '_' with '@' and '.')
//
//   Based on Brook Eaton's Waveguide Mesh Drum project.
//   http://www-ccrma.stanford.edu/~be/drum/drum.htm
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
#ifndef _NSOUND_MESH_JUNCTION_H_
#define _NSOUND_MESH_JUNCTION_H_

#include <Nsound/Nsound.h>

namespace Nsound
{

//-----------------------------------------------------------------------------
//! WARNING: This is Experimental, you should not use this class as it may not
//! be working or will change in future releases of Nsound.
class MeshJunction
{
    public:

    //! Constructor
    MeshJunction(
        const float64 & leak_gain,
        const float64 & tau,
        const float64 & delta,
        const float64 & gamma);

    //! This operator can not assign neighbor pointers.
    MeshJunction(const MeshJunction & copy);

    //! Deconstructor
    ~MeshJunction();

    //! Clears all velicityes in the junctino.
    void
    clear();

    // DOXME
    float64
    getVelocity() const;

    //! This operator can not assign neighbor pointers.
    MeshJunction &
    operator=(const MeshJunction & rhs);

    // DOXME
    void setNeighborNorth(MeshJunction * ptr);

    // DOXME
    void setNeighborSouth(MeshJunction * ptr);

    // DOXME
    void setNeighborEast(MeshJunction * ptr);

    // DOXME
    void setNeighborWest(MeshJunction * ptr);

     //! Saves previous south and east velocities.
    void saveState();

    //! Adds velocity to this junction.
    void strike(const float64 & velocity);

    //! updates junction velocity.
    void updateVelocity();

    private:

    void
    updateBoundry();             //! updates junction velocity.

    float64 leak_gain_;          //! Leak gain

    float64 velocity_;           //! This junction's velocity.

    float64 velocity_north_;     //! Velocity going north.
    float64 velocity_south_;     //! Velocity going south.
    float64 velocity_east_;      //! Velocity going east.
    float64 velocity_west_;      //! Velocity going west.

    float64 previous_velocity_;
    float64 previous_velocity_north_;
    float64 previous_velocity_south_;
    float64 previous_velocity_east_;
    float64 previous_velocity_west_;

    float64 vc_;
    float64 yj_;
    float64 yc_;

    MeshJunction * neighbor_north_;
    MeshJunction * neighbor_south_;
    MeshJunction * neighbor_east_;
    MeshJunction * neighbor_west_;

}; // class MeshJunction


} // namespace Nsound

// :mode=c++: jEdit modeline
#endif
