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

#ifndef _NSOUND_CALLABLE_HPP_
#define _NSOUND_CALLABLE_HPP_

#include <Nsound/Nsound.h>

//~#include <Nsound/Buffer.h>
#include <Nsound/Interfaces.hpp>


namespace Nsound
{


//~template <class T>
//~class Iterate
//~{

//~private:
//~    const std::vector<T> & _vector;
//~    typename std::vector<T>::const_iterator _itor;

//~public:
//~    Iterate(const std::vector<T> & t);
//~    auto begin() const -> decltype( _vector.cbegin() ) { return _vector.cbegin(); }
//~    auto end() const ->   decltype( _vector.cend() )   { return _vector.cend(); }
//~};


//~Buffer & operator<<(Buffer & lhs, const Iterate<float64> & rhs);


template <class T>
class Callable
{
public:
    virtual T operator()() const = 0;
    virtual ~Callable() {}
};


template <class T>
class Constant : public Callable<T>
{
public:
    Constant(T t) : _t(t) {}
    T operator()() const  {return _t;}
    virtual ~Constant() {}
private:
    const T _t;
};


template <class R, class T>
class Circular : public Callable<R>
{
public:
    Circular(T & t);
    R operator()();
    virtual ~Circular() {}
private:
    T & _vector;
    typename T::iterator _itor;
};



//-----------------------------------------------------------------------------
// inline implementation

//~template <class T>
//~Iterate<T>::
//~Iterate(const std::vector<T> & v)
//~    :
//~    _vector(v),
//~    _itor(v.begin())
//~{}


//~    inline
//~    Buffer &
//~    operator<<(Buffer & out, const Iterate<float64> & in)
//~    {
//~    //~    std::copy(in.begin(), in.end(), std::back_inserter(out.data()));
//~    //~    return out;
//~
//~        auto n = std::distance(in.begin(), in.end());
//~
//~        out.preallocate(n);
//~
//~        for(auto x : in) { out << x; }
//~
//~        return out;
//~    }


template <class R, class T>
Circular<R,T>::
Circular(T & t)
    :
    _vector(t),
    _itor(t.begin())
{}


template <class R, class T>
R
Circular<R,T>::
operator()()
{
    if(_itor == _vector.end()) _itor = _vector.begin();
    return *_itor++;
}


} // namespace

#endif

// :mode=c++: jEdit modeline