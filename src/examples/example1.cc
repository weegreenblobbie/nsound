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

//~    hpeq_design(sr, N, fc, bw, g0, gfc, gbw);

    Biquad bq(sr, fc, bw, gfc, gbw, g0, N);

    cout
        << "-----------------------------------------------------------\n"
        << bq.to_json() << "\n"
        << "-----------------------------------------------------------\n";

//~    Biquad bq2(bq.kernel());

//~    std::string json_2 = bq2.to_json();

//~    cout << "bq2.to_json(): " << bq2.to_json() << "\n";

//~    Biquad bq3 = Biquad::from_json(json_2);

//~    cout << "bq3.to_json(): " << bq3.to_json() << "\n";

//~    bq3.plot(sr);

    bq.plot();

    Plotter::show();

    return 0;
}
