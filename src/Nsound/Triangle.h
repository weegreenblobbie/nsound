//-----------------------------------------------------------------------------
//
//  $Id: Triangle.h 874 2014-09-08 02:21:29Z weegreenblobbie $
//
//  Nsound is a C++ library and Python module for audio synthesis featuring
//  dynamic digital filters. Nsound lets you easily shape waveforms and write
//  to disk or plot them. Nsound aims to be as powerful as Csound but easy to
//  use.
//
//  Copyright (c) 2006 Nick Hilton
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
#ifndef _NSOUND_TRIANGLE_H_
#define _NSOUND_TRIANGLE_H_

#include <Nsound/Nsound.h>
#include <Nsound/Generator.h>

namespace Nsound
{

// Forward declaration
class Buffer;

//-----------------------------------------------------------------------------
//! Triangle generator

class Triangle : public Generator
{
    public:
    Triangle(const Nsound::float64 & sample_rate);

    //! Triangle with 2 points                                               //
    //                                                                       //
    //!       /\                                                             //
    //!      /  \                                                            //
    //!     /    \                                                           //
    //!    /      \                                                          //
    //!   /        \       release                                           //
    //!  /          \    |-----|                                             //
    //!  |------|    \        /                                              //
    //!   attack      \      /                                               //
    //!                \    /                                                //
    //!                 \  /                                                 //
    //!                  \/                                                  //
    //!                                                                      //
    //!                                                                      //
    Triangle(
        const Nsound::float64 & sample_rate,
        const Nsound::float64 & attack_time,
        const Nsound::float64 & release_time);

    //! Triangle with 6 points                                               //
    //                                                                       //
    //!         2                                                            //
    //!        /\                                                            //
    //!       /  \                                                           //
    //!      /    \                                                          //
    //!     /      \                                                         //
    //!  --1        3--4          6--                                        //
    //!                 \        /                                           //
    //!                  \      /                                            //
    //!                   \    /                                             //
    //!                    \  /                                              //
    //!                     \/                                               //
    //!                      5                                               //
    //!                                                                      //
    //!  |----------lambda----------|                                        //
    Triangle(
        const Nsound::float64 & sample_rate,
        const Nsound::float64 & percent_lambda_1,
        const Nsound::float64 & percent_lambda_2,
        const Nsound::float64 & amplitude_2,
        const Nsound::float64 & percent_lambda_3,
        const Nsound::float64 & percent_lambda_4,
        const Nsound::float64 & percent_lambda_5,
        const Nsound::float64 & amplitude_5,
        const Nsound::float64 & percent_lambda_6);

};

};

#endif
