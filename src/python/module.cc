//-----------------------------------------------------------------------------
//
//  The Nsound Python module, _Nsound, which Nsound/__init__.py imports.
//
//-----------------------------------------------------------------------------

#include "bindings.h"

PYBIND11_MODULE(_Nsound, m)
{
    m.doc() = "Nsound, audio synthesis with dynamic digital filters.";

    py::register_exception<Nsound::Exception>(
        m, "Exception", PyExc_RuntimeError);

    // The enums come first, the default arguments use them.  Then the
    // classes, each after the classes its methods take by default.
    bind_constants(m);
    bind_buffer(m);
    bind_audio_stream(m);
    bind_generators(m);
    bind_filters(m);
    bind_instruments(m);
    bind_effects(m);
    bind_audio_io(m);
}
