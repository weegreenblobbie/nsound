//-----------------------------------------------------------------------------
//
//  $Id: FilterDelay_UnitTest.cc 874 2014-09-08 02:21:29Z weegreenblobbie $
//
//  Copyright (c) 2006 Nick Hilton
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

#include <Nsound/NsoundAll.h>

#include "UnitTest.h"


using std::cerr;
using std::cout;
using std::endl;

using namespace Nsound;


// The __FILE__ macro includes the path, I don't want the whole path.
static const char * THIS_FILE = "FilterMedian_UnitTest.cc";

static const float64 GAMMA = 1.5e-14;

namespace filter_median_unit_test
{

void
check_close_func(
    const uint32 line_,
    const Buffer & gold,
    const Buffer & data)
{
    Buffer diff(gold - data);
    diff.abs();

    if(diff.getMax() > GAMMA)
    {
        cerr << THIS_FILE << "(" << line_ << "): Output did not match gold!\n";

        cerr
            << "GOLD: " << gold << "\n"
            << "DATA: " << data << "\n";

        gold.plot("gold");
        data.plot("data");
        diff.plot("diff");
        Plotter::show();
        exit(1);
    }

    cout << SUCCESS << endl;
}


#define check_close(gold, data) filter_median_unit_test::check_close_func(__LINE__, (gold), (data))


} // namespace



void FilterMedian_UnitTest()
{
    cout << endl << TEST_HEADER << "Median w=3";

    Buffer b1({1, 2, 3, 4, 5, 6, 7});

    FilterMedian fm(3);

    Buffer data = fm.filter(b1);

    // pool     median
    // {1 1 1}  1
    // {1 1 2}  1
    // {1 2 3}  2
    // {2 3 4}  3
    // {3 4 5}  4
    // {4 5 6}  5
    // {5 6 7}  6

    Buffer gold({1.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0});

    check_close(gold, data);

    cout << TEST_HEADER << "Median w=3";

    b1 = {6, 4, 2, 1, 7, 3, 5};

    data = fm.filter(b1);

    // pool     median
    // {6 6 6}  6
    // {4 6 6}  6
    // {2 4 6}  4
    // {1 2 4}  2
    // {1 2 7}  2
    // {1 3 7}  3
    // {3 5 7}  5

    gold = {6.0, 6.0, 4.0, 2.0, 2.0, 3.0, 5.0};

    check_close(gold, data);

    cout << TEST_HEADER << "Median w=5";

    fm = FilterMedian(4);

    b1 = {6, 4, 2, 1, 7, 3, 5, 9, 4, 6, 2};

    data = fm.filter(b1);

    //     pool     median
    // {6 6 6 6 6}  6
    // {4 6 6 6 6}  6
    // {2 4 6 6 6}  6
    // {1 2 4 6 6}  4
    // {1 2 4 6 7}  4
    // {1 2 3 4 7}  3
    // {1 2 3 5 7}  3
    // {1 3 5 7 9}  5
    // {3 4 5 7 9}  5
    // {3 4 5 6 9}  5
    // {2 4 5 6 9}  5

    gold = {6.0, 6.0, 6.0, 4.0, 4.0, 3.0, 3.0, 5.0, 5.0, 5.0, 5.0};

    check_close(gold, data);
}

