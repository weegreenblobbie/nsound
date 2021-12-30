*****************
Buffer Creation
*****************

A Buffer holds audio samples at discrete sample periods.  The Buffer object
knows nothing about time, endianness or bit precision.  It is a generic
container of floating point data.

There are 3 general ways to create an Nsound Buffer:

1) Creating an empty Buffer
2) Call ones(), rand() or zeros()
3) Reading a wavefile from disk

Creating An Empty Buffer
========================

Call the constructor::

    import Nsound as ns
    b = ns.Buffer()

The new Buffer `b` is empty.  Calling the getLength() method will return 0.

The underlying data structure that is held by the Buffer class is a
std::vector. One can preallocate memory when creating a buffer by specify the
number of samples to preallocate::

    b = ns.Buffer(1024)

The new Buffer `b` is empty, even though memory was preallocated.  Calling the
getLength() method will return 0.

In general, you don't need to worry about preallocating memory.  It is meant
to be useful when implementing new features in Nsound when the size of
Buffers are already known.

Call Ones, Rand or Zeros
=========================

The Buffer class includes some convience functions for creating Buffers that
are filled with oness, random numbers or zeros:

.. py:function:: Buffer.ones(n_samples)
.. py:function:: Buffer.rand(n_samples)
.. py:function:: Buffer.zeros(n_samples)

Example usage::

    import Nsound as ns
    b1 = ns.Buffer.ones(10)
    b2 = ns.Buffer.rand(10)
    b3 = ns.Buffer.zeros(10)

In the example above, 10 samples were stored in the created Buffers.

Reading A Wavefile From Disk
============================

A Buffer can be created from a wavefile::

    b = ns.Buffer("california.wav")

The new Buffer `b` will contain all the samples in "california.wav".  If the
wavefile has more than one channel, `only the first channel` is read and stored
in the new Buffer.

The wavefile's data will be converted into float64 with a range of
(-1.0, 1.0.).

Converting Between Numpy Arrays And Buffers
===========================================

Suppose you have a numpy array::

    import numpy as np

    a = np.array([5,4,3,2,1])

You can now convert it into a Buffer like so::

    b = ns.Buffer(a)

    print(b)

    Nsound.Buffer([5, 4, 3, 2, 1, ])

You can also convert back using this syntax::

    a = np.array(b)

    print(a)

    [ 5.  4.  3.  2.  1.]