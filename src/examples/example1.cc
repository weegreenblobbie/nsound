//-----------------------------------------------------------------------------
//
//  $Id$
//
//-----------------------------------------------------------------------------

#include <Nsound/NsoundAll.h>
#include <Nsound/biquad/Design.h>
#include <Nsound/biquad/Biquad.hpp>

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

    Biquad bq(sr, fc, bw, gfc, gbw, g0, N);

    cout
        << "-----------------------------------------------------------\n"
        << bq.to_json() << "\n"
        << "-----------------------------------------------------------\n";

    std::string json_1 = "{ \"b\" : [1,2,3,4], \"a\" : [10,20,30,40] }";

    Biquad bq2 = Biquad::from_json(json_1);

    cout
        << bq2.to_json() << "\n";

    return 0;
}
