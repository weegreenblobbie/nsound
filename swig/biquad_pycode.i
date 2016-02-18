//-----------------------------------------------------------------------------
//
//  Copyright (c) 2016 to Present Nick Hilton
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

%pythoncode
%{

#------------------------------------------------------------------------------
# BEGIN biquad.i

# create a biquad "namespace"

class biquad(object):

    BUTTERWORTH = _nsound_biquad_BUTTERWORTH
    Biquad = _nsound_biquad_Biquad
    BiquadKernel = _nsound_biquad_BiquadKernel
    FilterBank = _nsound_biquad_FilterBank
    cas2dir = _nsound_biquad_cas2dir
    hpeq_design = _nsound_biquad_hpeq_design

# END biquad.i
#------------------------------------------------------------------------------

%}

// :mode=python:
