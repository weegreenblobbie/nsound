##############################################################################
#
# $Id: CudaTool.py 618 2011-02-09 05:10:37Z weegreenblobbie $
#
# Nsound is a C++ library and Python module for audio synthesis featuring
# dynamic digital filters. Nsound lets you easily shape waveforms and write
# to disk or plot them. Nsound aims to be as powerful as Csound but easy to
# use.
#
# Copyright (c) 2011-Present Nick Hilton
#
# Based on the CudaTool on the SCons wiki.
#
##############################################################################

"""
SCons.Tool.CudaTool

CUDA Tool for SCons

"""

import os
import platform
import sys

import SCons.Tool
import SCons.Scanner.C
import SCons.Defaults

CUDAScanner = SCons.Scanner.C.CScanner()

def add_common_nvcc_variables(env):
    """
    Add underlying common "NVIDIA CUDA compiler" variables that
    are used by multiple builders.
    """

    # "NVCC common command line"
    if not env.has_key('_NVCCCOMCOM'):
        # nvcc needs '-I' prepended before each include path, regardless of platform
        env['_NVCCWRAPCPPPATH'] = '$_CPPINCFLAGS'
        # prepend -Xcompiler before each flag
        env['_NVCCWRAPCFLAGS'] =     '${_concat("-Xcompiler ", CFLAGS,     "", __env__)}'
        env['_NVCCWRAPSHCFLAGS'] =   '${_concat("-Xcompiler ", SHCFLAGS,   "", __env__)}'
        env['_NVCCWRAPCCFLAGS'] =   '${_concat("-Xcompiler ", CCFLAGS,   "", __env__)}'
        env['_NVCCWRAPSHCCFLAGS'] = '${_concat("-Xcompiler ", SHCCFLAGS, "", __env__)}'
        # assemble the common command line
        env['_NVCCCOMCOM'] = '${_concat("-Xcompiler ", CXXFLAGS, "", __env__)} $_CPPDEFFLAGS $_NVCCWRAPCPPPATH'


def generate(env):

    cudaToolkitPath = env['CUDA_TOOLKIT_PATH']

    CUDAScanner = SCons.Scanner.C.CScanner()

    static_obj, shared_obj = SCons.Tool.createObjBuilders(env)


    # Add this suffix to the list of things buildable by Object
    static_obj.add_action(".cu", "$NVCCCOM")
    shared_obj.add_action(".cu", "$SHNVCCCOM")
    static_obj.add_emitter(".cu", SCons.Defaults.StaticObjectEmitter)
    shared_obj.add_emitter(".cu", SCons.Defaults.SharedObjectEmitter)

    # Add this suffix to the list of things scannable
    SCons.Tool.SourceFileScanner.add_scanner(".cu", CUDAScanner)

    add_common_nvcc_variables(env)

    # default compiler
    env['NVCC']   = 'nvcc'
    env['SHNVCC'] = 'nvcc'

    # set the include path, and pass both c compiler flags and c++ compiler flags
    env['NVCCFLAGS'] = SCons.Util.CLVar('')
    env['SHNVCCFLAGS'] = SCons.Util.CLVar('') + ' -shared'

    # 'NVCC Command'
    env['NVCCCOM']   = '$NVCC -o $TARGET -c $NVCCFLAGS $_NVCCWRAPCFLAGS $NVCCWRAPCCFLAGS $_NVCCCOMCOM $SOURCES'
    env['SHNVCCCOM'] = '$SHNVCC -o $TARGET -c $SHNVCCFLAGS $_NVCCWRAPSHCFLAGS $_NVCCWRAPSHCCFLAGS $_NVCCCOMCOM $SOURCES'

    # the suffix of CUDA source files is '.cu'
    env['CUDAFILESUFFIX'] = '.cu'

    # Add nvcc to PATH
    env.PrependENVPath('PATH', cudaToolkitPath + '/bin')

    env.Detect('nvcc')

    # Include path
    env.AppendUnique(CPPPATH=[cudaToolkitPath + "/include"])

    # Library path
    bits = platform.architecture()[0]
    if "64" in bits:
        libdir = cudaToolkitPath + "/lib64"
    else:
        libdir = cudaToolkitPath + "/lib"

    env.AppendUnique(LIBPATH=[libdir])

    env.AppendUnique(LINKFLAGS = ["-Wl,-rpath=%s" % libdir])

    env.AppendUnique(LIBS=["cuda", "cudart"])

def exists(env):
    return env.Detect('nvcc')

