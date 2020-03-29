///////////////////////////////////////////////////////////////////////////////
//
//  $Id: Nsound.i 912 2015-07-26 00:50:29Z weegreenblobbie $
//
//  Copyright (c) 2009 to Present Nick Hilton
//
//  weegreenblobbie_yahoo_com (replace '_' with '@' and '.')
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
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
///////////////////////////////////////////////////////////////////////////////

%module Nsound

%{
#include "Nsound/NsoundAll.h"
%}

%feature("autodoc", "1");

%include <exception.i>
%include <std_string.i>
%include <std_vector.i>

using std::string;
using std::vector;

//-----------------------------------------------------------------------------
// Exceptions
%exception
{
    try {
        $action
    }
    catch (Nsound::Exception e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return NULL;
    }
}

%include "ignored.i"

%include "src/Nsound/Nsound.h"

%include "Buffer.i"
%include "AudioStream.i"
%include "AudioPlaybackRt.i"
%include "Plotter.i"

typedef vector<Nsound::FFTChunk> FFTChunkVectorT;

namespace std
{
    %template(FloatVector)   vector<Nsound::float64>;
    %template(BooleanVector) vector<Nsound::boolean>;
    %template(Uint32Vector) vector<Nsound::uint32>;
    %template(BooleanVectorVector) vector<vector<Nsound::boolean> >;
    %template(FFTChunkVectorT) vector<Nsound::FFTChunk>;
    %template(StringVector) vector<std::string>;
}

// Keep these above the others
%include "src/Nsound/CircularIterators.h"
%include "src/Nsound/Instrument.h"
%include "src/Nsound/BufferSelection.h"
%include "src/Nsound/Buffer.h"
%include "src/Nsound/AudioStreamSelection.h"
%include "src/Nsound/AudioStream.h"
%include "src/Nsound/FFTChunk.h"
%include "src/Nsound/WindowType.h"

// The rest.
%include "src/Nsound/AudioBackendType.h"
%include "src/Nsound/AudioBackend.h"
%include "src/Nsound/AudioPlayback.h"
%include "src/Nsound/AudioPlaybackRt.h"
%include "src/Nsound/BufferWindowSearch.h"
%include "src/Nsound/CircularBuffer.h"
%include "src/Nsound/Clarinet.h"
%include "src/Nsound/DelayLine.h"
%include "src/Nsound/DrumBD01.h"
%include "src/Nsound/DrumKickBass.h"
%include "src/Nsound/EnvelopeAdsr.h"
%include "src/Nsound/FFTransform.h"
%include "src/Nsound/Filter.h"
%include "src/Nsound/FilterAllPass.h"
%include "src/Nsound/FilterBandPassVocoder.h"
%include "src/Nsound/FilterCombLowPassFeedback.h"
%include "src/Nsound/FilterDelay.h"
%include "src/Nsound/FilterDC.h"
%include "src/Nsound/FilterIIR.h"
%include "src/Nsound/FilterStageIIR.h"
%include "src/Nsound/FilterLeastSquaresFIR.h"
%include "src/Nsound/FilterLowPassFIR.h"
%include "src/Nsound/FilterLowPassIIR.h"
%include "src/Nsound/FilterLowPassMoogVcf.h"
%include "src/Nsound/FilterHighPassFIR.h"
%include "src/Nsound/FilterHighPassIIR.h"
%include "src/Nsound/FilterBandRejectFIR.h"
%include "src/Nsound/FilterBandRejectIIR.h"
%include "src/Nsound/FilterBandPassFIR.h"
%include "src/Nsound/FilterBandPassIIR.h"
%include "src/Nsound/FilterMedian.hpp"
%include "src/Nsound/FilterMovingAverage.h"
%include "src/Nsound/FilterFlanger.h"
%include "src/Nsound/FilterParametricEqualizer.h"
%include "src/Nsound/FilterPhaser.h"
%include "src/Nsound/FilterSlinky.h"
%include "src/Nsound/FilterTone.h"
%include "src/Nsound/FluteSlide.h"
%include "src/Nsound/Generator.h"
%include "src/Nsound/GeneratorDecay.h"
%include "src/Nsound/Granulator.h"
%include "src/Nsound/GuitarBass.h"
%include "src/Nsound/Hat.h"
//    %include "src/Nsound/Kernel.h"
%include "src/Nsound/Mesh2D.h"
//    %include "src/Nsound/MeshJunction.h"
%include "src/Nsound/Mixer.h"
%include "src/Nsound/OrganPipe.h"
%include "src/Nsound/Plotter.h"
%include "src/Nsound/Pluck.h"
%include "src/Nsound/Pulse.h"
%include "src/Nsound/RandomNumberGenerator.h"
%include "src/Nsound/ReverberationRoom.h"
%include "src/Nsound/RngTausworthe.h"
%include "src/Nsound/Sawtooth.h"
%include "src/Nsound/Sine.h"
%include "src/Nsound/Spectrogram.h"
%include "src/Nsound/Cosine.h"
%include "src/Nsound/Square.h"
%include "src/Nsound/Stretcher.h"
%include "src/Nsound/TicToc.h"
%include "src/Nsound/Triangle.h"
%include "src/Nsound/Utils.h"
%include "src/Nsound/Vocoder.h"
%include "src/Nsound/Wavefile.h"

%pythoncode
%{

#------------------------------------------------------------------------------
# from nsound.i

# python modules
import inspect
import os.path
import types
import sys
import warnings

import matplotlib.pylab

__package__ = "%s" % PACKAGE_RELEASE
__version__ = "%s" % PACKAGE_VERSION
__author__ = "%s" % NSOUND_AUTHORS
__copyright__ = "%s" % NSOUND_COPYRIGHT
__license__ = "GNU General Public License 2.0 or later"

def get_caller(offset = 0, basename = False):

    filename, line_number = inspect.stack()[1 + offset][1:3]

    if basename:
        filename = os.path.basename(filename)

    return filename, line_number

def rel_to_abs(path):

    if os.path.isabs(path):
        return path

    caller, line_no = get_caller(offset = 1)
    caller = os.path.abspath(caller)
    prefix = os.path.dirname(caller)

    return os.path.abspath(os.path.join(prefix, path))

%}

// :mode=c++:
