//-----------------------------------------------------------------------------
//
//  The Python bindings for Nsound's enums and its build settings.
//
//-----------------------------------------------------------------------------

#include "bindings.h"

#include <Nsound/AudioBackendType.h>
#include <Nsound/AudioPlaybackRt.h>
#include <Nsound/WindowType.h>

using namespace Nsound;

void
bind_constants(py::module_ & m)
{
    // Python IntEnums, their values are ints in the module too.
    py::native_enum<WindowType>(m, "WindowType", "enum.IntEnum")
        .value("BARTLETT", BARTLETT)
        .value("BLACKMAN", BLACKMAN)
        .value("BLACKMAN_HARRIS", BLACKMAN_HARRIS)
        .value("GAUSSIAN", GAUSSIAN)
        .value("GAUSSIAN_05", GAUSSIAN_05)
        .value("GAUSSIAN_10", GAUSSIAN_10)
        .value("GAUSSIAN_15", GAUSSIAN_15)
        .value("GAUSSIAN_20", GAUSSIAN_20)
        .value("GAUSSIAN_25", GAUSSIAN_25)
        .value("GAUSSIAN_30", GAUSSIAN_30)
        .value("GAUSSIAN_33", GAUSSIAN_33)
        .value("GAUSSIAN_35", GAUSSIAN_35)
        .value("GAUSSIAN_40", GAUSSIAN_40)
        .value("GAUSSIAN_45", GAUSSIAN_45)
        .value("GAUSSIAN_50", GAUSSIAN_50)
        .value("GAUSSIAN_55", GAUSSIAN_55)
        .value("GAUSSIAN_60", GAUSSIAN_60)
        .value("GAUSSIAN_65", GAUSSIAN_65)
        .value("GAUSSIAN_66", GAUSSIAN_66)
        .value("GAUSSIAN_70", GAUSSIAN_70)
        .value("GAUSSIAN_75", GAUSSIAN_75)
        .value("GAUSSIAN_80", GAUSSIAN_80)
        .value("GAUSSIAN_85", GAUSSIAN_85)
        .value("GAUSSIAN_90", GAUSSIAN_90)
        .value("GAUSSIAN_95", GAUSSIAN_95)
        .value("GAUSSIAN_99", GAUSSIAN_99)
        .value("HAMMING", HAMMING)
        .value("HANNING", HANNING)
        .value("KAISER", KAISER)
        .value("NUTTALL", NUTTALL)
        .value("PARZEN", PARZEN)
        .value("RECTANGULAR", RECTANGULAR)
        .export_values()
        .finalize();

    py::native_enum<AudioBackendType>(m, "AudioBackendType", "enum.IntEnum")
        .value("BACKEND_TYPE_NONE", BACKEND_TYPE_NONE)
        .value("BACKEND_TYPE_LIBAO", BACKEND_TYPE_LIBAO)
        .value("BACKEND_TYPE_LIBPORTAUDIO", BACKEND_TYPE_LIBPORTAUDIO)
        .export_values()
        .finalize();

    py::native_enum<BufferUnderrunMode>(m, "BufferUnderrunMode", "enum.IntEnum")
        .value("BUM_SILENCE", BUM_SILENCE)
        .value("BUM_NOISE", BUM_NOISE)
        .value("BUM_TONE", BUM_TONE)
        .export_values()
        .finalize();

    m.attr("M_PI") = M_PI;

    m.attr("PACKAGE_NAME") = PACKAGE_NAME;
    m.attr("PACKAGE_VERSION") = PACKAGE_VERSION;
    m.attr("PACKAGE_RELEASE") = PACKAGE_RELEASE;
    m.attr("NSOUND_AUTHORS") = NSOUND_AUTHORS;
    m.attr("NSOUND_COPYRIGHT") = NSOUND_COPYRIGHT;
    m.attr("NSOUND_VERSION_A") = NSOUND_VERSION_A;
    m.attr("NSOUND_VERSION_B") = NSOUND_VERSION_B;
    m.attr("NSOUND_VERSION_C") = NSOUND_VERSION_C;

    // The optional features, 1 when the module was built with them.
    #ifdef NSOUND_64_BIT
        m.attr("NSOUND_64_BIT") = 1;
    #endif

    #ifdef NSOUND_C_PYLAB
        m.attr("NSOUND_C_PYLAB") = 1;
    #endif

    #ifdef NSOUND_IN_PYTHON_MODULE
        m.attr("NSOUND_IN_PYTHON_MODULE") = 1;
    #endif

    #ifdef NSOUND_LIBAO
        m.attr("NSOUND_LIBAO") = 1;
    #endif

    #ifdef NSOUND_LIBPORTAUDIO
        m.attr("NSOUND_LIBPORTAUDIO") = 1;
    #endif

    #ifdef NSOUND_OPENMP
        m.attr("NSOUND_OPENMP") = 1;
    #endif
}
