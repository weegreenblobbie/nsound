//-----------------------------------------------------------------------------
//
//  $Id: Mesh2D.cc 904 2015-06-15 03:26:19Z weegreenblobbie $
//
//  Copyright (c) 2008 Nick Hilton
//
//  weegreenblobbie2_gmail_com (replace '_' with '@' and '.')
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

#include <Nsound/AudioStream.h>
#include <Nsound/Buffer.h>
#include <Nsound/Generator.h>
#include <Nsound/Mesh2D.h>
#include <Nsound/MeshJunction.h>

#include <cmath>
#include <cstdio>
#include <iostream>
#include <sstream>

using namespace Nsound;

using std::cout;
using std::cerr;
using std::endl;

//-----------------------------------------------------------------------------
Mesh2D::
Mesh2D(
    const float64 & sample_rate,
    uint32 width,
    uint32 height,
    const float64 & leak_gain,
    const float64 & tau,
    const float64 & delta,
    const float64 & gamma)
    :
    sample_rate_(sample_rate),
    X_(width),
    Y_(height),
    leak_gain_(leak_gain),
    tau_(tau),
    delta_(delta),
    gamma_(gamma),
    mesh_(NULL),
    center_(NULL),
    dump_mesh_(false),
    dirname_("."),
    prefix_("mesh")
{
    // Range check
    if(tau_ < 0.0)
    {
        tau_ = 1e-6;
    }
    else if(tau_ > 0.5)
    {
        tau_ = 0.5;
    }

    allocMemory();
}

//-----------------------------------------------------------------------------
Mesh2D::
Mesh2D(const Mesh2D & copy)
    :
    sample_rate_(0.0),
    X_(0),
    Y_(0),
    leak_gain_(0.0),
    tau_(0.0),
    delta_(0.0),
    gamma_(0.0),
    mesh_(NULL),
    center_(NULL),
    dump_mesh_(false),
    dirname_("."),
    prefix_("mesh")
{
    *this = copy;
}

//-----------------------------------------------------------------------------
Mesh2D::
~Mesh2D()
{
    freeMemory();
}

void
Mesh2D::
allocMemory()
{
    mesh_ = new MeshJunction * [X_ * Y_];

    // Allocate new MeshJunctions
    for(uint32 i = 0; i < X_ * Y_; ++i)
    {
        mesh_[i] = new MeshJunction(leak_gain_, tau_, delta_, gamma_);
    }

    // Set neighbors.
    for(uint32 x = 0; x < X_; ++x)
    {
        for(uint32 y = 0; y < Y_; ++y)
        {
            // Set north neighbor.
            if(y + 1 < Y_)
            {
                mesh(x,y)->setNeighborNorth(mesh(x,y+1));
            }

            // Set south neighbor.
            if(y >= 1)
            {
                mesh(x,y)->setNeighborSouth(mesh(x,y-1));
            }

            // Set east neighbor.
            if(x + 1 < X_)
            {
                mesh(x,y)->setNeighborEast(mesh(x+1,y));
            }

            // Set west neighbor.
            if(x >= 1)
            {
                mesh(x,y)->setNeighborWest(mesh(x-1,y));
            }
        }
    }

    // Set the center pointer.
    center_ = mesh(X_ / 2, Y_ / 2);
}

void
Mesh2D::
clear()
{
    for(uint32 i = 0; i < X_ * Y_; ++i)
    {
        mesh_[i]->clear();
    }
}

void
Mesh2D::
freeMemory()
{
    // delete junctions
    for(uint32 i = 0; i < X_ * Y_; ++i)
    {
        delete mesh_[i];
    }

    delete [] mesh_;
}

float64
Mesh2D::
getEnergy() const
{
    float64 energy = 0.0;

    for(uint32 i = 0; i < X_ * Y_; ++i)
    {
        float64 v = mesh_[i]->getVelocity();
        energy +=  v * v;
    }

    return energy / static_cast<float64>(X_ * Y_);
}

//-----------------------------------------------------------------------------
MeshJunction *
Mesh2D::
mesh(uint32 x, uint32 y)
{
    // -----------------
    // || array index ||
    // ||  x, y       ||
    // -----------------

    // -----------------------
    // ||  9  || 10  || 11  ||
    // || 0,3 || 1,3 || 2,3 ||
    // -----------------------
    // ||  6  ||  7  ||  8  ||
    // || 0,2 || 1,2 || 2,2 ||
    // -----------------------
    // ||  3  ||  4  ||  5  ||
    // || 0,1 || 1,1 || 2,1 ||
    // -----------------------
    // ||  0  ||  1  ||  2  ||
    // || 0,0 || 1,0 || 2,0 ||
    // -----------------------

    return mesh_[y * X_ + x];
}

//-----------------------------------------------------------------------------
Mesh2D &
Mesh2D::
operator=(const Mesh2D & rhs)
{
    if(this == & rhs)
    {
        return *this;
    }

    leak_gain_ = rhs.leak_gain_;
    tau_       = rhs.tau_;
    delta_     = rhs.delta_;
    gamma_     = rhs.gamma_;

    freeMemory();

    X_ = rhs.X_;
    Y_ = rhs.Y_;

    allocMemory();

    return *this;
}

Buffer
Mesh2D::
strike(
    const Buffer & x_pos,
    const Buffer & y_pos,
    const Buffer & velocity)
{
    Buffer output;

    uint32 x_size = x_pos.getLength();
    uint32 y_size = y_pos.getLength();

    for(uint32 i = 0; i < velocity.getLength(); ++i)
    {
        output << tick(x_pos[i % x_size], y_pos[i % y_size], velocity[i]);
    }

    return output;
}

void
Mesh2D::
writeMeshMap()
{
    if(!dump_mesh_)
    {
        return;
    }

    std::stringstream ss;

    ss << dirname_ << "/" << prefix_ << "_map.txt";

    FILE * f_out = fopen(ss.str().c_str(), "w");

    if(!f_out)
    {
        M_THROW("Unable to open '" << ss.str().c_str() << "' for writing.");
        return;
    }

    for(int32 y = Y_ - 1; y >= 0; --y)
    {
        for(uint32 x = 0; x < X_; ++x)
        {
            if(mesh(x,y) != NULL)
            {
                fprintf(f_out, " %5d", x + y * X_);
            }
            else
            {
                fprintf(f_out, " -1");
            }
        }
        fprintf(f_out, "\n");
    }
    fclose(f_out);
}


void
Mesh2D::
writeMeshFile(const std::string & mode)
{
    if(!dump_mesh_)
    {
        return;
    }

    std::stringstream ss;

    ss << dirname_ << "/" << prefix_ << ".txt";

    FILE * f_out = fopen(ss.str().c_str(), mode.c_str());

    if(!f_out)
    {
        M_THROW("Unable to open '" << ss.str().c_str() << "' for writing.");
        return;
    }

    for(int32 y = Y_ - 1; y >= 0; --y)
    {
        for(uint32 x = 0; x < X_; ++x)
        {
            fprintf(f_out, "%6.3f ", mesh(x,y)->getVelocity());
        }
    }
    fprintf(f_out, "\n");
    fclose(f_out);
}

void
Mesh2D::
setMeshDumping(
    boolean flag,
    const std::string & dirname,
    const std::string & prefix)
{
    dump_mesh_ = flag;
    dirname_ = dirname;
    prefix_ = prefix;
}

Buffer
Mesh2D::
strike(
    const float64 & x_pos,
    const float64 & y_pos,
    const float64 & velocity)
{
    Buffer output;

    clear();

    output << tick(x_pos, y_pos, velocity);

    writeMeshMap();
    writeMeshFile("w");

    for(uint32 i = 0; i < 44100; ++i)
    {
        output << tick(x_pos, y_pos, 0.0);

        writeMeshFile("a");

        center_->getVelocity();
    }

    // Shape the attack
    Generator gen(sample_rate_);

    Buffer attack = gen.drawLine(0.0025, 0.0, 1.0);

    return output * attack;
}

float64
Mesh2D::
tick(
    const float64 & x_pos,
    const float64 & y_pos,
    const float64 & velocity)
{
    float64 x_max = static_cast<float64>(X_);
    float64 y_max = static_cast<float64>(Y_);

    float64 x_strike = x_pos * x_max;
    float64 y_strike = y_pos * y_max;

    if(velocity > 0.0)
    {
        float64 dx = 0.5 / (X_ - 1);

        x_strike = 0.5 / 3.0;
        y_strike = 0.5 / 3.0;

        for(uint32 x = 2; x < X_; ++x)
        {
            for(uint32 y = 2; y < Y_; ++y)
            {
                float64 x_pos = (static_cast<float64>(x) - 1.0) * dx;
                float64 y_pos = (static_cast<float64>(y) - 1.0) * dx;

                float64 xx = x_pos - x_strike;
                xx *= xx;

                float64 yy = y_pos - y_strike;
                yy *= yy;

                float64 power = velocity * std::exp(-200.0 * (xx + yy));

                mesh(x, y)->strike(power);
            }
        }
    }

    for(uint32 i = 0; i < X_ * Y_; ++i)
    {
        mesh_[i]->saveState();
    }

    for(uint32 i = 0; i < X_ * Y_; ++i)
    {
        mesh_[i]->updateVelocity();
    }

    return center_->getVelocity();
}


