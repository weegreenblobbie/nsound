//-----------------------------------------------------------------------------
//
//  $Id$
//
//-----------------------------------------------------------------------------

#include <Nsound/NsoundAll.h>
#include <Nsound/biquad/Biquad.hpp>
#include <Nsound/biquad/FilterBank.hpp>

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

    //-------------------------------------------------------------------------
    // individual biquad

    Biquad bq(sr, fc, bw, gfc, gbw, g0, N);

    cout
        << "-----------------------------------------------------------\n"
        << "Biquad: bq.to_json()\n"
        << "-----------------------------------------------------------\n"
        << bq.to_json() << "\n";

    //-------------------------------------------------------------------------
    // filterbank

    FilterBank fb(sr);

    Biquad bq_lo_cut(sr, 0, 1000, -6, -3, 0, 2);

    fb.add(bq_lo_cut);

    Biquad bq_boost(sr, sr/4, 4000, 2, 1, 0, 2);

    fb.add(bq_boost);

    Biquad bq_hi_cut(sr, sr/2, 1000, -6, -3, 0, 2);

    fb.add(bq_hi_cut);

    cout
        << "-----------------------------------------------------------\n"
        << "FilterBank: fb.to_json()\n"
        << "-----------------------------------------------------------\n"
        << fb.to_json() << "\n";

    //-------------------------------------------------------------------------
    // plots

    bq.plot();

    fb.plot();

    Plotter::show();

    return 0;
}
