********************
Basic Manipulations
********************

AudioStream and Buffer objects have nearly all of their mathematical operators
overloaded.  It is easy to manipulate the data they contain.  Every operation
that can be done on a Buffer can be done on an AudioStream.  In the following
examples, only the Buffer case will be demonstrated, but it can easily be done
to an AudioStream as well.

Concatenation
-------------

After creating an empty Buffer, one can insert samples into it with
concatenation::

    import Nsound as ns
    b = ns.Buffer()
    b << 1 << 2 << 3

    b.getLength()
    # 3
    b.toList()
    # [1.0, 2.0, 3.0]

Buffers can be concatenated together::

    b2 = ns.Buffer()
    b2 << 6 << 7 << 8

    b << b2

    b.getLength()
    # 6
    b.toList()
    # [1.0, 2.0, 3.0, 6.0, 7.0, 8.0]

Scalar Math
-----------

When a scaler is applied to a Buffer or AudioStream, the scalar is applied
element-wise for all the samples contained in the Buffer::

    import Nsound as ns
    b = ns.Buffer()
    b << 1 << 2 << 3 << 6 << 7 << 8

    b.toList()
    # [1.0, 2.0, 3.0, 6.0, 7.0, 8.0]

    b += 1.0            # b = b + 1.0 would also work
    b.toList()
    # [2.0, 3.0, 6.0, 7.0, 8.0, 9.0]

    b *= 2.0
    b.toList()
    # [4.0, 6.0, 12.0, 14.0, 16.0, 18.0]

    b /= 3.0
    b.toList()
    # [1.3333, 2.0, 2.6666, 4.6666, 5.3333, 6.0]

Vector Math
-----------

Nsound also allows element-wise math between two Buffers.  Unlike other
packages such as Numpy or Matlab, the Buffers don't have to be the same length::

    import Nsound as ns

    b1 = ns.Buffer()
    b1 << 1 << 1 << 1

    b2 = ns.Buffer()
    b2 << 0 << 1 << 1 << 1

    # Addition

    b3 = b1 + b2

    b3.toList()
    # [1.0, 2.0, 2.0]

    b3 = b2 + b1
    b3
    # [1.0, 2.0, 2.0, 1.0]

    # Subtraction

    b3 = b1 - b2

    b3.toList()
    # [1.0, 0.0, 0.0]

    b3 = b2 - b1

    b3.toList()
    # [-1.0, 0.0, 0.0, 1.0]

    # Products

    b3 = b1 * b2

    b3.toList()
    # [0.0, 1.0, 1.0]

    b3 = b2 * b1

    b3.toList()
    # [0.0, 1.0, 1.0, 1.0]

    # Quotients

    b3 = b1 / b2

    b3.toList()
    # [1e+20, 1.0, 1.0]

    b3 = b2 / b1

    b3.toList()
    # [0.0, 1.0, 1.0, 1.0]
