"""Nsound, a C++ library and Python module for audio synthesis featuring
dynamic digital filters.

The classes and functions come from the C++ extension, _Nsound.
"""

import inspect
import os.path

from ._Nsound import *

__version__ = PACKAGE_VERSION
__author__ = NSOUND_AUTHORS
__copyright__ = NSOUND_COPYRIGHT
__license__ = "GNU General Public License 2.0 or later"


def get_caller(offset=0, basename=False):
    """Returns the file name and line number of the caller, offset frames up.
    """
    filename, line_number = inspect.stack()[1 + offset][1:3]

    if basename:
        filename = os.path.basename(filename)

    return filename, line_number


def rel_to_abs(path):
    """Returns path relative to the caller's file as an absolute path."""
    if os.path.isabs(path):
        return path

    caller, _ = get_caller(offset=1)
    prefix = os.path.dirname(os.path.abspath(caller))

    return os.path.abspath(os.path.join(prefix, path))


# The one and only plotter, it imports matplotlib, or prints why it can't and
# leaves plotting off.
_plotter = Plotter()
