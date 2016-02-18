//-----------------------------------------------------------------------------
//
//  Copyright (c) 2009 to Present Nick Hilton
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

%ignore Nsound::biquad::BandEdge;
%ignore Nsound::biquad::CHEBYSHEV_1;
%ignore Nsound::biquad::CHEBYSHEV_2;
%ignore Nsound::biquad::ELLIPTIC;
%ignore Nsound::biquad::Biquad::from_json(const picojson::value &);
%ignore Nsound::biquad::FilterBank::from_json(const picojson::value &);

%rename(_nsound_biquad_BUTTERWORTH)  Nsound::biquad::BUTTERWORTH;
%rename(_nsound_biquad_Biquad)       Nsound::biquad::Biquad;
%rename(_nsound_biquad_BiquadKernel) Nsound::biquad::BiquadKernel;
%rename(_nsound_biquad_DesignType)   Nsound::biquad::DesignType;
%rename(_nsound_biquad_FilterBank)   Nsound::biquad::FilterBank;
%rename(_nsound_biquad_cas2dir)      Nsound::biquad::cas2dir;
%rename(_nsound_biquad_hpeq_design)  Nsound::biquad::hpeq_design;
