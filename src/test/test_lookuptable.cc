#include <Nsound/NsoundAll.h>

#include <cassert>
#include <cmath>
#include <iostream>

using namespace Nsound;

using std::cout;
using std::cerr;
using std::endl;

static const float64 GAMMA = 1.5e-12;

uint32 check(const float64 data, const float64 expected, const std::string & message)
{
    if (std::fabs(expected - data) > 1.5e-12)
    {
        cout << "Case failed:  " << message << endl
             << "    Expected: " << expected << endl
             << "      Actual: " << data << endl;
        return 1;
    }
    return 0;
}
int
main(void)
{
    auto table1 = LookupTable();

    table1.add_point(0.0, 1.0);
    table1.add_point(2.0, 1.0);
    table1.add_point(4.0, 2.0);
    table1.add_point(8.0, 0.0);
    table1.add_point(-0.0001, 0.0);

    struct Test
    {
        float64 in;
        float64 out;
        std::string message;
    };

    auto test_cases = std::vector<Test> {
        // input    output    message
        {-1.0,      0.0,      "x=-1.0"},
        {-0.1,      0.0,      "x=-0.1"},
        { 0.0000000,1.0,      "x=0.0"},
        { 0.0000001,1.0,      "x=0.0"},
        { 0.1,      1.0,      "x=0.1"},
        { 1.999,    1.0,      "x=1.999"},
        { 3.0,      1.5,      "x=3.0"},
        { 4.5,      1.75,     "x=4.5"},
        { 5.0,      1.5,      "x=5.0"},
        { 5.5,      1.25,     "x=5.5"},
        { 6.0,      1.0,      "x=6.0"},
        { 7.0,      0.5,      "x=7.0"},
        { 7.9999999,5e-8,     "x=7.9999999"},
        { 8.0,      0.0,      "x=8.0"},
        { 8.0000001,0.0,      "x=8.0000001"},
        { 9.0,      0.0,      "x=9.0"},
    };

    auto num_errors = 0u;

    for (const auto & tc : test_cases)
    {
        num_errors += check(table1.interpolate(tc.in), tc.out, tc.message);
    }

    cout << "num_errors = " << num_errors << endl;

    assert( num_errors == 0 );

    return 0;
}

