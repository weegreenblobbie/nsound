"""
$Id: _nsound_utils.py 760 2013-09-17 03:17:53Z weegreenblobbie $

Nsound is a C++ library and Python module for audio synthesis featuring
dynamic digital filters. Nsound lets you easily shape waveforms and write
to disk or plot them. Nsound aims to be as powerful as Csound but easy to
use.

Copyright (c) 2004 to Present Nick Hilton

weegreenblobbie_at_yahoo_com

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

import os
import shlex
import subprocess

def dbg_print(env, filename, message, basename = True):
    """
    Prints debug messages if enabled.
    """

    if not env['NS_CONFIG_DEBUG']: return

    if basename:
        filename = os.path.basename(filename)

    print "%s: %s" % (filename, message)


def run_command(command, env):
    """
    Runs the command in a subprocess and returns stdout, stderr, exit_code.
    """

    cmd = shlex.split(command)

    p = subprocess.Popen(
        cmd,
        env = env,
        stdout = subprocess.PIPE,
        stderr = subprocess.PIPE)

    stdout, stderr = p.communicate()

    return stdout.strip(), stderr.strip(), p.returncode

