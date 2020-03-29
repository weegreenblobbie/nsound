//-----------------------------------------------------------------------------
//
//  $Id: test_audio_stream.cc 912 2015-07-26 00:50:29Z weegreenblobbie $
//
//  Copyright (c) 2005-2006 Nick Hilton
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

#include <Nsound/AudioStream.h>
#include <Nsound/Buffer.h>
#include <Nsound/Sine.h>
#include <Nsound/TicToc.h>

#include "Test.h"

#include <cmath>
#include <stdlib.h>
#include <iostream>

using namespace Nsound;

using std::cerr;
using std::cout;
using std::endl;
using std::flush;

static const char * THIS_FILE = "test_audio_stream.cc";

void testAutioStream();

int main(int argc, char ** argv)
{
    cout << "////////////////////////////////////////////////////////////"
         << endl
         << "// Testing class Nsound::AudioStream"
         << endl
         << "////////////////////////////////////////////////////////////"
         << endl
         << flush;

    testAutioStream();

    return 0;
}

//-----------------------------------------------------------------------------
void testAutioStream()
{
    Tic();
    cout << TEST_HEADER
         << "AudioStream::operator<<, AudioStream::operator[] ... " << flush;

    AudioStream as1(48000, 2);

    uint32 test_size = 29 * 3 + 1;

    float64 test_value = 3.14;

    for(uint32 i = 0; i < test_size; ++i)
    {
        as1 << test_value;

        if(as1[0][i] != test_value || as1[1][i] != test_value)
        {
            cerr << TEST_ERROR_HEADER
                 << "as1[0]["
                 << i
                 << "] should equal "
                 << test_value
                 << ", but it equals "
                 << as1[0][i]
                 << endl;
            exit(1);
        }
    }

    cout << Toc() << " seconds: SUCCESS"
         << endl
         << TEST_HEADER << "testing AudioStream::getLength() ... " << flush;

    Tic();

    if(as1.getLength() != test_size)
    {
        cerr << TEST_ERROR_HEADER
             << "as1.getLength() should equal "
             << test_size
             << ", "
             << as1.getLength()
             << " != "
             << test_size
             << endl;
        exit(1);
    }

    cout << Toc() << " seconds: SUCCESS"
         << endl
         << TEST_HEADER << "testing AudioStream::operator== ... " << flush;

    Tic();

    AudioStream as2(48000, 2);

    if(as1 == as2 || as2 == as1)
    {
        cerr << TEST_ERROR_HEADER
             << "as1 should _NOT_ equal as2"
             << endl;
        exit(1);
    }

    for(uint32 i = 0; i < test_size; i++)
    {
        as2 << test_value;
    }

    if(as1 != as2 || as2 != as1)
    {
        cerr << TEST_ERROR_HEADER
             << "as1 _SHOULD_ equal as2"
             << endl;
        exit(1);
    }

    cout << Toc() << " seconds: SUCCESS"
         << endl
         << TEST_HEADER << "testing AudioStream::AudioStream(&) ... " << flush;

    Tic();

    AudioStream as3(as1);

    if(as1 != as3 || as3 != as1)
    {
        cerr << TEST_ERROR_HEADER
             << "as1 _SHOULD_ equal as3"
             << endl;
        exit(1);
    }

    cout << Toc() << " seconds: SUCCESS"
         << endl
         << TEST_HEADER << "testing AudioStream::operator+,-,*,/ ... " << flush;

    Tic();

    AudioStream as4;

    as4 = as1;

    if(1 + as4 * 2 != 2 * as4 + 1)
    {
        cerr << TEST_ERROR_HEADER
             << "left hand side _SHOULD_ equal right hand side"
             << endl;
        exit(1);
    }

    if(1 - as4 / 2 != -1 * as4 / 2 + 1)
    {
        cerr << TEST_ERROR_HEADER
             << "left hand side _SHOULD_ equal right hand side"
             << endl;
        exit(1);
    }

    if(2.0 / as4 != 2.0 * (1.0 / as4))
    {
        cerr << TEST_ERROR_HEADER
             << "left hand side _SHOULD_ equal right hand side"
             << endl;
        exit(1);
    }

    cout << Toc() << " seconds: SUCCESS"
         << endl
         << TEST_HEADER << "test AudioStream::operator+= ... " << flush;

    Sine sine(48000);

    AudioStream as5(48000, 2);
    as5 << sine.generate(1.0,1.0);

    AudioStream as6 = -1.0 * as5;

    as5 += as6;

    uint32 n_samples = as5.getLength();
    for(uint32 i = 0; i < n_samples; ++i)
    {
        if(as5[0][i] != 0.0 || as5[1][i] != 0.0)
        {
            cerr << TEST_ERROR_HEADER
                 << "as5 + as6 _SHOULD_ be all zeros!"
                 << endl;
            exit(1);
        }
    }

    cout << Toc() << " seconds: SUCCESS"
         << endl;

    // Test AudioStream::reverse()
    cout << TEST_HEADER << "test AudioStream::reverse() ... " << flush;

    as5 = sine.generate(4.0,1.0);

    // as5 now has 4 seconds of sine data.

    Buffer envelope = sine.drawLine(4.0,1.0,0.0);

    as5 *= envelope;

    as6 = as5;

    as5.reverse();

    //as5[0].plot("as as5[0] reversed, upward envelope");

    as6.reverse();

    as5 -= as6;

    uint32 length = as6.getLength();
    for(uint32 i = 0; i < length; ++i)
    {
        if(as5[0][i] != 0.0 || as5[1][i] != 0.0)
        {
            cerr << TEST_ERROR_HEADER
                 << "as6 should be all zeros!"
                 << endl;
            exit(1);
        }
    }

    cout << Toc() << " seconds: SUCCESS"
         << endl
         << TEST_HEADER << "test AudioStream::pan() ... " << flush;

    AudioStream as7(48000, 2);

    as7 << sine.drawLine(1.0,1.0,1.0);

    as7.pan(0.5);

    //as7[0].plot("as7 left");
    //as7[1].plot("as7 right");

    // -1.0------0.5-------0.0+++++++0.5+++++++++++1.0
    //   |-----------------2.0----------------------|
    //                                ^
    //
    // Pan values are from -1.0, 100% for right channel, to 1.0, 100% for the
    // left channel.
    //
    // A pan value of 0.5 is 75% of the total interval towards the left
    // channel leaving only 25% for the right.

    for(uint32 i = 0; i < as7.getLength(); ++i)
    {
        if(as7[0][i] != 0.75 || as7[1][i] != 0.25)
        {
            cerr << TEST_ERROR_HEADER
                 << "AudioStream::pan(float64 d) is borken!"
                 << endl;
            exit(1);
        }
    }

    // Set as7 back to all ones.

    as7[0] = sine.drawLine(1.0,1.0,1.0);
    as7[1] = sine.drawLine(1.0,1.0,1.0);

    // set left channel to 25% and right to 75%
    as7.pan(-0.5);

    for(uint32 i = 0; i < as7.getLength(); ++i)
    {
        if(as7[0][i] != 0.25 || as7[1][i] != 0.75)
        {
            cerr << TEST_ERROR_HEADER
                 << "AudioStream::pan(float64 d) is borken!"
                 << endl;
            exit(1);
        }
    }

    cout << Toc() << " seconds: SUCCESS"
         << endl
         << TEST_HEADER << "test AudioStream::pan(Buffer b) ... " << flush;

    // Test Dynamic pan

    Buffer pan = sine.generate(1.0,1.0);

    // Set as7 back to all ones.

    as7[0] = sine.drawLine(1.0,1.0,1.0);
    as7[1] = sine.drawLine(1.0,1.0,1.0);

    as7.pan(pan);

    //pan.plot("pan");

    //as7[0].plot("as7 left");
    //as7[1].plot("as7 right");

    Buffer left_test = (pan + 1) / 2;

    //left_test.plot("left_test");

    if(as7[0] != left_test)
    {
        cerr << TEST_ERROR_HEADER
             << "AudioStream::pan(Buffer b) is borken!"
             << endl;
        exit(1);
    }


    // There is some roundoff error when Nsound creates right_test.  Instead
    // of using operator != lets subtrarct the two and then check if the
    // difference is large.

    Buffer right_test = left_test * -1.0 + 1.0;

    //right_test.plot("right_test");

    Buffer difference = as7[1] - right_test;

    cerr.precision(20);

    //difference.plot("difference");
    //Plotter::show();

    for(uint32 i = 0; i < difference.getLength(); ++i)
    {
        if(difference[i] > 0.0000001 || difference[i] < -0.0000001)
        {
            cerr << TEST_ERROR_HEADER
                 << "AudioStream::pan(Buffer b) is borken!"
                 << endl;
            exit(1);
        }
    }

    cout << Toc() << " seconds: SUCCESS"
         << endl;


    // testing substream

    cout << TEST_HEADER << "test AudioStream::substream() ... " << flush;

    Tic();

    AudioStream as8(44100, 10);

    for(uint32 i = 0; i < 44100; ++i)
    {
        as8 << static_cast<float64>(i);
    }

    uint32 start_index = 0;
    n_samples = 44100;

    AudioStream substr(as8.substream(start_index, n_samples));

    // Verify sub stream
    for(uint32 ch = 0; ch < 10; ++ch)
    {
        for(uint32 i = 0; i < 44100; ++i)
        {
            if(static_cast<uint32>(substr[ch][i]) != i)
            {
                cerr << TEST_ERROR_HEADER
                     << "as8.substream() error!"
                     << endl;
                exit(1);
            }
        }
    }

    start_index = 1000;
    n_samples = 1000;

    substr = as8.substream(1000, 1000);
    // Verify sub stream
    for(uint32 ch = 0; ch < 10; ++ch)
    {
        for(uint32 i = 1000; i < 1000; ++i)
        {
            if(static_cast<uint32>(substr[ch][i]) != i)
            {
                cerr << TEST_ERROR_HEADER
                     << "as8.substream() error!"
                     << endl;
                exit(1);
            }
        }
    }

    // use the substream(float32,float32) version
    substr = as8.substream(0.0, 1.0);

    // Verify sub stream
    for(uint32 ch = 0; ch < 10; ++ch)
    {
        for(uint32 i = 0; i < 44100; ++i)
        {
            if(static_cast<uint32>(substr[ch][i]) != i)
            {
                cerr << TEST_ERROR_HEADER
                     << "as8.substream() error!"
                     << endl;
                exit(1);
            }
        }
    }

    substr = as8.substream(1000.0 / 44100.0, 1000.0 / 44100.0);
    // Verify sub stream
    for(uint32 ch = 0; ch < 10; ++ch)
    {
        for(uint32 i = 1000; i < 1000; ++i)
        {
            if(static_cast<uint32>(substr[ch][i]) != i)
            {
                cerr << TEST_ERROR_HEADER
                     << "as8.substream() error!"
                     << endl;
                exit(1);
            }
        }
    }

    cout << Toc() << " seconds: SUCCESS" << endl;
}
