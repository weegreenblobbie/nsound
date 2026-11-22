"""Tests plotting from the Python module, checking what matplotlib drew.

Run with matplotlib's Agg backend, MPLBACKEND=Agg, it opens no windows.
"""

import os
import tempfile
import unittest

import matplotlib.pyplot as plt

import Nsound as ns


class TestPlotter(unittest.TestCase):
    """Plotter and plot() draw with matplotlib in the running Python."""

    def tearDown(self):
        """Closes the figures a test drew."""
        plt.close("all")

    def test_buffer_plot(self):
        """Buffer.plot() draws its samples with the title, in a new figure."""
        ns.Buffer([1.0, 2.0, 4.0]).plot("the title")

        self.assertEqual(len(plt.get_fignums()), 1)

        axes = plt.gca()

        self.assertEqual(axes.get_title(), "the title")
        self.assertEqual(list(axes.get_lines()[0].get_ydata()), [1.0, 2.0, 4.0])

    def test_plotter_plot(self):
        """Plotter.plot() passes the data, format and kwargs to pyplot."""
        pylab = ns.Plotter()
        pylab.figure()
        pylab.plot(ns.Buffer([0.5, 1.5]), ns.Buffer([3.0, 5.0]), "r--", "linewidth=2.5")

        line = plt.gca().get_lines()[0]

        self.assertEqual(list(line.get_xdata()), [0.5, 1.5])
        self.assertEqual(list(line.get_ydata()), [3.0, 5.0])
        self.assertEqual(line.get_linestyle(), "--")
        self.assertEqual(line.get_linewidth(), 2.5)

    def test_savefig(self):
        """Plotter.savefig() writes the figure, with its kwargs."""
        pylab = ns.Plotter()
        pylab.figure("figsize=(4, 3)")
        pylab.plot(ns.Buffer([1.0, 2.0]))

        with tempfile.TemporaryDirectory() as folder:
            filename = os.path.join(folder, "plot.png")

            pylab.savefig(filename, "dpi=50")

            with open(filename, "rb") as fd:
                header = fd.read(24)

        self.assertEqual(header[1:4], b"PNG")
        self.assertEqual(int.from_bytes(header[16:20], "big"), 200)
        self.assertEqual(int.from_bytes(header[20:24], "big"), 150)

    def test_python_errors_raise(self):
        """Errors in the kwargs are raised as Python exceptions."""
        with self.assertRaises(SyntaxError):
            ns.Plotter().plot(ns.Buffer([1.0]), "", "linewidth=")
