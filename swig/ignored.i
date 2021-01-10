///////////////////////////////////////////////////////////////////////////////
//
//  $Id: ignored.i 912 2015-07-26 00:50:29Z weegreenblobbie $
//
//  Copyright (c) 2009 to Present Nick Hilton
//
//  weegreenblobbie2_gmail_com (replace '_' with '@' and '.')
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
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
///////////////////////////////////////////////////////////////////////////////

%ignore Nsound::AudioStream::operator=;
%ignore Nsound::AudioStream::operator[];
%ignore Nsound::AudioStreamSelection::operator=;
%ignore Nsound::Axes::operator=;
%ignore Nsound::Buffer::begin;
%ignore Nsound::Buffer::end;
%ignore Nsound::Buffer::operator=;
%ignore Nsound::Buffer::operator[];
%ignore Nsound::BufferSelection::operator=;
%ignore Nsound::BufferWindowSearch::operator=;
%ignore Nsound::circular_iterator::operator=;
%ignore Nsound::circular_iterator::operator++;
%ignore Nsound::circular_iterator::operator--;
%ignore Nsound::const_circular_iterator::operator=;
%ignore Nsound::const_circular_iterator::operator++;
%ignore Nsound::const_circular_iterator::operator--;
%ignore Nsound::EnvelopeAdsr::operator=;
%ignore Nsound::FFTChunk::operator=;
%ignore Nsound::FilterAllPass::operator=;
%ignore Nsound::FilterCombLowPassFeedback::operator=;
%ignore Nsound::FilterDelay::operator=;
%ignore Nsound::FilterFlanger::operator=;
%ignore Nsound::FilterIIR::operator=;
%ignore Nsound::FilterLeastSquaresFIR::operator=;
%ignore Nsound::FilterMovingAverage::operator=;
%ignore Nsound::FilterPhaser::operator=;
%ignore Nsound::FilterStageIIR::operator=;
%ignore Nsound::Generator::operator=;
%ignore Nsound::Granulator::operator=;
%ignore Nsound::Hat::operator=;
%ignore Nsound::Mesh2D::operator=;
%ignore Nsound::Plotter::show;
%ignore Nsound::ReverberationRoom::operator=;
%ignore Nsound::RngTausworthe::operator=;
%ignore Nsound::Spectrogram::operator=;
%ignore Nsound::Stretcher::operator=;

// :mode=c++:
