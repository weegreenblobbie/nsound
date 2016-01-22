//-----------------------------------------------------------------------------
//
//  $Id: test_moving_average.cc 874 2014-09-08 02:21:29Z weegreenblobbie $
//
//-----------------------------------------------------------------------------

// Nsound headers
#include <Nsound/NsoundAll.h>

using namespace Nsound;

//-----------------------------------------------------------------------------
int
main(void)
{
    float64 sr = 48000.0;

    Sine sine(sr);

    // Draw a pulse.
    Buffer input = sine.silence(0.25)
                << sine.drawLine(0.25, 1.0, 1.0)
                << sine.silence(0.25);

    input += sine.whiteNoise(0.75) * 0.25;

    input.plot("noisy input");

    FilterMovingAverage fma(101);

    Buffer output = fma.filter(input);

    output.plot("filtered output");

    Plotter::show();

    return 0;
}

