"""

$Id: test_DelayLine.py 904 2015-06-15 03:26:19Z weegreenblobbie $

Copyright (c) 2009 to Present Nick Hilton

weegreenblobbie2_gmail_com (replace '_' with '@' and '.')

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Library General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

"""

import unittest

import Nsound as ns
fn = ns.rel_to_abs

class Test(unittest.TestCase):

    def test_01(self):
        "DelayLine stuff"

        dl = ns.DelayLine(1, 5)

        gold = [0.0] * 5
        gold.extend([0.0, 1.0, 2.0, 3.0, 4.0])
        gold = [int(x) for x in gold]

        data = []

        for i in range(10):

            x = dl.read()

            data.append(int(x))

            dl.write(i)

        self.assertEqual(gold, data)
