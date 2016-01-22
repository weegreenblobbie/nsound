#!/usr/bin/env python

"""

$Id: setup.py 894 2015-06-07 03:24:33Z weegreenblobbie $

A wrapper around setup_builder.py to provide the standard Python distutils
interface that everyone is familiar with.

This script invokes the scons build system to generate a SWIG interface file
and generate setup_build.py which does the actual distutils work of building
the Python module.
"""

import os
import sys
import subprocess
import shlex

import distutils.spawn

import os.path

#------------------------------------------------------------------------------
# Globals & constats

NSOUND_DIR = os.path.dirname(os.path.abspath(__file__))
SETUP_BUILDER_PY = "setup_builder.py"
NSOUND_H = os.path.join(NSOUND_DIR, "src", "Nsound", "Nsound.h")

def main():

    tool_check()

    configure()

    process_nsound_h()

    # execute actual distutils setup() script

    import setup_builder


def is_exe(fpath):
    '''
    Return True if the shell can execute fpath.
    '''
    return os.path.isfile(fpath) and os.access(fpath, os.X_OK)


def which(program):
    '''
    Returns the full path to program if found, None otherwise.

    Unlike other solutions, this finds .bat and .exe files also.

    Reference: http://stackoverflow.com/questions/377017/test-if-executable-exists-in-python
    '''

    extentions = ['', '.bat', '.exe']

    for ext in extentions:

        tool = program + ext

        for path in os.environ['PATH'].split(os.pathsep):

            exe = os.path.join(path, tool)

            if is_exe(exe):
                return exe

    return None


def tool_check():

    stdout = sys.stdout

    tools = ['scons', 'swig']

    for t in tools:
        stdout.write("Checking for %s ..." % t)

        tool = which(t)

        if tool:
            print " %s" % tool

        else:
            raise RuntimeError(
                "FAILURE!\nCould not find tool '%s',"
                " is it installed?  Is it in your PATH?" % t)


def configure():

    stdout = sys.stdout

    #--------------------------------------------------------------------------
    # Configure the Nsound python module by generating setup_builder.py

    stdout.write("Configuring Nsound ...")
    stdout.flush()

    os.chdir(NSOUND_DIR)

    scons = which('scons')

    cmd = [scons, SETUP_BUILDER_PY]

    p = subprocess.Popen(
        cmd,
        env = os.environ,
        stdout = subprocess.PIPE,
        stderr = subprocess.PIPE)

    out, err = p.communicate()

    if p.returncode:
        raise RuntimeError(
            "FAILURE!\ncmd = %s\n%s\n%s" % (repr(cmd), out, err))

    print " done"


def process_nsound_h():
    '''
    Read in Nsound.h and display configuration summay.
    '''

    d = {
        'ao' : 'no',
        'c++11' : 'no',
        'cuda' : 'no',
        'float64' : 'no',
        'little-endian' : 'no',
        'matplotlib' : 'no',
        'openmp' : 'no',
        'portaudio' : 'no',
        'real-time-audio' : 'no',
    }

    release = None

    with open(NSOUND_H, "r") as fd:
        text = fd.readlines()

    for l in text:

        if '#define NSOUND_LIBAO 1' in l:
            d['ao'] = 'yes'

        elif '#define NSOUND_CPP11 1' in l:
            d['c++11'] = 'yes'

        elif '#define NSOUND_CUDA 1' in l:
            d['cuda'] = 'yes'

        elif '#define NSOUND_64_BIT 1' in l:
            d['float64'] = 'yes'

        elif '#define NSOUND_LITTLE_ENDIAN' in l:
            d['little-endian'] = 'yes'

        elif '#define NSOUND_C_PYLAB 1' in l:
            d['matplotlib'] = 'yes'

        elif '#define NSOUND_OPENMP 1' in l:
            d['openmp'] = 'yes'

        elif '#define NSOUND_LIBPORTAUDIO 1' in l:
            d['portaudio'] = 'yes'

        elif '#define PACKAGE_RELEASE' in l:
            release = l.strip().split()[-1]
            release = release.replace('"', '')

    if d['c++11'] == 'yes' and d['portaudio'] == 'yes':
        d['real-time-audio'] = 'yes'

    #--------------------------------------------------------------------------
    # Display summary

    fmt = "    %-16s: %s"

    print fmt  % ('release', release)

    keys = d.keys()
    keys.sort()

    for k in keys:
        print fmt % (k, d[k])


if __name__ == "__main__": main()




# :mode=python: