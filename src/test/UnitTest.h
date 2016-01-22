//-----------------------------------------------------------------------------
//
//  $Id: UnitTest.h 874 2014-09-08 02:21:29Z weegreenblobbie $
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

#ifndef _NSOUND_UNIT_TEST_H_
#define _NSOUND_UNIT_TEST_H_

// Some macros for messages.

#define TEST_HEADER \
    endl << THIS_FILE << ":"; cout.width(4); cout << __LINE__ \
    << ": "; cout.width(0); cout

#define TEST_ERROR_HEADER \
    " FAILURE!" << endl << THIS_FILE << ":"; cerr.width(4); cerr << __LINE__ \
    << ": ***Error!  "; cerr.width(0); cerr

#define SUCCESS \
    " SUCCESS!"

// The unit tests.

void Buffer_UnitTest();
void BufferResample_UnitTest();
void DelayLine_UnitTest();
void FFTransform_UnitTest();
void FilterBandPassFIR_UnitTest();
void FilterBandPassIIR_UnitTest();
void FilterBandRejectFIR_UnitTest();
void FilterBandRejectIIR_UnitTest();
void FilterCombLowPassFeedback_UnitTest();
void FilterDelay_UnitTest();
void FilterHighPassFIR_UnitTest();
void FilterHighPassIIR_UnitTest();
void FilterLeastSquaresFIR_UnitTest();
void FilterLowPassFIR_UnitTest();
void FilterLowPassIIR_UnitTest();
void FilterParametricEqualizer_UnitTest();
void Generator_UnitTest();
void Sine_UnitTest();
void Triangle_UnitTest();
void Wavefile_UnitTest();

#endif
