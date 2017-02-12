
%rename(_get_at_index) SwigBufferProxy::operator[];


%extend SwigBufferProxy
{
    std::size_t __len__() const
    {
        return $self->_buf.getLength();
    }
}


%typemap(out) Nsound::float64 & SwigBufferProxy::operator[]
{
    $result = PyFloat_FromDouble(*$1);
}


%typemap(out) SwigBufferProxy & operator<<
{
    // SwigBufferProxy.i
    // $symname

#~    $result = SWIG_NewPointerObj(
#~        SWIG_as_voidptr(result), SWIGTYPE_p_Nsound__Buffer, 0);

    if(result) {} // disable unsed warning

    Py_INCREF($self);
    $result = $self;
}


%rename(__add__) SwigBufferProxy::operator+;
%rename(__sub__) SwigBufferProxy::operator-;
%rename(__mul__) SwigBufferProxy::operator*;
%rename(__div__) SwigBufferProxy::operator/;

%rename(__radd__) SwigBufferProxy::_radd;
%rename(__rsub__) SwigBufferProxy::_rsub;
%rename(__rmul__) SwigBufferProxy::_rmul;
%rename(__rdiv__) SwigBufferProxy::_rdiv;



%feature("shadow") SwigBufferProxy::_swig_shadow()
%{

# SwigBufferProxy.i


def toList(self):
    return [x for x in self]


def __repr__(self):
    return "Nsound.SwigBufferProxy holding %d samples" % self.getLength()


def __str__(self):
    return self._buf.__str__()


def __lshift__(self, rhs):

    # Read wavefile
    if (
            isinstance(rhs, str) or
            isinstance(rhs, Buffer)
    ):
        return _Nsound.SwigBufferProxy___lshift__(self, rhs)

    else:

        # try to convert to a float
        try:
            return _Nsound.SwigBufferProxy___lshift__(self, float(rhs))

        except TypeError:
            pass

        # try to convert to Buffer

        try:
            return _Nsound.SwigBufferProxy___lshift__(self, Buffer(rhs))

        except TypeError:
            pass

        # assume the object is iteratble
        try:
           for x in rhs:
               self = _Nsound.SwigBufferProxy___lshift__(self, float(x))
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

        return _Nsound.SwigBufferProxy__get_at_index(self, i)

    elif isinstance(i, slice):

        b = Buffer()

        for index in xrange(*i.indices(_Nsound.Buffer_getLength(self))):
            b << _Nsound.SwigBufferProxy__get_at_index(self, index)

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

        _Nsound.SwigBufferProxy__set_at_index(self, i, d)

    else:
        raise TypeError("Expecting index type int, but got %s" %(type(i)))

%}

# :mode=python: