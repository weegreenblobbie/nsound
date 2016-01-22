//-----------------------------------------------------------------------------
//
//  $Id$
//
//-----------------------------------------------------------------------------

#include <Nsound/NsoundAll.h>
#include <Nsound/biquad/Design.h>

#include <iostream>

using std::cout;
using std::cerr;
using std::endl;

using namespace Nsound;

using namespace Nsound::biquad;


int
main(void)
{
    float64 sr = 48000;
    uint32  N = 4;

    float64 fc = 5000;
    float64 bw = 2400;

    float64 g0 = 0;
    float64 gfc = 12;
    float64 gbw = gfc - 3;

    hpeq_design(sr, N, fc, bw, g0, gfc, gbw);

    return 0;
}
