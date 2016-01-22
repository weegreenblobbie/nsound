//-----------------------------------------------------------------------------
//
//  $Id: Filter.h 911 2015-07-10 03:04:24Z weegreenblobbie $
//
//  Nsound is a C++ library and Python module for audio synthesis featuring
//  dynamic digital filters. Nsound lets you easily shape waveforms and write
//  to disk or plot them. Nsound aims to be as powerful as Csound but easy to
//  use.
//
//  Copyright (c) 2007 Nick Hilton
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

#ifndef _NSOUND_INTERFACES_HPP_
#define _NSOUND_INTERFACES_HPP_

#include <Nsound/Nsound.h>
#include <Nsound/Interfaces.hpp>


namespace nsound
{


template <class T>
class Constant : public Callable<T>
{
public:
    Constant(const T & t) : _t(t) {}
    T operator()          {return _t;}
    bool finished() const { return false; }
private:
    const T _t;
};


template <class R, class T>
class Iterate : public Callable<R>
{
public:
    Iterate(const T & t);
    R operator();
    bool finished() const { return _itor == _end; }
    T::const_iterator cbegin() { return _begin; }
    T::const_iterator cend()   { return _end; }
private:
    T::const_iterator _begin;
    T::const_iterator _end;
    T::const_iterator _itor;
};


template <class R, class T>
class Circular : public Callable<R>
{
public:
    Circular(const T & t);
    R operator();
    bool finished() const { return false; }
private:
    const T::const_iterator _begin;
    const T::const_iterator _end;
    T::const_iterator       _itor;
};



//-----------------------------------------------------------------------------
// inline implementation


template <class R, class T>
Iterate::
Iterate(const T & t)
    :
    _begin(t.cbegin());
    _end(t.cend()),
    _itor(t.cbegin())
{}


template <class R, class T>
R
Iterate::
operator()
{
    M_ASSERT_MSG(_itor < _end, "already reached end!");
    R temp{*_itor};
    ++itor;
    return temp;
}


template <class R, class T>
Circular::
Circular(const T & t)
    :
    _begin(t.cbegin()),
    _end(t.cend()),
    _itor(t.cbegin())
{}


template <class R, class T>
R
Circular::
operator()
{
    if(_itor == _end) _itor = _begin;
    R temp{*_itor};
    ++itor;
    return temp;
}


} // namespace

#endif

// :mode=c++: jEdit modeline