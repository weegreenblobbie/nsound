//-----------------------------------------------------------------------------
//
//  $Id: Mesh2D.h 874 2014-09-08 02:21:29Z weegreenblobbie $
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
#ifndef _NSOUND_MESH_2D_H_
#define _NSOUND_MESH_2D_H_

#include <Nsound/Nsound.h>

#include <iostream>
#include <vector>

namespace Nsound
{

// forwards
class Buffer;
class MeshJunction;

//-----------------------------------------------------------------------------
//! WARNING: This is Experimental, you should not use this class as it may not
//! be working or will change in future releases of Nsound.
class Mesh2D
{
    public:

    //! Constructor
    Mesh2D(
        const float64 & sample_rate,
        uint32 width,
        uint32 height,
        const float64 & leak_gain,
        const float64 & tau,
        const float64 & delta = 6.0,
        const float64 & gamma = 8.0);

    //! Copy Constructor
    Mesh2D(const Mesh2D & copy);

    //! Destructor
    ~Mesh2D();

    //! Clears all MeshJunctions to zero.
    void
    clear();

    //! Calculate total energy in the mesh.
    float64
    getEnergy() const;

    //! Assignment operator.
    Mesh2D &
    operator=(const Mesh2D & rhs);

    //! Hit the mesh many times at different locations.
    Buffer
    strike(
        const Buffer & x_pos,
        const Buffer & y_pos,
        const Buffer & velocity);

    //! Hit the mesh once.
    Buffer
    strike(
        const float64 & x_pos,
        const float64 & y_pos,
        const float64 & velocity);

    //! Enables the dumping of mesh values to disk for use with mesh2blender.py
    void
    setMeshDumping(
        boolean flag,
        const std::string & dirname = ".",
        const std::string & prefix = "mesh");

    private:

    //! Allocates memory for the mesh
    void
    allocMemory();

    //! Frees memory from the mesh
    void
    freeMemory();

    //! Write to disk the mesh map, used with mesh2blender.py
    void
    writeMeshMap();

    void
    writeMeshFile(const std::string & mode);

    MeshJunction *
    mesh(uint32 x, uint32 y);

    //! Calculate one sample.
    float64
    tick(
        const float64 & x_pos,
        const float64 & y_pos,
        const float64 & velocity);

    float64 sample_rate_;

    uint32 X_;     //! The number of MeshJunctions columns.
    uint32 Y_;     //! The number of MeshJunctions rows.

    float64 leak_gain_; //! MeshJunction leak gain
    float64 tau_;       //! MeshJunction tau
    float64 delta_;     //! MeshJunction delta
    float64 gamma_;     //! MeshJunction gamma

    MeshJunction ** mesh_; //! Array of pointers.

    MeshJunction * center_; //! Center MeshJunction where final sound is collected.

    boolean dump_mesh_;

    std::string dirname_;
    std::string prefix_;

}; // class Mesh2D

} // namespace Nsound

// :mode=c++: jEdit modeline
#endif
