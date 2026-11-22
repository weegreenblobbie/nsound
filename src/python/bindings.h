//-----------------------------------------------------------------------------
//
//  What the Python binding files share: pybind11, the std::vector and
//  std::string conversions, and one bind_*() function per file.
//
//-----------------------------------------------------------------------------

#pragma once

// pybind11 includes Python.h, which must come first.
#include <pybind11/pybind11.h>
#include <pybind11/native_enum.h>
#include <pybind11/stl.h>

#include <Nsound/Nsound.h>

#include <string>
#include <type_traits>

namespace py = pybind11;

namespace Nsound
{
    class Buffer;
};

//! str(Buffer), all the samples of a short Buffer, the first and last 6 of
//! a long one.
std::string buffer_str(const Nsound::Buffer & b);

//! Adds Class(const Class &) when the class can be copied, Python's way to
//! copy an object as it was with SWIG.
template <class PyClass>
void
def_copy(PyClass & cls)
{
    using T = typename PyClass::type;

    if constexpr (std::is_copy_constructible_v<T>)
    {
        cls.def(py::init<const T &>());
    }
}

void bind_constants(py::module_ & m);
void bind_buffer(py::module_ & m);
void bind_audio_stream(py::module_ & m);
void bind_generators(py::module_ & m);
void bind_filters(py::module_ & m);
void bind_instruments(py::module_ & m);
void bind_effects(py::module_ & m);
void bind_audio_io(py::module_ & m);
