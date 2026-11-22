//-----------------------------------------------------------------------------
//
//  The Python bindings for Buffers, their selections and iterators.
//
//-----------------------------------------------------------------------------

#include "bindings.h"

#include <Nsound/AudioPlayback.h>
#include <Nsound/AudioPlaybackRt.h>
#include <Nsound/AudioStream.h>
#include <Nsound/Buffer.h>
#include <Nsound/BufferSelection.h>
#include <Nsound/BufferWindowSearch.h>
#include <Nsound/CircularBuffer.h>
#include <Nsound/CircularIterators.h>
#include <Nsound/Wavefile.h>

#include <sstream>
#include <string>
#include <string_view>

using namespace Nsound;

//! The index of sample i, counting from the end when i < 0 as Python does.
static
uint32
sample_index(const Buffer & b, int64 i)
{
    int64 n = b.getLength();

    int64 j = i < 0 ? i + n : i;

    if(j < 0 || j >= n)
    {
        throw py::index_error(
            "Buffer index " + std::to_string(i) + " is out of range, "
            "the Buffer holds " + std::to_string(n) + " samples");
    }

    return static_cast<uint32>(j);
}

std::string
buffer_str(const Buffer & b)
{
    const uint32 N = 6;

    uint32 size = b.getLength();

    std::ostringstream ss;
    ss << "Nsound.Buffer([";

    if(size <= 2 * N)
    {
        for(uint32 i = 0; i < size; ++i) ss << b[i] << ", ";
    }
    else
    {
        for(uint32 i = 0; i < N; ++i) ss << b[i] << ", ";

        ss << "... ";

        for(uint32 i = size - N; i < size; ++i) ss << b[i] << ", ";
    }

    ss << "])";

    return ss.str();
}

void
bind_buffer(py::module_ & m)
{
    // The iterators come from Buffer.cbegin(), they point into the Buffer's
    // samples.
    py::class_<circular_iterator> circular(m, "circular_iterator");

    circular
        .def("reset",
            &circular_iterator::reset)
        .def("__float__",
            &circular_iterator::__float__)
        .def("__str__",
            &circular_iterator::__str__)
        .def("__iadd__",
            &circular_iterator::operator+=,
            py::arg("n"),
            py::return_value_policy::reference_internal)
        .def("__isub__",
            &circular_iterator::operator-=,
            py::arg("n"),
            py::return_value_policy::reference_internal)
        .def("__eq__",
            &circular_iterator::operator==,
            py::is_operator())
        .def("__ne__",
            &circular_iterator::operator!=,
            py::is_operator())
        .def(py::init<const circular_iterator &>(),
            py::keep_alive<1, 2>());

    py::class_<const_circular_iterator> const_circular(m, "const_circular_iterator");

    const_circular
        .def("reset",
            &const_circular_iterator::reset)
        .def("__float__",
            &const_circular_iterator::__float__)
        .def("__iadd__",
            &const_circular_iterator::operator+=,
            py::arg("n"),
            py::return_value_policy::reference_internal)
        .def("__isub__",
            &const_circular_iterator::operator-=,
            py::arg("n"),
            py::return_value_policy::reference_internal)
        .def("__eq__",
            &const_circular_iterator::operator==,
            py::is_operator())
        .def("__ne__",
            &const_circular_iterator::operator!=,
            py::is_operator())
        .def(py::init<const const_circular_iterator &>(),
            py::keep_alive<1, 2>());

    // The samples of a Buffer where a BooleanVector is true, from Buffer's
    // __call__, it keeps the Buffer alive.
    py::class_<BufferSelection> buffer_selection(m, "BufferSelection");

    buffer_selection
        .def(py::init<Buffer &, const BooleanVector &>(),
            py::arg("buffer"),
            py::arg("bv"),
            py::keep_alive<1, 2>())
        .def("set",
            &BufferSelection::set,
            py::arg("rhs"))
        .def("__iadd__",
            &BufferSelection::operator+=,
            py::arg("rhs"),
            py::return_value_policy::reference_internal)
        .def("__isub__",
            &BufferSelection::operator-=,
            py::arg("rhs"),
            py::return_value_policy::reference_internal)
        .def("__imul__",
            &BufferSelection::operator*=,
            py::arg("rhs"),
            py::return_value_policy::reference_internal)
        .def("__itruediv__",
            &BufferSelection::operator/=,
            py::arg("rhs"),
            py::return_value_policy::reference_internal)
        .def("__ixor__",
            &BufferSelection::operator^=,
            py::arg("rhs"),
            py::return_value_policy::reference_internal)
        .def(py::init<const BufferSelection &>(),
            py::keep_alive<1, 2>());

    py::class_<Buffer> buffer(m, "Buffer");

    buffer
        .def(py::init<>())
        .def(py::init<uint32>(),
            py::arg("chunk_size"))
        .def(py::init<const FloatVector &>(),
            py::arg("list"))
        .def(py::init<const std::string &, uint32>(),
            py::arg("filename"),
            py::arg("chunk_size") = 4096)
        .def("abs",
            &Buffer::abs)
        .def("getAbs",
            &Buffer::getAbs)
        .def("add",
            &Buffer::add,
            py::arg("buffer"),
            py::arg("offset") = 0,
            py::arg("n_samples") = 0)
        .def("argmax",
            &Buffer::argmax)
        .def("argmin",
            &Buffer::argmin)
        .def("getNBytes",
            &Buffer::getNBytes)
        .def("exp",
            &Buffer::exp)
        .def("getExp",
            &Buffer::getExp)
        .def("convolve",
            &Buffer::convolve,
            py::arg("H"))
        .def("getConvolve",
            &Buffer::getConvolve,
            py::arg("H"))
        .def("dB",
            &Buffer::dB)
        .def("getdB",
            &Buffer::getdB)
        .def("derivative",
            &Buffer::derivative,
            py::arg("n"))
        .def("getDerivative",
            &Buffer::getDerivative,
            py::arg("n"))
        .def("downSample",
            &Buffer::downSample,
            py::arg("n"))
        .def("getDownSample",
            &Buffer::getDownSample,
            py::arg("n"))
        .def("findPeaks",
            &Buffer::findPeaks,
            py::arg("window_size") = 0,
            py::arg("min_height") = 0.0)
        .def("getLength",
            &Buffer::getLength)
        .def("limit",
            py::overload_cast<float64, float64>(&Buffer::limit),
            py::arg("min"),
            py::arg("max"))
        .def("limit",
            py::overload_cast<const Buffer &, const Buffer &>(&Buffer::limit),
            py::arg("min"),
            py::arg("max"))
        .def("getLimit",
            py::overload_cast<float64, float64>(&Buffer::getLimit, py::const_),
            py::arg("min"),
            py::arg("max"))
        .def("getLimit",
            py::overload_cast<const Buffer &, const Buffer &>(&Buffer::getLimit, py::const_),
            py::arg("min"),
            py::arg("max"))
        .def("log",
            &Buffer::log)
        .def("getLog",
            &Buffer::getLog)
        .def("log10",
            &Buffer::log10)
        .def("getLog10",
            &Buffer::getLog10)
        .def("getMax",
            &Buffer::getMax)
        .def("getMaxMagnitude",
            &Buffer::getMaxMagnitude)
        .def("getMean",
            &Buffer::getMean)
        .def("getMin",
            &Buffer::getMin)
        .def("mul",
            &Buffer::mul,
            py::arg("buffer"),
            py::arg("offset") = 0,
            py::arg("n_samples") = 0)
        .def("normalize",
            &Buffer::normalize)
        .def("getNormalize",
            &Buffer::getNormalize)
        .def("getSignalEnergy",
            &Buffer::getSignalEnergy,
            py::arg("window_size"))
        .def("getStd",
            &Buffer::getStd)
        .def("getSum",
            &Buffer::getSum)
        .def("zNorm",
            &Buffer::zNorm)
        .def("getZNorm",
            &Buffer::getZNorm)
        .def("plot",
            &Buffer::plot,
            py::arg("title") = "Buffer")
        .def("preallocate",
            &Buffer::preallocate,
            py::arg("n"))
        .def("readWavefile",
            &Buffer::readWavefile,
            py::arg("filename"))
        .def("resample",
            py::overload_cast<float64>(&Buffer::resample),
            py::arg("factor"))
        .def("resample",
            py::overload_cast<const Buffer &>(&Buffer::resample),
            py::arg("factor"))
        .def("getResample",
            py::overload_cast<float64, const uint32, float64>(&Buffer::getResample, py::const_),
            py::arg("factor"),
            py::arg("N") = 10,
            py::arg("beta") = 5.0)
        .def("getResample",
            py::overload_cast<const Buffer &, const uint32, float64>(&Buffer::getResample, py::const_),
            py::arg("factor"),
            py::arg("N") = 10,
            py::arg("beta") = 5.0)
        .def("getResample",
            py::overload_cast<const uint32, const uint32, const uint32, float64>(&Buffer::getResample, py::const_),
            py::arg("L"),
            py::arg("M"),
            py::arg("N") = 10,
            py::arg("beta") = 5.0)
        .def("reverse",
            &Buffer::reverse)
        .def("getReverse",
            &Buffer::getReverse)
        .def("round",
            &Buffer::round)
        .def("getRound",
            &Buffer::getRound)
        .def("select",
            &Buffer::select,
            py::arg("start_index"),
            py::arg("stop_index"))
        .def("write",
            [](const Buffer & self)
            {
                return py::bytearray(self.write());
            },
            "Returns the samples as a bytearray, for read().")
        .def("read",
            [](Buffer & self, const std::string & data)
            {
                self.read(data.data(), data.size());
            },
            py::arg("data"),
            "Reads the samples from write()'s bytes or bytearray.")
        .def("smooth",
            &Buffer::smooth,
            py::arg("n_passes"),
            py::arg("n_samples_per_average"))
        .def("getSmooth",
            &Buffer::getSmooth,
            py::arg("n_passes"),
            py::arg("n_samples_per_average"))
        .def("speedUp",
            py::overload_cast<float64>(&Buffer::speedUp),
            py::arg("step_size"))
        .def("speedUp",
            py::overload_cast<const Buffer &>(&Buffer::speedUp),
            py::arg("step_size"))
        .def("getSpeedUp",
            py::overload_cast<float64>(&Buffer::getSpeedUp, py::const_),
            py::arg("step_size"))
        .def("getSpeedUp",
            py::overload_cast<const Buffer &>(&Buffer::getSpeedUp, py::const_),
            py::arg("step_size"))
        .def("sqrt",
            &Buffer::sqrt)
        .def("getSqrt",
            &Buffer::getSqrt)
        .def("subbuffer",
            &Buffer::subbuffer,
            py::arg("start_index"),
            py::arg("n_samples") = 0)
        .def("upSample",
            &Buffer::upSample,
            py::arg("n"))
        .def("getUpSample",
            &Buffer::getUpSample,
            py::arg("n"))
        .def("writeWavefile",
            &Buffer::writeWavefile,
            py::arg("filename"))
        .def_static("ones",
            &Buffer::ones,
            py::arg("n_samples"))
        .def_static("rand",
            &Buffer::rand,
            py::arg("n_samples"))
        .def_static("zeros",
            &Buffer::zeros,
            py::arg("n_samples"));

    def_copy(buffer);

    // Python's sequence protocol: len(), b[i], b[i:j] and b[i] = x.
    // Python iterates with __getitem__ until IndexError, so appending to
    // a Buffer while iterating over it is safe.
    buffer
        .def("__len__",
            &Buffer::getLength)
        .def("__getitem__",
            [](const Buffer & self, int64 i)
            {
                return self[sample_index(self, i)];
            },
            py::arg("index"))
        .def("__getitem__",
            [](const Buffer & self, const py::slice & slice)
            {
                py::ssize_t start, stop, step, length;

                if(! slice.compute(self.getLength(), &start, &stop, &step, &length))
                {
                    throw py::error_already_set();
                }

                Buffer out;

                for(py::ssize_t i = 0; i < length; ++i)
                {
                    out << self[static_cast<uint32>(start + i * step)];
                }

                return out;
            },
            py::arg("slice"))
        .def("__setitem__",
            [](Buffer & self, int64 i, float64 x)
            {
                self[sample_index(self, i)] = x;
            },
            py::arg("index"),
            py::arg("x"))
        .def("toList",
            [](const Buffer & self)
            {
                return std::vector<float64>(self.begin(), self.end());
            },
            "Returns the samples as a list.")
        .def("cbegin",
            py::overload_cast<>(&Buffer::cbegin),
            "Returns an iterator that wraps around to the start.",
            py::keep_alive<0, 1>())
        .def("__str__",
            &buffer_str)
        .def("__repr__",
            [](const Buffer & self)
            {
                return "Nsound.Buffer holding "
                    + std::to_string(self.getLength())
                    + " samples";
            });

    // b << x appends a Buffer, an AudioStream's mono samples, a sample, a
    // wavefile's samples or each number in an iterable.
    buffer
        .def("__lshift__",
            py::overload_cast<const AudioStream &>(&Buffer::operator<<),
            py::return_value_policy::reference_internal)
        .def("__lshift__",
            py::overload_cast<const Buffer &>(&Buffer::operator<<),
            py::return_value_policy::reference_internal)
        .def("__lshift__",
            [](Buffer & self, const std::string & filename) -> Buffer &
            {
                return self << filename.c_str();
            },
            py::return_value_policy::reference_internal)
        .def("__lshift__",
            py::overload_cast<float64>(&Buffer::operator<<),
            py::return_value_policy::reference_internal)
        .def("__lshift__",
            [](Buffer & self, const py::iterable & samples) -> Buffer &
            {
                for(const py::handle & x : samples)
                {
                    self << x.cast<float64>();
                }

                return self;
            },
            py::return_value_policy::reference_internal);

    // b >> x writes a wavefile, plays the samples or appends them to a list.
    buffer
        .def("__rshift__",
            [](const Buffer & self, const std::string & filename)
            {
                self >> filename.c_str();
            })
        .def("__rshift__",
            [](const Buffer & self, AudioPlayback & playback)
            {
                self >> playback;
            })
        .def("__rshift__",
            [](const Buffer & self, AudioPlaybackRt & playback)
            {
                self >> playback;
            })
        .def("__rshift__",
            [](const Buffer & self, py::list & samples)
            {
                for(float64 x : self)
                {
                    samples.append(x);
                }
            });

    // Arithmetic with Buffers and numbers, ^ is the power.
    buffer
        .def("__iadd__",
            py::overload_cast<const Buffer &>(&Buffer::operator+=),
            py::return_value_policy::reference_internal)
        .def("__iadd__",
            py::overload_cast<float64>(&Buffer::operator+=),
            py::return_value_policy::reference_internal)
        .def("__isub__",
            py::overload_cast<const Buffer &>(&Buffer::operator-=),
            py::return_value_policy::reference_internal)
        .def("__isub__",
            py::overload_cast<float64>(&Buffer::operator-=),
            py::return_value_policy::reference_internal)
        .def("__imul__",
            py::overload_cast<const Buffer &>(&Buffer::operator*=),
            py::return_value_policy::reference_internal)
        .def("__imul__",
            py::overload_cast<float64>(&Buffer::operator*=),
            py::return_value_policy::reference_internal)
        .def("__itruediv__",
            py::overload_cast<const Buffer &>(&Buffer::operator/=),
            py::return_value_policy::reference_internal)
        .def("__itruediv__",
            py::overload_cast<float64>(&Buffer::operator/=),
            py::return_value_policy::reference_internal)
        .def("__ipow__",
            py::overload_cast<const Buffer &>(&Buffer::operator^=),
            py::return_value_policy::reference_internal)
        .def("__ipow__",
            py::overload_cast<float64>(&Buffer::operator^=),
            py::return_value_policy::reference_internal)
        .def("__ixor__",
            py::overload_cast<const Buffer &>(&Buffer::operator^=),
            py::return_value_policy::reference_internal)
        .def("__ixor__",
            py::overload_cast<float64>(&Buffer::operator^=),
            py::return_value_policy::reference_internal)
        .def("__add__",
            [](const Buffer & a, const Buffer & b) { return a + b; },
            py::is_operator())
        .def("__add__",
            [](const Buffer & a, float64 x) { return a + x; },
            py::is_operator())
        .def("__radd__",
            [](const Buffer & a, float64 x) { return x + a; },
            py::is_operator())
        .def("__sub__",
            [](const Buffer & a, const Buffer & b) { return a - b; },
            py::is_operator())
        .def("__sub__",
            [](const Buffer & a, float64 x) { return a - x; },
            py::is_operator())
        .def("__rsub__",
            [](const Buffer & a, float64 x) { return x - a; },
            py::is_operator())
        .def("__mul__",
            [](const Buffer & a, const Buffer & b) { return a * b; },
            py::is_operator())
        .def("__mul__",
            [](const Buffer & a, float64 x) { return a * x; },
            py::is_operator())
        .def("__rmul__",
            [](const Buffer & a, float64 x) { return x * a; },
            py::is_operator())
        .def("__truediv__",
            [](const Buffer & a, const Buffer & b) { return a / b; },
            py::is_operator())
        .def("__truediv__",
            [](const Buffer & a, float64 x) { return a / x; },
            py::is_operator())
        .def("__rtruediv__",
            [](const Buffer & a, float64 x) { return x / a; },
            py::is_operator())
        .def("__pow__",
            [](const Buffer & a, const Buffer & b) { return a ^ b; },
            py::is_operator())
        .def("__pow__",
            [](const Buffer & a, float64 x) { return a ^ x; },
            py::is_operator());

    // b == other is a bool, comparing with a number gives a BooleanVector,
    // which b(bv) uses to select samples.
    buffer
        .def("__eq__",
            [](const Buffer & a, const Buffer & b) { return a == b; },
            py::is_operator())
        .def("__eq__",
            [](Buffer & a, float64 x) { return a == x; },
            py::is_operator())
        .def("__ne__",
            [](const Buffer & a, const Buffer & b) { return a != b; },
            py::is_operator())
        .def("__ne__",
            [](Buffer & a, float64 x) { return a != x; },
            py::is_operator())
        .def("__gt__",
            [](Buffer & a, float64 x) { return a > x; },
            py::is_operator())
        .def("__ge__",
            [](Buffer & a, float64 x) { return a >= x; },
            py::is_operator())
        .def("__lt__",
            [](Buffer & a, float64 x) { return a < x; },
            py::is_operator())
        .def("__le__",
            [](Buffer & a, float64 x) { return a <= x; },
            py::is_operator())
        .def("__call__",
            &Buffer::operator(),
            py::arg("bv"),
            py::keep_alive<0, 1>());

    // Pickling, with write() and read().
    buffer
        .def(py::pickle(
            [](const Buffer & self)
            {
                return py::bytes(self.write());
            },
            [](const py::bytes & state)
            {
                std::string_view bytes = state;

                Buffer b;
                b.read(bytes.data(), bytes.size());

                return b;
            }));

    py::class_<BufferWindowSearch> buffer_window_search(m, "BufferWindowSearch");

    // It points to the Buffer it searches, which it keeps alive.
    buffer_window_search
        .def(py::init<const Buffer &, uint32>(),
            py::arg("buffer"),
            py::arg("window_size") = 2048,
            py::keep_alive<1, 2>())
        .def(py::init<const BufferWindowSearch &>(),
            py::keep_alive<1, 2>())
        .def("getNextWindow",
            &BufferWindowSearch::getNextWindow)
        .def("getSamplesLeft",
            &BufferWindowSearch::getSamplesLeft)
        .def("reset",
            &BufferWindowSearch::reset)
        .def("setBuffer",
            &BufferWindowSearch::setBuffer,
            py::arg("buffer"),
            py::keep_alive<1, 2>());

    py::class_<CircularBuffer> circular_buffer(m, "CircularBuffer");

    circular_buffer
        .def(py::init<uint32>(),
            py::arg("n_samples"))
        .def("read",
            &CircularBuffer::read)
        .def("read_head",
            &CircularBuffer::read_head)
        .def("write",
            py::overload_cast<float64>(&CircularBuffer::write),
            py::arg("d"))
        .def("write",
            py::overload_cast<const AudioStream &>(&CircularBuffer::write),
            py::arg("as"))
        .def("write",
            py::overload_cast<const Buffer &>(&CircularBuffer::write),
            py::arg("b"));

    def_copy(circular_buffer);
}
