//-----------------------------------------------------------------------------
//
//  $Id: Buffer_UnitTest.cc 874 2014-09-08 02:21:29Z weegreenblobbie $
//
//  Copyright (c) 2005-2006 Nick Hilton
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

#include <Nsound/NsoundAll.h>
#include "UnitTest.h"

#include <stdlib.h>
#include <iostream>

using namespace Nsound;

using std::cout;
using std::cerr;
using std::endl;

static const char * THIS_FILE = "Buffer_UnitTest.cc";

static const float64 GAMMA = 1.5e-12;

void testBufferAdd();

//-----------------------------------------------------------------------------
void Buffer_UnitTest()
{
    cout << endl << THIS_FILE;

    Wavefile::setDefaultSampleSize(48);

    cout << TEST_HEADER << "Testing Buffer::operator<<(float64) ...";

    Buffer b1(29);

    for(float64 i = 0.0; i < 100.0; i += 1.1)
    {
        b1 << i;
    }

    uint32 index = 0;
    for(float64 i = 0.0; i < 100.0; i += 1.1)
    {
        if( ::fabs(b1[index++] - i) > GAMMA)
        {
            cerr << TEST_ERROR_HEADER
                 << "Output did not match expected values!"
                 << endl;

            exit(1);
        }
    }

    cout << SUCCESS;

    cout << TEST_HEADER << "Testing Buffer::Buffer(const Buffer &) ...";

    Buffer b2(b1);

    if(b1 != b2 || b2 != b1)
    {
        cerr << TEST_ERROR_HEADER
             << "Output did not match expected values!"
             << endl;

        Buffer diff(b1 - b2);
        diff.plot("data - gold");
        b1.plot("b1");
        b2.plot("b2");

        Plotter::show();

        exit(1);
    }

    cout << SUCCESS;

    cout << TEST_HEADER << "Testing Buffer::operator=(const Buffer &) ...";

    Buffer b3;

    b3 = b1;

    if(b3 != b1 || b1 != b3)
    {
        cerr << TEST_ERROR_HEADER
             << "Output did not match expected values!"
             << endl;

        Buffer diff(b3 - b1);
        diff.plot("data - gold");
        b3.plot("b3");
        b1.plot("b1");

        Plotter::show();

        exit(1);
    }

    cout << SUCCESS;

    cout << TEST_HEADER << "Testing Buffer::operator<<(const Buffer &) ...";

    Buffer b4(b3);

    b4 << b1;
    b3 << b1;

    if(!(b4 == b3) || !(b3 == b4))
    {
        cerr << TEST_ERROR_HEADER
             << "Output did not match expected values!"
             << endl;

        Buffer diff(b4 - b3);
        diff.plot("data - gold");
        b4.plot("b4");
        b3.plot("b3");

        Plotter::show();

        exit(1);
    }

    cout << SUCCESS;

    cout << TEST_HEADER << "Testing Buffer::operator[](uint32) ...";

    uint32 size = b4.getLength();

    for(uint32 i = 0; i < size; ++i)
    {
        if(::fabs(b4[i] - b3[i]) > GAMMA)
        {
            cerr << TEST_ERROR_HEADER
                 << "Output did not match expected values!"
                 << endl;

            Buffer diff(b4 - b3);
            diff.plot("data - gold");
            b4.plot("b4");
            b3.plot("b3");

            Plotter::show();

            exit(1);
        }
    }

    cout << SUCCESS;

    cout << TEST_HEADER << "Testing Buffer::operator+,-,*,/ ...";

    b4 = b3;

    if(1 + b4 * 2 != 2 * b3 + 1)
    {
        cerr << TEST_ERROR_HEADER
             << "Output did not match expected values!"
             << endl;

        exit(1);
    }

    if(1 - b4 / 2 != -1 * b3 / 2 + 1)
    {
        cerr << TEST_ERROR_HEADER
             << "Output did not match expected values!"
             << endl;

        exit(1);
    }

    if(2.0 / b4 != 4.0 * (0.5 / b4))
    {
        cerr << TEST_ERROR_HEADER
             << "Output did not match expected values!"
             << endl;

        exit(1);
    }

    b3 = -1.0 * b4;

    b4 += b3;

    uint32 n_samples = b4.getLength();
    for(uint32 i = 0; i < n_samples; ++i)
    {
        if(b4[i] != 0.0)
        {
            cerr << TEST_ERROR_HEADER
                 << "b4 += b3 _SHOULD_ be all zeros!"
                 << endl;
            exit(1);
        }
    }

    cout << SUCCESS;

    cout << TEST_HEADER << "Testing Buffer::abs() ...";

    b4 = Buffer();

    if(b4.getLength() != 0)
    {
        cerr << TEST_ERROR_HEADER
             << "b4.getLength() != 0"
             << endl;
        exit(1);
    }

    float64 neg = -1.0;
    float64 sum = 1.0;
    for(uint32 i = 0; i < 100; ++i)
    {
        sum *= neg;
        b4 << sum;
    }

    b4.abs();

    float64 m = b4.getMin();
    if(m < 0.0)
    {
        cerr << TEST_ERROR_HEADER
             << "b4.abs() is broken"
             << endl;
        exit(1);
    }

    cout << SUCCESS;

    cout << TEST_HEADER << "Testing Buffer::reverse() ...";

    b4 = Buffer();

    if(b4.getLength() != 0)
    {
        cerr << TEST_ERROR_HEADER
             << "b4.getLength() != 0"
             << endl;
        exit(1);
    }

    for(float64 f = 0.0; f < 100.0; f += 1.0)
    {
        b4 << f;
    }

    b4.reverse();

    index = 0;
    for(float64 f = 99.0; f >= 0.0; f -= 1.0)
    {
        if(::fabs(b4[index++] - f) > GAMMA)
        {
            cerr << TEST_ERROR_HEADER
                 << "b4.reverse() borken"
                 << endl;

            b4.plot("b4");

            Plotter::show();
            exit(1);
        }
    }

    cout << SUCCESS;

    cout << TEST_HEADER << "Testing Buffer::subbuffer() ...";

    b3 = Buffer();

    uint32 chunk = 100;

    for(uint32 i = 0; i < chunk * 2 + 1; ++i)
    {
        b3 << i;
    }

    uint32 test_size = 99;

    Buffer subbuf = b3.subbuffer(0, test_size);

    if (subbuf.getLength() != test_size)
    {
        cerr << TEST_ERROR_HEADER
             << "subbuf.getLength() = "
             << subbuf.getLength()
             << " != "
             << test_size
             << endl;
        exit(1);
    }

    test_size = chunk;

    subbuf = b3.subbuffer(0,test_size);

    if (subbuf.getLength() != test_size)
    {
        cerr << TEST_ERROR_HEADER
             << "subbuf.getLength() = "
             << subbuf.getLength()
             << " != "
             << test_size
             << endl;
        exit(1);
    }

    test_size = b3.getLength() + 1;

    subbuf = b3.subbuffer(0, test_size);

    if (subbuf.getLength() != b3.getLength())
    {
        cerr << TEST_ERROR_HEADER
             << "subbuf.getLength() = "
             << subbuf.getLength()
             << " != "
             << b3.getLength()
             << endl;
        exit(1);
    }

    test_size = chunk + 10;

    for(uint32 i = 0; i < 10; ++i)
    {
        subbuf = b3.subbuffer(i, test_size);

        if (subbuf.getLength() != test_size)
        {
            cerr << TEST_ERROR_HEADER
                 << "subbuf.getLength() = "
                 << subbuf.getLength()
                 << " != "
                 << test_size
                 << endl;
            exit(1);
        }
    }

    subbuf = b3.subbuffer(0,10);

    for(uint32 i = 0; i < 10; ++i)
    {
        if(static_cast<uint32>(subbuf[i]) != i)
        {
            cerr << TEST_ERROR_HEADER
                 << "b3.subbuffer() error!"
                 << endl;
            exit(1);
        }
    }

    // Try with offset

    subbuf = b3.subbuffer(3,3);

    for(uint32 i = 0; i < 3; ++i)
    {
        if(static_cast<uint32>(subbuf[i]) != i + 3)
        {
            cerr << TEST_ERROR_HEADER
                 << "b3.subbuffer() error!"
                 << endl;
            exit(1);
        }
    }

    // Try using default value for n_samples

    subbuf = b3.subbuffer(3);

    if(subbuf.getLength() != b3.getLength() - 3)
    {
        cerr << TEST_ERROR_HEADER
             << "subbuf.getLength() = "
             << subbuf.getLength()
             << " != "
             << b3.getLength() - 3
             << endl;
        exit(1);
    }

    // Try using default value for n_samples

    subbuf = b3.subbuffer(0);

    if(subbuf.getLength() != b3.getLength())
    {
        cerr << TEST_ERROR_HEADER
             << "subbuf.getLength() _SHOULD_ be 10, found "
             << subbuf.getLength()
             << endl;
        exit(1);
    }

    for(uint32 i = 0; i < 10; ++i)
    {
        if(static_cast<uint32>(subbuf[i]) != i)
        {
            cerr << TEST_ERROR_HEADER
                 << "b3.subbuffer() error!"
                 << endl;
            exit(1);
        }
    }

    subbuf = b3.subbuffer(5,5);

    if(subbuf.getLength() != 5)
    {
        cerr << TEST_ERROR_HEADER
             << "subbuf.getLength() = "
             << subbuf.getLength()
             << " != 5"
             << endl;
        exit(1);
    }

    for(uint32 i = 0; i < 5; ++i)
    {
        if(static_cast<uint32>(subbuf[i]) != i + 5)
        {
            cerr << TEST_ERROR_HEADER
                 << "subbuf["
                 << i
                 << "] = "
                 << subbuf[i]
                 << " != "
                 << i + 5
                 << endl;
            exit(1);
        }
    }

    cout << SUCCESS;

    cout << TEST_HEADER << "Testing Buffer::smooth() ...";

    Sine sine(100);

    b4 << "gold/Filter_noise.wav";

    uint32 b4_length = b4.getLength();
    float64 b4_max = b4.getMax();

    b4.smooth(1, 16);

    if(b4.getLength() != b4_length)
    {
        cerr << TEST_ERROR_HEADER
             << "b4.smooth() error!"
             << endl;
        exit(1);
    }

    if(b4.getMax() > b4_max)
    {
        cerr << TEST_ERROR_HEADER
             << "b4.smooth() error!"
             << endl;
        exit(1);
    }

    cout << SUCCESS;

    cout << TEST_HEADER << "Testing Buffer::pow() ...";

    b4 = 2 * Buffer::ones(5);

    b4 ^= 2.0;

    for(uint32 i = 0; i < 5; ++i)
    {
        if(b4[i] != 4.0)
        {
            cerr << TEST_ERROR_HEADER
                 << "Buffer::pow() error!"
                 << "  4.0 != " << b4[i]
                 << endl;
            exit(1);
        }
    }

    b4 ^= 0.50;

    for(uint32 i = 0; i < 5; ++i)
    {
        if(b4[i] != 2.0)
        {
            cerr << TEST_ERROR_HEADER
                 << "Buffer::pow() error!"
                 << "  2.0 != " << b4[i]
                 << endl;
            exit(1);
        }
    }

    b4 = (b4 ^ 2.0) + (b4 ^ 2.0);

    for(uint32 i = 0; i < 5; ++i)
    {
        if(b4[i] != 8.0)
        {
            cerr << TEST_ERROR_HEADER
                 << "Buffer::pow() error!"
                 << "  8.0 != " << b4[i]
                 << endl;
            exit(1);
        }
    }

    cout << SUCCESS;

    cout << TEST_HEADER << "Testing Buffer operators with different length ...";

    Buffer b5(100);
    b5 = sine.drawLine(1.0, 2.0, 2.0);

    Buffer b6(100);
    b6 = sine.drawLine(2.0, 3.0, 3.0);

    Buffer result = b5 * b6;

    if(result.getLength() != b5.getLength())
    {
        cerr << TEST_ERROR_HEADER
             << "result.getLength() != b5.getLength(), "
             << result.getLength()
             << " != "
             << b5.getLength()
             << endl;
        exit(1);
    }

    for(uint32 i = 0; i < result.getLength(); ++i)
    {
        if(result[i] != 6.0)
        {
            cerr << TEST_ERROR_HEADER
                 << "result["
                 << i
                 << "] != 6.0"
                 << endl;
            exit(1);
        }
    }

    result = b6 * b5;

    if(result.getLength() != b6.getLength())
    {
        cerr << TEST_ERROR_HEADER
             << "result.getLength() != b6.getLength(), "
             << result.getLength()
             << " != "
             << b6.getLength()
             << endl;
        exit(1);
    }

    for(uint32 i = 0; i < b5.getLength(); ++i)
    {
        if(result[i] != 6.0)
        {
            cerr << TEST_ERROR_HEADER
                 << "result["
                 << i
                 << "] != 6.0"
                 << endl;
            exit(1);
        }
    }

    for(uint32 i = b5.getLength(); i < result.getLength(); ++i)
    {
        if(result[i] != 3.0)
        {
            cerr << TEST_ERROR_HEADER
                 << "result["
                 << i
                 << "] != 3.0"
                 << endl;
            exit(1);
        }
    }

    cout << SUCCESS;

    cout << TEST_HEADER << "Testing Buffer::add() ...";

    testBufferAdd();

    cout << SUCCESS;

    cout << TEST_HEADER << "Testing Buffer::circular_iterator ...";

    b5 = sine.drawLine(1.0, 1.0, 100.0);

    Buffer::circular_iterator itor = b5.cbegin();

    for(uint32 i = 0; i < b5.getLength(); ++i)
    {
        ++itor;
    }

    if(itor != b5.cbegin())
    {
        cerr << TEST_ERROR_HEADER
             << "circular_iterator::operator++ is broken!"
             << endl;
        exit(1);
    }

    itor.reset();

    for(uint32 i = 0; i < b5.getLength(); ++i)
    {
        --itor;
    }

    if(itor != b5.cbegin())
    {
        cerr << TEST_ERROR_HEADER
             << "circular_iterator::operator-- is broken!"
             << endl;
        exit(1);
    }

    itor.reset();

    for(uint32 i = 0; i < 3; ++i)
    {
        ++itor;
    }

    if(itor != (b5.cbegin() + 3))
    {
        cerr << TEST_ERROR_HEADER
             << "circular_iterator::operator++ is broken!"
             << endl;
        exit(1);
    }

    itor.reset();

    for(uint32 i = 0; i < 3; ++i)
    {
        --itor;
    }

    if(itor != (b5.cbegin() - 3))
    {
        cerr << TEST_ERROR_HEADER
             << "circular_iterator::operator-- is broken!"
             << endl;
        exit(1);
    }

    cout << SUCCESS;

    cout << TEST_HEADER << "Testing Buffer advanced operators ...";

    Buffer b7 = sine.generate(1.0, 2.0);

    Buffer data = b7;

    data(data > 0.5) = 0.5;
    data(data < -0.5) = -0.5;

//~    // Create gold file
//~    data >> "gold/Buffer_out1.wav";

    // Read in gold
    Buffer gold("gold/Buffer_out1.wav");

    Buffer diff(gold - data);

    if(diff.getAbs().getMax() > GAMMA)
    {
        cerr << TEST_ERROR_HEADER
             << "Output did not match expected values!"
             << endl;

        diff.plot("gold - data");
        data.plot("data");
        gold.plot("gold");

        Plotter::show();

        exit(1);
    }

    data = b7;

    BooleanVector bv1 = data > 0.5;
    BooleanVector bv2 = data < -0.5;

    data(bv1) *= 0.1;
    data(bv2) *= 0.1;

    data(bv1) += 0.45;
    data(bv2) -= 0.45;

//~    // Create gold file
//~    data >> "gold/Buffer_out2.wav";

    // Read in gold
    gold = Buffer("gold/Buffer_out2.wav");

    diff = gold - data;

    if(diff.getAbs().getMax() > GAMMA)
    {
        cerr << TEST_ERROR_HEADER
             << "Output did not match expected values!"
             << endl;

        diff.plot("gold - data");
        data.plot("data");
        gold.plot("gold");

        Plotter::show();

        exit(1);
    }

    cout << SUCCESS << endl;
}


//-----------------------------------------------------------------------------
void reportError(
            const std::string & filename,
            unsigned int line_number,
            unsigned int n_samples,
            unsigned int offset,
            unsigned int a_length,
            unsigned int b_length,
            const std::string & message)
{
    cerr << endl
         << filename.c_str()
         << ":"
         << line_number
         << ": n_samples = "
         << n_samples
         << " : offset = "
         << offset
         << " : a_length = "
         << a_length
         << " : b_length = "
         << b_length
         << " : "
         << message.c_str()
         << endl;
    exit(1);
}

//-----------------------------------------------------------------------------
// Exhaustively test the Buffer::add() method
//
//  There are 11 cases to consider.
//
//  c = a + b
//
//  Case 1: n_samples == 0
//          && a_length < offset
//
//           [aaaaaaa]
//         + |--offset--|[bbbbbbbbbbbbbbbbbbbbbb]
//  result = |aaaaaaaa---bbbbbbbbbbbbbbbbbbbbbbb|
//
//  Case 2: n_samples != 0
//          && a_length < offset
//          && n_samples <= b_length
//
//           [aaaaaaa]
//         + |--offset--|[bbbbbbbbbbbbbbbbbbbbbb]
//                       |---n_samples---|
//  result = |aaaaaaaa---bbbbbbbbbbbbbbbb|
//
//  Case 3: n_samples != 0
//          && a_length < offset
//          && n_samples > b_length
//
//           [aaaaaaa]
//         + |--offset--|[bbbbbbbbbbbbbbbbbbbbbb]
//                       |---n_samples----------------|
//  result = |aaaaaaaa---bbbbbbbbbbbbbbbbbbbbbbbb-----|
//
//
//  Case 4: n_samples == 0
//          && a_length >= offset
//          && a_length < offset + b_length
//
//           [aaaaaaaaaaaaaaaaaaaaaaaaaaaa]
//         + |--offset--|[bbbbbbbbbbbbbbbbbbbbbb]
//  result = |aaaaaaaaaaaccccccccccccccccccbbbbb|
//
//
//  Case 5: n_samples != 0
//          && a_length >= offset
//          && n_samples <= b_length
//          && a_length <= offset + n_samples
//
//           [aaaaaaaaaaaaaaaaaaaa]
//         + |--offset--|[bbbbbbbbbbbbbbbbbbbbbb]
//                      |----n_samples----|
//  result = |aaaaaaaaaacccccccccccbbbbbbb|
//
//
//  Case 6: n_samples != 0
//          && a_length >= offset
//          && n_samples < b_length
//          && a_length > offset + n_samples
//
//           [aaaaaaaaaaaaaaaaaaaaaaaaaaaa]
//         + |--offset--|[bbbbbbbbbbbbbbbbbbbbbb]
//                      |--n_samples--|
//  result = |aaaaaaaaaacccccccccccccccaaa|
//
//  Case 7: n_samples != 0
//          && a_length >= offset
//          && n_samples > b_length
//          && a_length <= offset + b_length
//
//           [aaaaaaaaaaaaaaaaaaaa]
//         + |--offset--|[bbbbbbbbbbbbbbbbbbbbbb]
//                      |----n_samples---------------|
//  result = |aaaaaaaaaaaccccccccccbbbbbbbbbbbbbb----|
//
//
//  Case 8: n_samples == 0
//          && a_length >= offset + b_length
//
//           [aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa]
//         + |--offset--|[bbbbbbbbbbbbbbbbbbbbbb]
//  result = |aaaaaaaaaaaccccccccccccccccccccccccaaaaaa|
//
//  Case 9:  n_samples != 0
//           && a_length >= offset + n_samples
//           && n_samples < b_length
//
//           [aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa]
//         + |--offset--|[bbbbbbbbbbbbbbbbbbbb]
//                      |----n_samples----|
//  result = |aaaaaaaaaaaccccccccccccccccccaaaaaaaaaaaa|
//
//  Case 10: n_samples != 0
//           && a_length >= offset + n_samples
//           && n_samples >= b_length
//
//           [aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa]
//         + |--offset--|[bbbbbbbbbbbbbbbbbbbbb]
//                      |----n_samples---------------|
//  result = |aaaaaaaaaaacccccccccccccccccccccccaaaaaaa|
//
//  Case 11: n_samples != 0
//           && a_length > offset + b_length
//           && a_length < offset + n_samples
//
//           [aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa]
//         + |--offset--|[bbbbbbbbbbbbbbbbbbbbb]
//                      |----n_samples----------------------|
//  result = |aaaaaaaaaaacccccccccccccccccccccccaaaaaaaa----|
//
//-----------------------------------------------------------------------------
void testBufferAdd()
{
    bool final_case[12];

    for(uint32 i = 0; i < 12; ++i)
    {
        final_case[i] = false;
    }

    for(uint32 run = 0; run < 2; ++run)
    {
        Buffer b;
        Buffer a;

        uint32 test_size = 15;

        if(run == 0)
        {
            for(uint32 i = 0; i < test_size; ++i)
            {
                b << 1.0;
            }

            a = b;

            // Make buffer a longer.
            for(uint32 i = 0; i < test_size * 5 + 7; ++i)
            {
                a << 1.0;
            }
        }
        else
        {
            for(uint32 i = 0; i < test_size; ++i)
            {
                a << 1.0;
            }

            b = a;

            // Make buffer a longer.
            for(uint32 i = 0; i < test_size * 5 + 7; ++i)
            {
                b << 1.0;
            }
        }

        uint32 a_length = a.getLength();
        uint32 b_length = b.getLength();

        // Brute force test the add() method.

        uint32 offset_end = a_length + test_size * 6 + 7;

        for(uint32 n_samples = 0; n_samples < offset_end; ++n_samples)
        {
            for(uint32 offset = 0; offset < offset_end; ++offset)
            {
                bool current_run[12];

                for(uint32 i = 0; i < 12; ++i)
                {
                    current_run[i] = false;
                }

                Buffer result(a);
                result.add(b,offset,n_samples);

                uint32 result_length = result.getLength();

                ///////////////////////////////////////////////////////////////
                // Case 1:
                //      result = [aaaaaaa]
                //             + |--offset--|[bbbbbbbbbbbbbbbbbbbbbb]
                //             = |----------------------------------|
                ///////////////////////////////////////////////////////////////
                if(n_samples == 0 && a_length < offset)
                {
                    current_run[0] = true;

                    // Verify result length
                    if(result_length != offset + b_length)
                    {
                        reportError(__FILE__,__LINE__,
                                        n_samples, offset, a_length, b_length,
                                        "result length error");
                    }

                    // Check prefix values.
                    for(uint32 i = 0; i < a_length; ++i)
                    {
                        if(result[i] != 1.0)
                        {
                            reportError(__FILE__,__LINE__,
                                        n_samples, offset, a_length, b_length,
                                        "result prefix error");
                        }
                    }

                    // Check gap values
                    for(uint32 i = a_length + 1; i < offset; ++i)
                    {
                        if(result[i] != 0.0)
                        {
                            reportError(__FILE__,__LINE__,
                                        n_samples, offset, a_length, b_length,
                                        "result gap error");
                        }
                    }

                    // Check b values
                    for(uint32 i = offset + 1; i < result_length; ++i)
                    {
                        if(result[i] != 1.0)
                        {
                            reportError(__FILE__,__LINE__,
                                        n_samples, offset, a_length, b_length,
                                        "result b value error");
                        }
                    }
                }

                ///////////////////////////////////////////////////////////////
                //  Case 2: n_samples != 0
                //          && a_length < offset
                //          && n_samples <= b_length
                //
                //  result = [aaaaaaa]
                //         + |--offset--|[bbbbbbbbbbbbbbbbbbbbbb]
                //                       |---n_samples---|
                //         = |---------------------------|
                ///////////////////////////////////////////////////////////////
                else if(n_samples != 0
                            && a_length < offset
                            && n_samples <= b_length)
                {
                    current_run[1] = true;
                    // Verify result length
                    if(result_length != offset + n_samples)
                    {
                        reportError(__FILE__,__LINE__,
                                        n_samples, offset, a_length, b_length,
                                        "result length error");
                    }

                    // Check prefix values.
                    for(uint32 i = 0; i < a_length; ++i)
                    {
                        if(result[i] != 1.0)
                        {
                            reportError(__FILE__,__LINE__,
                                        n_samples, offset, a_length, b_length,
                                        "result prefix error");
                        }
                    }

                    // Check gap values
                    for(uint32 i = a_length + 1; i < offset; ++i)
                    {
                        if(result[i] != 0.0)
                        {
                            reportError(__FILE__,__LINE__,
                                        n_samples, offset, a_length, b_length,
                                        "result gap error");
                        }
                    }

                    // Check b values
                    for(uint32 i = offset + 1; i < result_length; ++i)
                    {
                        if(result[i] != 1.0)
                        {
                            reportError(__FILE__,__LINE__,
                                        n_samples, offset, a_length, b_length,
                                        "result b value error");
                        }
                    }
                }

                ///////////////////////////////////////////////////////////////
                //  Case 3: n_samples != 0
                //          && a_length < offset
                //          && n_samples > b_length
                //
                //  result = [aaaaaaa]
                //         + |--offset--|[bbbbbbbbbbbbbbbbbbbbbb]
                //                       |---n_samples----------------|
                //         = |----------------------------------------|
                ///////////////////////////////////////////////////////////////
                else if(n_samples != 0
                            && a_length < offset
                            && n_samples > b_length)
                {
                    current_run[2] = true;

                    // Verify result length
                    if(result_length != offset + n_samples)
                    {
                        reportError(__FILE__,__LINE__,
                                        n_samples, offset, a_length, b_length,
                                        "result length error");
                    }

                    // Check prefix values.
                    for(uint32 i = 0; i < a_length; ++i)
                    {
                        if(result[i] != 1.0)
                        {
                            reportError(__FILE__,__LINE__,
                                        n_samples, offset, a_length, b_length,
                                        "result prefix error");
                        }
                    }

                    // Check gap values
                    for(uint32 i = a_length + 1; i < offset; ++i)
                    {
                        if(result[i] != 0.0)
                        {
                            reportError(__FILE__,__LINE__,
                                        n_samples, offset, a_length, b_length,
                                        "result gap error");
                        }
                    }

                    // Check b values
                    for(uint32 i = offset + 1; i < offset + b_length; ++i)
                    {
                        if(result[i] != 1.0)
                        {
                            reportError(__FILE__,__LINE__,
                                        n_samples, offset, a_length, b_length,
                                        "result b value error");
                        }
                    }

                    // Check tail values
                    for(uint32 i = offset + b_length + 1; i < result_length; ++i)
                    {
                        if(result[i] != 0.0)
                        {
                            reportError(__FILE__,__LINE__,
                                        n_samples, offset, a_length, b_length,
                                        "result tail error");
                        }
                    }
                }

                ///////////////////////////////////////////////////////////////
                //  Case 4: n_samples == 0
                //          && a_length >= offset
                //          && a_length < offset + b_length
                //
                //  result = [aaaaaaaaaaaaaaaaaaaaaaaaaaaa]
                //         + |--offset--|[bbbbbbbbbbbbbbbbbbbbbb]
                //         = |----------------------------------|
                ///////////////////////////////////////////////////////////////
                else if(n_samples == 0
                            && a_length >= offset
                            && a_length < offset + b_length)
                {
                    current_run[3] = true;

                    // Verify result length
                    if(result_length != offset + b_length)
                    {
                        reportError(__FILE__,__LINE__,
                                        n_samples, offset, a_length, b_length,
                                        "result length error");
                    }

                    // Check prefix values.
                    for(uint32 i = 0; i < offset; ++i)
                    {
                        if(result[i] != 1.0)
                        {
                            reportError(__FILE__,__LINE__,
                                        n_samples, offset, a_length, b_length,
                                        "result prefix error");
                        }
                    }

                    // Check overlap values
                    for(uint32 i = offset + 1; i < a_length; ++i)
                    {
                        if(result[i] != 2.0)
                        {
                            reportError(__FILE__,__LINE__,
                                        n_samples, offset, a_length, b_length,
                                        "result overlap error");
                        }
                    }

                    // Check tail values
                    for(uint32 i = a_length + 1; i < result_length; ++i)
                    {
                        if(result[i] != 1.0)
                        {
                            reportError(__FILE__,__LINE__,
                                        n_samples, offset, a_length, b_length,
                                        "result tail error");
                        }
                    }
                }

                ///////////////////////////////////////////////////////////////
                //  Case 5: n_samples != 0
                //          && a_length >= offset
                //          && n_samples <= b_length
                //          && a_length <= offset + n_samples
                //
                //  result = [aaaaaaaaaaaaaaaaaaaa]
                //         + |--offset--|[bbbbbbbbbbbbbbbbbbbbbb]
                //                      |----n_samples----|
                //         = |----------------------------|
                ///////////////////////////////////////////////////////////////
                else if(n_samples != 0
                            && a_length >= offset
                            && a_length <= offset + n_samples
                            && n_samples <= b_length)
                {
                    current_run[4] = true;

                    // Verify result length
                    if(result_length != offset + n_samples)
                    {
                        reportError(__FILE__,__LINE__,
                                        n_samples, offset, a_length, b_length,
                                        "result length error");
                    }

                    // Check prefix values.
                    for(uint32 i = 0; i < offset; ++i)
                    {
                        if(result[i] != 1.0)
                        {
                            reportError(__FILE__,__LINE__,
                                        n_samples, offset, a_length, b_length,
                                        "result prefix error");
                        }
                    }

                    // Check overlap values
                    for(uint32 i = offset + 1; i < a_length; ++i)
                    {
                        if(result[i] != 2.0)
                        {
                            reportError(__FILE__,__LINE__,
                                        n_samples, offset, a_length, b_length,
                                        "result overlap error");
                        }
                    }

                    // Check tail values
                    for(uint32 i = a_length + 1; i < result_length; ++i)
                    {
                        if(result[i] != 1.0)
                        {
                            reportError(__FILE__,__LINE__,
                                        n_samples, offset, a_length, b_length,
                                        "result tail error");
                        }
                    }
                }
                ///////////////////////////////////////////////////////////////
                //  Case 6: n_samples != 0
                //          && a_length >= offset
                //          && n_samples < b_length
                //          && a_length > offset + n_samples
                //
                //  result = [aaaaaaaaaaaaaaaaaaaaaaaaaaaa]
                //         + |--offset--|[bbbbbbbbbbbbbbbbbbbbbb]
                //                      |--n_samples--|
                //         = |----------------------------|
                ///////////////////////////////////////////////////////////////
                else if(n_samples != 0
                            && a_length >= offset
                            && a_length > offset + n_samples
                            && n_samples < b_length)
                {
                    current_run[5] = true;

                    // Verify result length
                    if(result_length != a_length)
                    {
                        reportError(__FILE__,__LINE__,
                                        n_samples, offset, a_length, b_length,
                                        "result length error");
                    }

                    // Check prefix values.
                    for(uint32 i = 0; i < offset; ++i)
                    {
                        if(result[i] != 1.0)
                        {
                            reportError(__FILE__,__LINE__,
                                        n_samples, offset, a_length, b_length,
                                        "result prefix error");
                        }
                    }

                    // Check overlap values
                    for(uint32 i = offset + 1; i < offset + n_samples; ++i)
                    {
                        if(result[i] != 2.0)
                        {
                            reportError(__FILE__,__LINE__,
                                        n_samples, offset, a_length, b_length,
                                        "result overlap error");
                        }
                    }

                    // Check tail values
                    for(uint32 i = a_length + 1; i < result_length; ++i)
                    {
                        if(result[i] != 1.0)
                        {
                            reportError(__FILE__,__LINE__,
                                        n_samples, offset, a_length, b_length,
                                        "result tail error");
                        }
                    }
                }
                ///////////////////////////////////////////////////////////////
                //  Case 7: n_samples != 0
                //          && a_length >= offset
                //          && n_samples > b_length
                //          && a_length <= offset + b_length
                //
                //  result = [aaaaaaaaaaaaaaaaaaaa]
                //         + |--offset--|[bbbbbbbbbbbbbbbbbbbbbb]
                //                      |----n_samples---------------|
                //         = |---------------------------------------|
                ///////////////////////////////////////////////////////////////
                else if(n_samples != 0
                            && a_length >= offset
                            && a_length <= offset + b_length
                            && n_samples > b_length)
                {
                    current_run[6] = true;

                    // Verify result length
                    if(result_length != offset + n_samples)
                    {
                        reportError(__FILE__,__LINE__,
                                        n_samples, offset, a_length, b_length,
                                        "result length error");
                    }

                    // Check prefix values.
                    for(uint32 i = 0; i < offset; ++i)
                    {
                        if(result[i] != 1.0)
                        {
                            reportError(__FILE__,__LINE__,
                                        n_samples, offset, a_length, b_length,
                                        "result prefix error");
                        }
                    }

                    // Check overlap values
                    for(uint32 i = offset + 1; i < a_length; ++i)
                    {
                        if(result[i] != 2.0)
                        {
                            reportError(__FILE__,__LINE__,
                                        n_samples, offset, a_length, b_length,
                                        "result overlap error");
                        }
                    }

                    // Check b values
                    for(uint32 i = a_length + 1; i < offset + b_length; ++i)
                    {
                        if(result[i] != 1.0)
                        {
                            reportError(__FILE__,__LINE__,
                                        n_samples, offset, a_length, b_length,
                                        "result b value error");
                        }
                    }

                    // Check tail values
                    for(uint32 i = offset + b_length + 1; i < result_length; ++i)
                    {
                        if(result[i] != 0.0)
                        {
                            reportError(__FILE__,__LINE__,
                                        n_samples, offset, a_length, b_length,
                                        "result tail error");
                        }
                    }
                }
                ///////////////////////////////////////////////////////////////
                //  Case 8: n_samples == 0
                //          && a_length >= offset + b_length
                //
                //  result = [aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa]
                //         + |--offset--|[bbbbbbbbbbbbbbbbbbbbbb]
                //         = |-----------------------------------------|
                ///////////////////////////////////////////////////////////////
                else if(n_samples == 0
                            && a_length >= offset + b_length)
                {
                    current_run[7] = true;

                    // Verify result length
                    if(result_length != a_length)
                    {
                        reportError(__FILE__,__LINE__,
                                        n_samples, offset, a_length, b_length,
                                        "result length error");
                    }

                    // Check prefix values.
                    for(uint32 i = 0; i < offset; ++i)
                    {
                        if(result[i] != 1.0)
                        {
                            reportError(__FILE__,__LINE__,
                                        n_samples, offset, a_length, b_length,
                                        "result prefix error");
                        }
                    }

                    // Check overlap values
                    for(uint32 i = offset + 1; i < offset + b_length; ++i)
                    {
                        if(result[i] != 2.0)
                        {
                            reportError(__FILE__,__LINE__,
                                        n_samples, offset, a_length, b_length,
                                        "result overlap error");
                        }
                    }

                    // Check a values
                    for(uint32 i = offset + b_length + 1; i < result_length; ++i)
                    {
                        if(result[i] != 1.0)
                        {
                            reportError(__FILE__,__LINE__,
                                        n_samples, offset, a_length, b_length,
                                        "result a value error");
                        }
                    }
                }
                ///////////////////////////////////////////////////////////////
                //  Case 9:  n_samples != 0
                //           && a_length >= offset + n_samples
                //           && n_samples < b_length
                //
                //  result = [aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa]
                //         + |--offset--|[bbbbbbbbbbbbbbbbbbbb]
                //                      |----n_samples----|
                //         = |-----------------------------------------|
                ///////////////////////////////////////////////////////////////
                else if(n_samples != 0
                            && a_length >= offset + n_samples
                            && n_samples < b_length)
                {
                    current_run[8] = true;

                    // Verify result length
                    if(result_length != a_length)
                    {
                        reportError(__FILE__,__LINE__,
                                        n_samples, offset, a_length, b_length,
                                        "result length error");
                    }

                    // Check prefix values.
                    for(uint32 i = 0; i < offset; ++i)
                    {
                        if(result[i] != 1.0)
                        {
                            reportError(__FILE__,__LINE__,
                                        n_samples, offset, a_length, b_length,
                                        "result prefix error");
                        }
                    }

                    // Check overlap values
                    for(uint32 i = offset + 1; i < offset + n_samples; ++i)
                    {
                        if(result[i] != 2.0)
                        {
                            reportError(__FILE__,__LINE__,
                                        n_samples, offset, a_length, b_length,
                                        "result overlap error");
                        }
                    }

                    // Check a values
                    for(uint32 i = offset + n_samples + 1; i < result_length; ++i)
                    {
                        if(result[i] != 1.0)
                        {
                            reportError(__FILE__,__LINE__,
                                        n_samples, offset, a_length, b_length,
                                        "result a value error");
                        }
                    }
                }
                ///////////////////////////////////////////////////////////////
                //  Case 10: n_samples != 0
                //           && a_length >= offset + n_samples
                //           && n_samples >= b_length
                //
                //  result = [aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa]
                //         + |--offset--|[bbbbbbbbbbbbbbbbbbbbb]
                //                      |----n_samples------------|
                //         = |-----------------------------------------|
                ///////////////////////////////////////////////////////////////
                else if(n_samples != 0
                            && a_length >= offset + n_samples
                            && n_samples >= b_length)
                {
                    current_run[9] = true;

                    // Verify result length
                    if(result_length != a_length)
                    {
                        reportError(__FILE__,__LINE__,
                                        n_samples, offset, a_length, b_length,
                                        "result length error");
                    }

                    // Check prefix values.
                    for(uint32 i = 0; i < offset; ++i)
                    {
                        if(result[i] != 1.0)
                        {
                            reportError(__FILE__,__LINE__,
                                        n_samples, offset, a_length, b_length,
                                        "result prefix error");
                        }
                    }

                    // Check overlap values
                    for(uint32 i = offset + 1; i < offset + b_length; ++i)
                    {
                        if(result[i] != 2.0)
                        {
                            reportError(__FILE__,__LINE__,
                                        n_samples, offset, a_length, b_length,
                                        "result overlap error");
                        }
                    }

                    // Check a values
                    for(uint32 i = offset + b_length + 1; i < result_length; ++i)
                    {
                        if(result[i] != 1.0)
                        {
                            a.plot("a");
                            result.plot("result");
                            Plotter::show();
                            reportError(__FILE__,__LINE__,
                                        n_samples, offset, a_length, b_length,
                                        "result a value error");
                        }
                    }
                }
                ///////////////////////////////////////////////////////////////
                //  Case 11: n_samples != 0
                //           && a_length > offset + b_length
                //           && a_length < offset + n_samples
                //
                //  result = [aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa]
                //         + |--offset--|[bbbbbbbbbbbbbbbbbbbbb]
                //                      |----n_samples----------------------|
                //         = |----------------------------------------------|
                ///////////////////////////////////////////////////////////////
                else if(n_samples != 0
                            && a_length > offset + b_length
                            && a_length < offset + n_samples)
                {
                    current_run[10] = true;

                    // Verify result length
                    if(result_length != offset + n_samples)
                    {
                        reportError(__FILE__,__LINE__,
                                        n_samples, offset, a_length, b_length,
                                        "result length error");
                    }

                    // Check prefix values.
                    for(uint32 i = 0; i < offset; ++i)
                    {
                        if(result[i] != 1.0)
                        {
                            reportError(__FILE__,__LINE__,
                                        n_samples, offset, a_length, b_length,
                                        "result prefix error");
                        }
                    }

                    // Check overlap values
                    for(uint32 i = offset + 1; i < offset + b_length; ++i)
                    {
                        if(result[i] != 2.0)
                        {
                            reportError(__FILE__,__LINE__,
                                        n_samples, offset, a_length, b_length,
                                        "result overlap error");
                        }
                    }

                    // Check a values
                    for(uint32 i = offset + b_length + 1; i < a_length; ++i)
                    {
                        if(result[i] != 1.0)
                        {
                            reportError(__FILE__,__LINE__,
                                        n_samples, offset, a_length, b_length,
                                        "result a value error");
                        }
                    }

                    // Check tail values
                    for(uint32 i = a_length + 1; i < result_length; ++i)
                    {
                        if(result[i] != 0.0)
                        {
                            reportError(__FILE__,__LINE__,
                                        n_samples, offset, a_length, b_length,
                                        "result tail error");
                        }
                    }
                }
                else
                {
                    reportError(
                            __FILE__,
                            __LINE__,
                            n_samples,
                            offset,
                            a_length,
                            b_length,
                            "unhandled case");
                    exit(1);
                }

                for(uint32 i = 0; i < 11; ++i)
                {
                    if(current_run[i])
                        final_case[i] = true;
                }
            }
        }
    }

    cout << "cases not hit: ";

    for(uint32 i = 0; i < 11; ++i)
    {
        if(!final_case[i])
            cout << i + 1 << " ";
    }

    cout << ": ";

}
