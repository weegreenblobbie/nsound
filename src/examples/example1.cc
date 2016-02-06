#include <Nsound/NsoundAll.h>
#include <Nsound/biquad/FilterBank.hpp>

#include <iostream>

using std::cout;

using namespace Nsound;
using namespace Nsound::biquad;

int
main(void)
{
    //-------------------------------------------------------------------------
    // filterbank

    float64 sr = 48000;   // sample rate

    auto khz = 1000.0;

    Biquad bq0(sr, 0*khz, 1*khz,  9,  6, 0, 4);
    Biquad bq1(sr, 4*khz, 2*khz, 12,  9, 0, 4);
    Biquad bq2(sr, 9*khz, 2*khz, -6, -3, 0, 4);
    Biquad bq3(sr,  sr/2, 8*khz,  6,  3, 0, 4);

//~    FilterBank fb(sr);

//~    fb.add(bq0);
//~    fb.add(bq1);
//~    fb.add(bq2);
//~    fb.add(bq3);

//~    cout
//~        << "-----------------------------------------------------------\n"
//~        << "FilterBank: fb.to_json()\n"
//~        << "-----------------------------------------------------------\n"
//~        << fb.to_json() << "\n";

    std::string src_json = R"xxx(
        {
          "filters": [
            {
              "band_width_hz": 1000,
              "freq_center_hz": 0,
              "gain_db_at_band_width": 6,
              "gain_db_at_fc": 9,
              "gain_db_baseline": 0,
              "order": 4,
              "samplerate": 48000
            },
            {
              "band_width_hz": 2000,
              "freq_center_hz": 4000,
              "gain_db_at_band_width": 9,
              "gain_db_at_fc": 12,
              "gain_db_baseline": 0,
              "order": 4,
              "samplerate": 48000
            },
            {
              "band_width_hz": 2000,
              "freq_center_hz": 9000,
              "gain_db_at_band_width": -3,
              "gain_db_at_fc": -6,
              "gain_db_baseline": 0,
              "order": 4,
              "samplerate": 48000
            },
            {
              "band_width_hz": 8000,
              "freq_center_hz": 24000,
              "gain_db_at_band_width": 3,
              "gain_db_at_fc": 6,
              "gain_db_baseline": 0,
              "order": 4,
              "samplerate": 48000
            }
          ],
          "samplerate": 48000
        }
    )xxx";

    FilterBank fb = FilterBank::from_json(src_json);

    fb.plot();

    Plotter pylab;
    pylab.ylim(-7, 13);
    pylab.title("N=4, Butterworth");

    Plotter::show();

    return 0;
}
