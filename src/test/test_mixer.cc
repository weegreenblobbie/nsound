//-----------------------------------------------------------------------------
//
//  $Id: test_mixer.cc 874 2014-09-08 02:21:29Z weegreenblobbie $
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

#include <Nsound/AudioStream.h>
#include <Nsound/Buffer.h>
#include <Nsound/Mixer.h>
#include <Nsound/Nsound.h>
#include <Nsound/Sine.h>
#include <Nsound/TicToc.h>
#include <Nsound/Wavefile.h>

#include "Test.h"

#include <stdlib.h>
#include <iostream>

using namespace Nsound;

using std::cout;
using std::cerr;
using std::endl;
using std::flush;

static const char * THIS_FILE = "test_mixer.cc";

#define THIS_TEST_HEADER \


void testRange(
    const Buffer & buffer,
    uint32 start_index,
    uint32 n_samples,
    float64 value,
    unsigned int line)
{
    for(uint32 i = start_index; i < start_index + n_samples; ++i)
    {
        if(buffer[i] != value)
        {
            cerr << endl
                 << TEST_HEADER2(line)
                 << "sample should be "
                 << value
                 << " found "
                 << buffer[i]
                 << ", FAILURE"
                 << endl
                 << flush;
            exit(1);
        }
    }
}


//-----------------------------------------------------------------------------
int
main(int argc, char ** argv)
{
    cout << "////////////////////////////////////////////////////////////"
         << endl
         << "// Testing class Nsound::Mixer"
         << endl
         << "////////////////////////////////////////////////////////////"
         << endl
         << flush;

    Tic();
    cout << TEST_HEADER
         << "Mixer::add(), Mixer::getStream() ... " << flush;

    uint32 SAMPLES_PER_SECOND = 10;

    Sine sine(SAMPLES_PER_SECOND);

    AudioStream as1(SAMPLES_PER_SECOND, 1);

    // Draws 1 second (1000 samples) of samples at 1.0.
    as1 << sine.drawLine(1.0,1.0,1.0);

    Mixer mixer;

    // Insert stream at time 0, 30 beats per minute.
    mixer.add(0.0, 30.0, as1);

    // Grab 4.0 seconds of the resulting stream.

    AudioStream result;

    result = mixer.getStream(4.0);

    if(result.getLength() != 4.0 * (1.0 * SAMPLES_PER_SECOND))
    {
        cerr << endl
                 << TEST_HEADER2(__LINE__)
                 << "result has incorrect length! FAILURE"
                 << endl
                 << flush;
        exit(1);
    }

    // Should produce
    //
    // 10 samples at 1.0
    // 10 samples at 0.0
    // 10 samples at 1.0
    // 10 samples at 0.0

    uint32 samples = 10;

    testRange(result[0],  0, 10, 1.0, __LINE__);
    testRange(result[0], 10, 10, 0.0, __LINE__);
    testRange(result[0], 20, 10, 1.0, __LINE__);
    testRange(result[0], 30, 10, 0.0, __LINE__);

    cout << Toc() << " seconds: SUCCESS"
         << endl
         << TEST_HEADER
         << "Mixer::add(), Mixer::getStream() ... " << flush;

    // Add as1 again.
    mixer.add(0.0, 30.0, as1);

    result = mixer.getStream(4.0);

    samples = 10;
    // This should produce:
    //
    // 10 samples at 2.0
    // 10 samples at 0.0
    // 10 samples at 2.0
    // 10 samples at 0.0

    testRange(result[0],  0, 10, 2.0, __LINE__);
    testRange(result[0], 10, 10, 0.0, __LINE__);
    testRange(result[0], 20, 10, 2.0, __LINE__);
    testRange(result[0], 30, 10, 0.0, __LINE__);

    cout << Toc() << " seconds: SUCCESS"
         << endl
         << TEST_HEADER
         << "Mixer::add(), Mixer::getStream() with offset ... " << flush;

    result = mixer.getStream(0.5, 4.5);

    if(result.getLength() != 4.0 * (1.0 * SAMPLES_PER_SECOND))
    {
        cerr << endl
                 << TEST_HEADER2(__LINE__)
                 << "result has incorrect length! FAILURE"
                 << endl
                 << flush;
        exit(1);
    }

    samples = 5;

    // This should produce:
    //
    // 5  samples at 2.0
    // 10 samples at 0.0
    // 10 samples at 2.0
    // 10 samples at 0.0
    // 5  samples at 2.0

    testRange(result[0],  0,  5, 2.0, __LINE__);
    testRange(result[0],  5, 10, 0.0, __LINE__);
    testRange(result[0], 15, 10, 2.0, __LINE__);
    testRange(result[0], 25, 10, 0.0, __LINE__);
    testRange(result[0], 35,  5, 2.0, __LINE__);

    cout << Toc() << " seconds: SUCCESS"
         << endl
         << TEST_HEADER
         << "Mixer::add(), Mixer::clear() ... " << flush;

    mixer.clear();

    if(mixer.size() != 0)
    {
        cerr << TEST_ERROR_HEADER
             << "mixer.size() should be 0.0, found "
             << mixer.size()
             << endl;
        exit(1);
    }

    cout << Toc() << " seconds: SUCCESS"
         << endl
         << TEST_HEADER
         << "Mixer::add(), Mixer::add() with different BPM ... " << flush;

    // Insert the AudioStream at different bpm.

    mixer.add(0.0, 30.0, as1);
    mixer.add(0.5, 30.0, as1);

    result = mixer.getStream(4.0);

    // result[0].plot("result:234");

    samples = 5;

    // This should produce:
    //
    // 5 samples at 1.0
    // 5 samples at 2.0
    // 5 samples at 1.0
    // 5 samples at 0.0
    // 5 samples at 1.0
    // 5 samples at 2.0
    // 5 samples at 1.0
    // 5 samples at 0.0

    testRange(result[0],  0, 5, 1.0, __LINE__);
    testRange(result[0],  5, 5, 2.0, __LINE__);
    testRange(result[0], 10, 5, 1.0, __LINE__);
    testRange(result[0], 15, 5, 0.0, __LINE__);
    testRange(result[0], 20, 5, 1.0, __LINE__);
    testRange(result[0], 25, 5, 2.0, __LINE__);
    testRange(result[0], 30, 5, 1.0, __LINE__);
    testRange(result[0], 35, 5, 0.0, __LINE__);

    cout << Toc() << " seconds: SUCCESS"
         << endl
         << TEST_HEADER
         << "Mixer::add(), Mixer::getStream() with offset ... " << flush;

    result = mixer.getStream(0.5, 4.5);

    // result[0].plot("result:266");

    if(result.getLength() != 4.0 * (1.0 * SAMPLES_PER_SECOND))
    {
        cerr << endl
                 << TEST_HEADER2(__LINE__)
                 << "result has incorrect length! FAILURE"
                 << endl
                 << flush;
        exit(1);
    }

    samples = 5;

    // This should produce:
    //
    // 5 samples at 2.0
    // 5 samples at 1.0
    // 5 samples at 0.0
    // 5 samples at 1.0
    // 5 samples at 2.0
    // 5 samples at 1.0
    // 5 samples at 0.0
    // 5 samples at 1.0

    testRange(result[0],  0, 5, 2.0, __LINE__);
    testRange(result[0],  5, 5, 1.0, __LINE__);
    testRange(result[0], 10, 5, 0.0, __LINE__);
    testRange(result[0], 15, 5, 1.0, __LINE__);
    testRange(result[0], 20, 5, 2.0, __LINE__);
    testRange(result[0], 25, 5, 1.0, __LINE__);
    testRange(result[0], 30, 5, 0.0, __LINE__);
    testRange(result[0], 35, 5, 1.0, __LINE__);

    cout << Toc() << " seconds: SUCCESS"
         << endl
         << TEST_HEADER
         << "Mixer::add(), Mixer::getStream() with zero bpm ... " << flush;

    AudioStream as2(SAMPLES_PER_SECOND, 1);

    as2 << sine.drawLine(0.5,3.0,3.0);

    mixer.add(1.5, 0.0, as2);

    result = mixer.getStream(0.5, 4.5);

    // result[0].plot("result: 314");

    // Plotter::show();

    samples = 5;

    // This should produce:
    //
    // 5 samples at 2.0
    // 5 samples at 1.0
    // 5 samples at 3.0
    // 5 samples at 1.0
    // 5 samples at 2.0
    // 5 samples at 1.0
    // 5 samples at 0.0
    // 5 samples at 1.0

    testRange(result[0],  0, 5, 2.0, __LINE__);
    testRange(result[0],  5, 5, 1.0, __LINE__);
    testRange(result[0], 10, 5, 3.0, __LINE__);
    testRange(result[0], 15, 5, 1.0, __LINE__);
    testRange(result[0], 20, 5, 2.0, __LINE__);
    testRange(result[0], 25, 5, 1.0, __LINE__);
    testRange(result[0], 30, 5, 0.0, __LINE__);
    testRange(result[0], 35, 5, 1.0, __LINE__);

    cout << Toc() << " seconds: SUCCESS"
         << endl;

    return 0;
}

