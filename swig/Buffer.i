//-----------------------------------------------------------------------------
//
//  $Id: Buffer.i 912 2015-07-26 00:50:29Z weegreenblobbie $
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


%rename(__lshift__) Nsound::Buffer::operator<<;
%rename(_get_at_index) Nsound::Buffer::operator[];

%extend Nsound::Buffer
{
    // Instead of renaming this function, we create an alias so both names
    // will work.
    std::size_t __len__() const
    {
        return $self->getLength();
    }

    std::string __str__() const
    {
        std::size_t N = 6;

        std::stringstream ss;
        ss << "Nsound.Buffer([";

        std::size_t size = $self->getLength();

        if(size <= 2 * N)
        {
            for(std::size_t i = 0; i < size; ++i)
            {
                ss << $self->operator[](i) << ", ";
            }
        }
        else
        {
            for(std::size_t i = 0; i < N; ++i)
            {
                ss << $self->operator[](i) << ", ";
            }

            ss << "... ";

            for(std::size_t i = size - N; i < size; ++i)
            {
                ss << $self->operator[](i) << ", ";
            }
        }

        ss << "])";

        return ss.str();
    }
}


%typemap(out) Nsound::float64 & Nsound::Buffer::operator[]
{
    $result = PyFloat_FromDouble(*$1);
}


// Avoid deleting references
%typemap(out) Nsound::Buffer &
{
    // %typemap(out) Nsound::Buffer &

    $result = SWIG_NewPointerObj(
        SWIG_as_voidptr(result), SWIGTYPE_p_Nsound__Buffer, 0);
}


%feature("shadow") Nsound::Buffer::_swig_shadow()
%{

#------------------------------------------------------------------------------
# from swig/Buffer.i
#

def __add__(self, rhs):
    temp = Buffer(self)
    temp += rhs
    return temp


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
    raise Exception("Operator not supported")


def __irshift__(self, rhs):
    raise Exception("Operator not supported")


def __lshift__(self, rhs):

    # Read wavefile
    if (
            isinstance(rhs, str) or
            isinstance(rhs, Buffer)
    ):
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

        if i < 0:
            i += self.getLength()

        return _Nsound.Buffer__get_at_index(self, i)

    elif isinstance(i, slice):

        b = Buffer()

        for index in xrange(*i.indices(_Nsound.Buffer_getLength(self))):
            b << _Nsound.Buffer__get_at_index(self, index)

        return b

    else:
        raise StopIteration


def __setitem__(self,i, d):

    if isinstance(i, int):

        if i >= self.getLength():
            raise IndexError("Error: Index out of bounds, %d >= %d" %(
                i, self.getLength()))

        elif i < -self.getLength():
            raise IndexError("Error: Index out of bounds, %d < -%d" %(
                i, self.getLength()))

        if type(d) != types.IntType and type(d) != types.FloatType:
            raise TypeError("Error: Argument must be numeric")

        _Nsound.Buffer__set_at_index(self, i, d)

    else:
        raise TypeError("Expecting index type int, but got %s" %(type(i)))


def __pow__(self, power):
    return _Nsound.__xor__(self, power)


def __ipow__(self, power):
    self = _Nsound.__xor__(self, power)
    return self


def __repr__(self):
    return "Nsound.Buffer holding %d samples" % self.getLength()


def toList(self):
    return [x for x in self]


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

// :mode=python:
