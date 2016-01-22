//-----------------------------------------------------------------------------
//
//  $Id: AudioStream.i 912 2015-07-26 00:50:29Z weegreenblobbie $
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


%rename(__lshift__)    Nsound::AudioStream::operator<<;
%rename(_get_at_index) Nsound::AudioStream::operator[];


%extend Nsound::AudioStream
{
    // Instead of renaming this function, we create an alias so both names
    // will work.
    std::size_t __len__() const
    {
        return $self->getLength();
    }
}


// Avoid deleting references
%typemap(out) Nsound::AudioStream &
{
    // %typemap(out) Nsound::AudioStream &

    $result = SWIG_NewPointerObj(
        SWIG_as_voidptr(result), SWIGTYPE_p_Nsound__AudioStream, 0);
}


%feature("shadow") Nsound::AudioStream::_swig_shadow()
%{

#------------------------------------------------------------------------------
# from swig/AudioStream.i
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
    if (
            isinstance(rhs, str) or
            isinstance(rhs, AudioStream) or
            isinstance(rhs, Buffer)
    ):
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

        return _Nsound.AudioStream__get_at_index(self, i)

    elif isinstance(i, slice):

        indices = range(*i.indices())

        n_channels = len(indices)

        a = AudioStream(self.getSampleRate(), n_channels)

        k = 0
        for index in indices:
            a[k] = Buffer(_Nsound.AudioStream__get_at_index(self, index))
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

        self._set_at_index(i, buf)

    else:
        raise TypeError("Expecting index type int, but got %s" %(type(i)))


def __pow__(self, power):
    return _Nsound.__xor__(self, power)


def __ipow__(self, power):
    self = _Nsound.__xor__(self, power)
    return self


def __str__(self):

    s = "Nsound.AudioStream(\n"

    for b in self:
        s += "    %s,\n" % b

    s += ")"

    return s


def __repr__(self):
    return "Nsound.AudioStream(%.1f, %d) holding %.3f seconds (%d samples)" % (
        self.getSampleRate(),
        self.getNChannels(),
        self.getDuration(),
        self.getLength()
    )


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

// :mode=python:
