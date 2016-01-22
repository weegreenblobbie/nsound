//-----------------------------------------------------------------------------
//
//  $Id: test_mesh.cc 874 2014-09-08 02:21:29Z weegreenblobbie $
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

#include <Nsound/NsoundAll.h>

#include "Test.h"

#include <cmath>
#include <iostream>
#include <sstream>

using namespace Nsound;

using std::cerr;
using std::cout;
using std::endl;
using std::flush;

static const char * THIS_FILE = "test_mesh.cc";

int main(int argc, char ** argv)
{
//~    for(int x = 3; x <= 19; x += 4)
//~    {
//~        for(float64 leak = 0.87; leak <= 0.99; leak += 0.06)
//~        {
//~            for(float64 tau = 0.005; tau <= 0.16; tau *= 2.0)
//~            {
//~                char buffer[1024];
//~
//~                sprintf(buffer, "mesh_x%02d_y19_leak%4.2f_tau%5.3f.wav",
//~                    x,
//~                    leak,
//~                    tau);
//~
//~                cout << "writing " << buffer;
//~
//~                Mesh2D mesh(44100.0, x, 19, leak, tau);
//~
//~                Buffer hit = mesh.strike(0.25, 0.25, 1.0);
//~
//~                Buffer output;
//~
//~                output << hit << hit << hit;
//~                output.normalize();
//~
//~                output *= 0.8;
//~
//~                output >> buffer;
//~
//~                cout << endl;
//~
//~            }
//~        }
//~    }

    //          sr       x   y   leak   tau
    Mesh2D mesh(44100.0, 11, 19, 0.88,  0.010);

    Buffer output;

    output << mesh.strike(0.333, 0.333, 3.0);

    output.normalize();

    output >> "mesh.wav";

    return 0;
}


