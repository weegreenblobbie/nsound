"""

Copyright (c) 2016 to Present Nick Hilton

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

class Test(unittest.TestCase):

    def test_01(self):
        "FilterMedian stuff"

        fm = ns.FilterMedian(3)

        b1 = ns.Buffer([1, 2, 3, 4, 5, 6, 7])

        data = fm.filter(b1)

        gold = [1, 1, 2, 3, 4, 5, 6]

        # convert to int for comparison conviences
        data = [int(x) for x in data]

        self.assertEqual(gold, data)


    def test_02(self):
        "FilterMedian stuff"

        fm = ns.FilterMedian(3)

        b1 = ns.Buffer([6, 4, 2, 1, 7, 3, 5])

        data = fm.filter(b1)

        gold = [6, 6, 4, 2, 2, 3, 5]

        # convert to int for comparison conviences
        data = [int(x) for x in data]

        self.assertEqual(gold, data)


    def test_03(self):
        "FilterMedian stuff"

        fm = ns.FilterMedian(4)

        b1 = ns.Buffer([6, 4, 2, 1, 7, 3, 5, 9, 4, 6, 2])

        data = fm.filter(b1)

        gold = [6, 6, 6, 4, 4, 3, 3, 5, 5, 5, 5]

        # convert to int for comparison conviences
        data = [int(x) for x in data]

        self.assertEqual(gold, data)


