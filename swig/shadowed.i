//-----------------------------------------------------------------------------
//
//  $Id: shadowed.i 912 2015-07-26 00:50:29Z weegreenblobbie $
//
//  Copyright (c) 2009 to Present Nick Hilton
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


//-----------------------------------------------------------------------------
// Buffer stuff


%rename(__lshift__) Nsound::Buffer::opertor<<;
%rename(_get_at_index) Nsound::Buffer::opertor[];

%extend


// %typemap(out) Nsound::Buffer & operator<<
// {
//     /* %typemap(out) Nsound::Buffer & operator<< */
//
//     if(result) { /* suppress unused warning */ }
//     Py_INCREF($self);
//     $result = $self;
// }

//%rename(__getitem__) Nsound::Buffer::operator[];
//
//%typemap(out) Nsound::float64 & Nsound::Buffer::__getitem__
//{
//    /* %typemap(out) Nsound::float64 & Nsound::Buffer::__getitem__ */
//
//    if(result) { /* suppress unused warning */ }
//    Py_INCREF($self);
//    $result = $self;
//}


%feature("shadow") Nsound::Buffer::swig_hook()
%{

#------------------------------------------------------------------------------
# from swig/shadowed.i
#
def __add__(self, rhs):
    temp = Buffer(self)
    temp += rhs
    return temp

def __len__(self):
    return self.getLength()

def __sub__(self, rhs):
    temp = Buffer(self)
    temp -= rhs
    return temp

def __div__(self, rhs):
    temp = Buffer(self)
    temp /= rhs
    return temp

def __mul__(self, rhs):
    temp = Buffer(self)
    temp *= rhs
    return temp

def __radd__(self, lhs):
    temp = Buffer(self)
    temp += lhs
    return temp

def __rmul__(self, lhs):
    temp = Buffer(self)
    temp *= lhs
    return temp

def __rsub__(self, lhs):
    temp = Buffer(self) * -1.0
    temp += lhs
    return temp

def __rdiv__(self, lhs):
    temp = Buffer(self)
    for i in range(temp.getLength()):

        if abs(self[i]) > 0.0:
            temp[i] = lhs / self[i]
        else:
            temp[i] = lhs / 1e-20

    return temp

def __ilshift__(self, rhs):
    self << rhs
    return self

def __irshift__(self, rhs):
    raise Exception("Operator not supported")

def __lshift__(self, rhs):

    # Read wavefile
    if isinstance(rhs, str):
        return _Nsound.Buffer___lshift__(self, rhs)

    elif isinstance(rhs, Buffer):
        return _Nsound.Buffer___lshift__(self, rhs)

    else:

        # try to convert to a float
        try:
            return _Nsound.Buffer___lshift__(self, float(rhs))

        except TypeError:
            pass

        # try to convert to Buffer

        try:
            return _Nsound.Buffer___lshift__(self, Buffer(rhs))

        except TypeError:
            pass

        # assume the object is iteratble
        try:
           for x in rhs:
               self = _Nsound.Buffer___lshift__(self, float(x))
        except TypeError:
           raise TypeError(str(type(rhs)) + ' is not iterable')

    return self

def __rshift__(self, rhs):

    if isinstance(rhs, list):
        for s in self:
            rhs.append(s)

    elif isinstance(rhs, AudioPlayback) or isinstance(rhs, AudioPlaybackRt):
        rhs.play(self)
        return

    elif not isinstance(rhs, str):
        raise TypeError("right hand side argument must be of type str or list")

    self.writeWavefile(rhs)

def __getitem__(self,i):

    if isinstance(i, int):

        if i >= self.getLength():
            raise StopIteration

        return _Nsound.Buffer_get_at_index(self, i)

    elif isinstance(i, slice):

        b = Buffer()

        for index in xrange(*i.indices(_Nsound.Buffer_getLength(self))):
            b << _Nsound.Buffer_get_at_index(self, index)

        return b

    else:
        raise StopIteration

def __setitem__(self,i, d):

    if isinstance(i, int):

        if i >= self.getLength():
            raise IndexError("Error: Index out of bounds, %d >= %d" %(
                i, self.getLength()))

        if type(d) != types.IntType and type(d) != types.FloatType:
            raise TypeError("Error: Argument must be numeric")

        _Nsound.Buffer_set_at_index(self, i, d)

    else:
        raise TypeError("Expecting index type int, but got %s" %(type(i)))

def __pow__(self, power):
    return _Nsound.__xor__(self, power)

def __ipow__(self, power):
    self = _Nsound.__xor__(self, power)
    return self

def __str__(self):
    return "Nsound.Buffer holding %d samples" % self.getLength()

def toList(self):

    array = []

    for i in range(self.getLength()):
        array.append(self.get_at_index(i))

    return array

# Pickle interface

def __getstate__(self):
    '''
    Returns a binary string
    '''
    return _Nsound.Buffer_write(self)

def __setstate__(self, s):
    '''
    Reads the binary string.
    '''

    self.__init__()
    _Nsound.Buffer_read(self, s)

#
#------------------------------------------------------------------------------

%}


//-----------------------------------------------------------------------------
// Injected AudioStream python code

// %typemap(out) Nsound::AudioStream & operator<<
// {
//     /* %typemap(out) Nsound::AudioStream & operator<< */
//
//     if(result) { /* suppress unused warning */ }
//     Py_INCREF($self);
//     $result = $self;
// }

//%typemap(out) Nsound::Buffer & AudioStream::get_at_index
//{
//    /* %typemap(out) Nsound::Buffer & AudioStream::get_at_index */
//
//    if(result) { /* suppress unused warning */ }
//    Py_INCREF($self);
//    $result = $self;
//}

%rename(_get_at_index) Nsound::AudioStream::operator[];

%feature("shadow") Nsound::AudioStream::_swig_hook()
%{

#------------------------------------------------------------------------------
# from swig/shadowed.i
#
def __add__(self, other):
    temp = AudioStream(self)
    temp += other
    return temp

def __len__(self):
    return self.getLength()

def __sub__(self, other):
    temp = AudioStream(self)
    temp -= other
    return temp

def __div__(self, other):
    temp = AudioStream(self)
    temp /= other
    return temp

def __mul__(self, other):
    temp = AudioStream(self)
    temp *= other
    return temp

def __radd__(self, other):
    temp = AudioStream(self)
    temp += other
    return temp

def __rmul__(self, other):
    temp = AudioStream(self)
    temp *= other
    return temp

def __rsub__(self, other):
    temp = AudioStream(self) * -1.0
    temp += other
    return temp

def __rdiv__(self, other):
    temp = AudioStream(self)
    for i in range(temp.getNChannels()):

        temp[i] = other / temp[i];

    return temp

def __ilshift__(self, rhs):
    raise Exception("Operator not supported")

def __irshift__(self, rhs):
    raise Exception("Operator not supported")

def __lshift__(self, rhs):

    # Read wavefile
    if isinstance(rhs, str):
        return _Nsound.AudioStream___lshift__(self, rhs)

    elif isinstance(rhs, AudioStream):
        return _Nsound.AudioStream___lshift__(self, rhs)

    elif isinstance(rhs, Buffer):
        return _Nsound.AudioStream___lshift__(self, rhs)

    else:

        # try to convert to a float
        try:
            return _Nsound.AudioStream___lshift__(self, float(rhs))

        except TypeError:
            pass

        # assume the object is iteratble
        try:
           for x in rhs:
               self = _Nsound.AudioStream___lshift__(self, float(x))
        except TypeError:
           raise TypeError(str(type(rhs)) + ' is not iterable')

    return self

def __rshift__(self, rhs):

    if isinstance(rhs, list):
        temp = self.getMono()
        for s in temp:
            rhs.append(s)

    elif isinstance(rhs, AudioPlayback) or isinstance(rhs, AudioPlaybackRt):
        rhs.play(self)
        return

    elif not isinstance(rhs, str):
        raise TypeError("Error: right hand side argument must be of type str")

    self.writeWavefile(rhs)

def __getitem__(self,i):

    if isinstance(i, int):
        if abs(i) >= self.getNChannels():
            raise StopIteration

        if i < 0:
            i += self.getNChannels()

        return _Nsound.AudioStream_get_at_index(self, i)

    elif isinstance(i, slice):

        indices = range(*i.indices())

        n_channels = len(indices)

        a = AudioStream(self.getSampleRate(), n_channels)

        k = 0
        for index in indices:
            a[k] = Buffer(_Nsound.AudioStream_get_at_index(self, index))
            k += 1

        return a

    else:

        raise StopIteration

def __setitem__(self, i, buf):

    if isinstance(i, int):
        if i >= self.getNChannels():
            raise IndexError("Error: Index out of bounds, %d >= %d" %(
                i, self.getNChannels()))

        if not isinstance(buf, Buffer):
            raise TypeError(
                "Error: Input argument must be of class Buffer, not %s" %(
                    type(buf)))

        self.set_at_index(i, buf)

    else:
        raise TypeError("Expecting index type int, but got %s" %(type(i)))

def __pow__(self, power):
    return _Nsound.__xor__(self, power)

def __ipow__(self, power):
    self = _Nsound.__xor__(self, power)
    return self

def __str__(self):
    return "Nsound.AudioStream(%.1f, %d) holding %.3f seconds (%d samples)" % \
        (self.getSampleRate(), self.getNChannels(), self.getDuration(), self.getLength())

__repr__ = __str__

# Pickle interface

def __getstate__(self):
    '''
    Returns a binary string
    '''
    return _Nsound.AudioStream_write(self)

def __setstate__(self, s):
    '''
    Reads the binary string.
    '''

    self.__init__()
    _Nsound.AudioStream_read(self, s)

#
#------------------------------------------------------------------------------

%}


//------------------------------------------------------------------------------
// Injected Plotter python code

%feature("shadow") Nsound::Plotter::swig_hook()
%{

def show():
    try:
        matplotlib.pylab.show()
    except:

        # If NSOUND_C_PYLAB doesn't exist, then matplotlib wasn't compiled in
        if not 'NSOUND_C_PYLAB' in dir():
            warnings.warn("Nsound wan't compiled with matplotlib")

        pass

show = staticmethod(show)
%}

//-----------------------------------------------------------------------------
// Injected AudioPlaybackRt python code

%feature("shadow") Nsound::AudioPlaybackRt::swig_hook()
%{

#------------------------------------------------------------------------------
# from swig/shadowed.i
#
def __lshift__(self, rhs):
    self.play(rhs)

def __rshift__(self, rhs):
    self.play(rhs)

def __str__(self):
    return "Nsound.AudioPlaybackRt(): %s\n%s" % (self.getInfo(), self.debug_print())

__repr__ = __str__

#
#------------------------------------------------------------------------------

%}

// :mode=python:
