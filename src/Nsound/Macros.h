//-----------------------------------------------------------------------------
//
//  $Id: Macros.h 912 2015-07-26 00:50:29Z weegreenblobbie $
//
//  Nsound is a C++ library and Python module for audio synthesis featuring
//  dynamic digital filters. Nsound lets you easily shape waveforms and write
//  to disk or plot them. Nsound aims to be as powerful as Csound but easy to
//  use.
//
//  Copyright (c) 2004-Present Nick Hilton
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
#ifndef _NSOUND_MACROS_H_
#define _NSOUND_MACROS_H_

#define M_LINE_PREFIX __FILE__ << ":" << __LINE__ << ": "
#define M_PRINT_LINE printf("%s:%4d: ", __FILE__, __LINE__)

// Round a to nearest higher multiple of b
#define M_ROUND_UP(a, b) ((a % b != 0) ?  (a - a % b + b) : a)

// Fancy float32 round function to nearst N decimal places.
#define M_ROUND_FLOAT32(f, N) (floorf((f) * 1.0E##N + 0.5f) / 1.0E##N)

#define M_ABS(x)    ((x) < 0 ? -(x) : (x))
#define M_MAX(x, y) ((x) > (y) ? (x) : (y))
#define M_MIN(x, y) ((x) < (y) ? (x) : (y))
#define M_SQUARE(x) ((x) * (x))


namespace Nsound
{

inline void __throw__(const std::string & message)
{
    std::cerr << message << std::endl;
    std::cerr.flush();
    throw Nsound::Exception(message);
}

};


#define M_CHECK_PTR(ptr)               \
	if((ptr) == NULL)                  \
	{                                  \
		std::stringstream ss;          \
		ss << M_LINE_PREFIX            \
			<< "FATAL ERROR: "         \
			<< #ptr                    \
			<< " is NULL";             \
		Nsound::__throw__(ss.str());   \
	}


#define M_ASSERT_VALUE(a, op, value)              \
	if(! ((a) op (value))  )                      \
	{                                             \
		std::stringstream ss;                     \
		ss << M_LINE_PREFIX                       \
			<< "FATAL ERROR: "                    \
			<< #a                                 \
			<< " " #op " "                        \
			<< #value                             \
			<< " condition not met "              \
			<< "("                                \
			<< (a)                                \
			<< " " #op " "                        \
			<< (value)                            \
			<< ")";                               \
		Nsound::__throw__(ss.str());              \
	}


#define M_ASSERT_MSG(expr, message)               \
	if(! (expr) )                                 \
	{                                             \
		std::stringstream ss;                     \
		ss << M_LINE_PREFIX                       \
			<< "Asertion Failed: "                \
			<< #expr                              \
			<< " : "                              \
			<< message;                           \
		Nsound::__throw__(ss.str());              \
	}


#define M_THROW(message)                          \
	{                                             \
		std::stringstream ss;                     \
		ss << M_LINE_PREFIX                       \
			<< message;                           \
		Nsound::__throw__(ss.str());              \
	}

#endif

// :mode=c++: jEdit modeline