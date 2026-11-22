//-----------------------------------------------------------------------------
//
//  The Python bindings for AudioStreams and their selections.
//
//-----------------------------------------------------------------------------

#include "bindings.h"

#include <Nsound/AudioPlayback.h>
#include <Nsound/AudioPlaybackRt.h>
#include <Nsound/AudioStream.h>
#include <Nsound/AudioStreamSelection.h>
#include <Nsound/Buffer.h>
#include <Nsound/Wavefile.h>

#include <cstdio>
#include <string>
#include <string_view>

using namespace Nsound;

//! The index of channel i, counting from the end when i < 0 as Python does.
static
uint32
channel_index(const AudioStream & as, int64 i)
{
    int64 n = as.getNChannels();

    int64 j = i < 0 ? i + n : i;

    if(j < 0 || j >= n)
    {
        throw py::index_error(
            "AudioStream index " + std::to_string(i) + " is out of range, "
            "the AudioStream has " + std::to_string(n) + " channels");
    }

    return static_cast<uint32>(j);
}

void
bind_audio_stream(py::module_ & m)
{
    // The samples of an AudioStream where a BooleanVectorVector is true,
    // from AudioStream's __call__, it keeps the AudioStream alive.
    py::class_<AudioStreamSelection> audio_stream_selection(m, "AudioStreamSelection");

    audio_stream_selection
        .def(py::init<AudioStream &, const BooleanVectorVector &>(),
            py::arg("as"),
            py::arg("bv"),
            py::keep_alive<1, 2>())
        .def(py::init<const AudioStreamSelection &>(),
            py::keep_alive<1, 2>())
        .def("set",
            &AudioStreamSelection::set,
            py::arg("rhs"))
        .def("__iadd__",
            &AudioStreamSelection::operator+=,
            py::arg("rhs"),
            py::return_value_policy::reference_internal)
        .def("__isub__",
            &AudioStreamSelection::operator-=,
            py::arg("rhs"),
            py::return_value_policy::reference_internal)
        .def("__imul__",
            &AudioStreamSelection::operator*=,
            py::arg("rhs"),
            py::return_value_policy::reference_internal)
        .def("__itruediv__",
            &AudioStreamSelection::operator/=,
            py::arg("rhs"),
            py::return_value_policy::reference_internal)
        .def("__ixor__",
            &AudioStreamSelection::operator^=,
            py::arg("rhs"),
            py::return_value_policy::reference_internal);

    py::class_<AudioStream> audio_stream(m, "AudioStream");

    audio_stream
        .def(py::init<>())
        .def(py::init<float64, uint32, uint32>(),
            py::arg("sample_rate"),
            py::arg("n_channels") = 1,
            py::arg("n_samples_pre_allocate") = 4096)
        .def(py::init<const std::string &>(),
            py::arg("filename"))
        .def("abs",
            &AudioStream::abs)
        .def("getAbs",
            &AudioStream::getAbs)
        .def("add",
            py::overload_cast<const AudioStream &, uint32, uint32>(&AudioStream::add),
            py::arg("as"),
            py::arg("offset"),
            py::arg("n_samples") = 0)
        .def("add",
            py::overload_cast<const AudioStream &, float64, float64>(&AudioStream::add),
            py::arg("as"),
            py::arg("offset_seconds"),
            py::arg("duration_seconds") = 0.0)
        .def("convolve",
            &AudioStream::convolve,
            py::arg("b"))
        .def("getConvolve",
            &AudioStream::getConvolve,
            py::arg("b"))
        .def("dB",
            &AudioStream::dB)
        .def("getdB",
            &AudioStream::getdB)
        .def("derivative",
            &AudioStream::derivative,
            py::arg("n"))
        .def("getDerivative",
            &AudioStream::getDerivative,
            py::arg("n"))
        .def("downSample",
            &AudioStream::downSample,
            py::arg("n"))
        .def("getDownSample",
            &AudioStream::getDownSample,
            py::arg("n"))
        .def("getDuration",
            &AudioStream::getDuration)
        .def("getLength",
            &AudioStream::getLength)
        .def("limit",
            py::overload_cast<float64, float64>(&AudioStream::limit),
            py::arg("min"),
            py::arg("max"))
        .def("limit",
            py::overload_cast<const Buffer &, const Buffer &>(&AudioStream::limit),
            py::arg("min"),
            py::arg("max"))
        .def("getLimit",
            py::overload_cast<float64, float64>(&AudioStream::getLimit, py::const_),
            py::arg("min"),
            py::arg("max"))
        .def("getLimit",
            py::overload_cast<const Buffer &, const Buffer &>(&AudioStream::getLimit, py::const_),
            py::arg("min"),
            py::arg("max"))
        .def("getMax",
            &AudioStream::getMax)
        .def("getMaxMagnitude",
            &AudioStream::getMaxMagnitude)
        .def("getMin",
            &AudioStream::getMin)
        .def("mono",
            &AudioStream::mono)
        .def("getMono",
            &AudioStream::getMono)
        .def("getNChannels",
            &AudioStream::getNChannels)
        .def("getSampleRate",
            &AudioStream::getSampleRate)
        .def("normalize",
            &AudioStream::normalize)
        .def("getNormalize",
            &AudioStream::getNormalize)
        .def("pad",
            &AudioStream::pad,
            py::arg("fill") = 0.0)
        .def("getPad",
            &AudioStream::getPad,
            py::arg("fill") = 0.0)
        .def("pan",
            py::overload_cast<float64>(&AudioStream::pan),
            py::arg("pan"))
        .def("pan",
            py::overload_cast<const Buffer &>(&AudioStream::pan),
            py::arg("pan"))
        .def("getPan",
            py::overload_cast<float64>(&AudioStream::getPan, py::const_),
            py::arg("pan"))
        .def("getPan",
            py::overload_cast<const Buffer &>(&AudioStream::getPan, py::const_),
            py::arg("pan"))
        .def("plot",
            &AudioStream::plot,
            py::arg("title") = "AudioStream")
        .def("readWavefile",
            &AudioStream::readWavefile,
            py::arg("filename"))
        .def("resample",
            py::overload_cast<float64>(&AudioStream::resample),
            py::arg("factor"))
        .def("resample",
            py::overload_cast<const Buffer &>(&AudioStream::resample),
            py::arg("factor"))
        .def("getResample",
            py::overload_cast<float64>(&AudioStream::getResample, py::const_),
            py::arg("factor"))
        .def("getResample",
            py::overload_cast<const Buffer &>(&AudioStream::getResample, py::const_),
            py::arg("factor"))
        .def("resample2",
            &AudioStream::resample2,
            py::arg("new_sample_rate"))
        .def("getResample2",
            &AudioStream::getResample2,
            py::arg("new_sample_rate"))
        .def("reverse",
            &AudioStream::reverse)
        .def("getReverse",
            &AudioStream::getReverse)
        .def("select",
            &AudioStream::select,
            py::arg("start_index"),
            py::arg("stop_index"))
        .def("write",
            [](const AudioStream & self)
            {
                return py::bytearray(self.write());
            },
            "Returns the channels as a bytearray, for read().")
        .def("read",
            [](AudioStream & self, const std::string & data)
            {
                self.read(data.data(), data.size());
            },
            py::arg("data"),
            "Reads the channels from write()'s bytes or bytearray.")
        .def("setNChannels",
            &AudioStream::setNChannels,
            py::arg("channels"))
        .def("setSampleRate",
            &AudioStream::setSampleRate,
            py::arg("sample_rate"))
        .def("smooth",
            &AudioStream::smooth,
            py::arg("n_passes"),
            py::arg("n_samples_per_average"))
        .def("getSmooth",
            &AudioStream::getSmooth,
            py::arg("n_passes"),
            py::arg("n_samples_per_average"))
        .def("speedUp",
            py::overload_cast<float32>(&AudioStream::speedUp),
            py::arg("step_size"))
        .def("speedUp",
            py::overload_cast<const Buffer &>(&AudioStream::speedUp),
            py::arg("step_size"))
        .def("getSpeedUp",
            py::overload_cast<float32>(&AudioStream::getSpeedUp, py::const_),
            py::arg("step_size"))
        .def("getSpeedUp",
            py::overload_cast<const Buffer &>(&AudioStream::getSpeedUp, py::const_),
            py::arg("step_size"))
        .def("sqrt",
            &AudioStream::sqrt)
        .def("getSqrt",
            &AudioStream::getSqrt)
        .def("substream",
            py::overload_cast<uint32, uint32>(&AudioStream::substream, py::const_),
            py::arg("start_index"),
            py::arg("n_samples") = 0)
        .def("substream",
            py::overload_cast<int32, int32>(&AudioStream::substream, py::const_),
            py::arg("start_index"),
            py::arg("n_samples") = 0)
        .def("substream",
            py::overload_cast<float64, float64>(&AudioStream::substream, py::const_),
            py::arg("start_time"),
            py::arg("duration") = 0)
        .def("transpose",
            &AudioStream::transpose)
        .def("getTranspose",
            &AudioStream::getTranspose)
        .def("upSample",
            &AudioStream::upSample,
            py::arg("n"))
        .def("getUpSample",
            &AudioStream::getUpSample,
            py::arg("n"))
        .def("writeWavefile",
            &AudioStream::writeWavefile,
            py::arg("filename"))
        .def_static("ones",
            &AudioStream::ones,
            py::arg("sample_rate"),
            py::arg("n_channels"),
            py::arg("duration"))
        .def_static("rand",
            &AudioStream::rand,
            py::arg("sample_rate"),
            py::arg("n_channels"),
            py::arg("duration"))
        .def_static("zeros",
            &AudioStream::zeros,
            py::arg("sample_rate"),
            py::arg("n_channels"),
            py::arg("duration"));

    def_copy(audio_stream);

    // Python's sequence protocol over the channels: a[i] is a channel's
    // Buffer, a[i:j] a new AudioStream and a[i] = b replaces a channel.
    // len() is the number of samples, as it was with SWIG.
    audio_stream
        .def("__len__",
            &AudioStream::getLength)
        .def("__getitem__",
            [](AudioStream & self, int64 i) -> Buffer &
            {
                return self[channel_index(self, i)];
            },
            py::arg("index"),
            py::return_value_policy::reference_internal)
        .def("__getitem__",
            [](const AudioStream & self, const py::slice & slice)
            {
                py::ssize_t start, stop, step, length;

                if(! slice.compute(self.getNChannels(), &start, &stop, &step, &length))
                {
                    throw py::error_already_set();
                }

                AudioStream out(self.getSampleRate(), static_cast<uint32>(length));

                for(py::ssize_t i = 0; i < length; ++i)
                {
                    out[static_cast<uint32>(i)] = self[static_cast<uint32>(start + i * step)];
                }

                return out;
            },
            py::arg("slice"))
        .def("__setitem__",
            [](AudioStream & self, int64 i, const Buffer & channel)
            {
                self[channel_index(self, i)] = channel;
            },
            py::arg("index"),
            py::arg("channel"))
        .def("__str__",
            [](const AudioStream & self)
            {
                std::string s = "Nsound.AudioStream(\n";

                for(uint32 i = 0; i < self.getNChannels(); ++i)
                {
                    s += "    " + buffer_str(self[i]) + ",\n";
                }

                return s + ")";
            })
        .def("__repr__",
            [](const AudioStream & self)
            {
                char text[128];

                std::snprintf(
                    text,
                    sizeof(text),
                    "Nsound.AudioStream(%.1f, %u) holding %.3f seconds (%u samples)",
                    self.getSampleRate(),
                    self.getNChannels(),
                    self.getDuration(),
                    self.getLength());

                return std::string(text);
            });

    // a << x appends to every channel: an AudioStream, a Buffer, a sample, a
    // wavefile's channels or each number in an iterable.
    audio_stream
        .def("__lshift__",
            py::overload_cast<const AudioStream &>(&AudioStream::operator<<),
            py::return_value_policy::reference_internal)
        .def("__lshift__",
            py::overload_cast<const Buffer &>(&AudioStream::operator<<),
            py::return_value_policy::reference_internal)
        .def("__lshift__",
            [](AudioStream & self, const std::string & filename) -> AudioStream &
            {
                return self << filename.c_str();
            },
            py::return_value_policy::reference_internal)
        .def("__lshift__",
            py::overload_cast<float64>(&AudioStream::operator<<),
            py::return_value_policy::reference_internal)
        .def("__lshift__",
            [](AudioStream & self, const py::iterable & samples) -> AudioStream &
            {
                for(const py::handle & x : samples)
                {
                    self << x.cast<float64>();
                }

                return self;
            },
            py::return_value_policy::reference_internal);

    // a >> x writes a wavefile, plays the channels or appends the mono
    // samples to a list.
    audio_stream
        .def("__rshift__",
            [](const AudioStream & self, const std::string & filename)
            {
                self >> filename.c_str();
            })
        .def("__rshift__",
            [](const AudioStream & self, AudioPlayback & playback)
            {
                self >> playback;
            })
        .def("__rshift__",
            [](const AudioStream & self, AudioPlaybackRt & playback)
            {
                self >> playback;
            })
        .def("__rshift__",
            [](const AudioStream & self, py::list & samples)
            {
                for(float64 x : self.getMono()[0])
                {
                    samples.append(x);
                }
            });

    // Arithmetic with AudioStreams, Buffers and numbers, ^ is the power.
    audio_stream
        .def("__iadd__",
            py::overload_cast<const AudioStream &>(&AudioStream::operator+=),
            py::return_value_policy::reference_internal)
        .def("__iadd__",
            py::overload_cast<const Buffer &>(&AudioStream::operator+=),
            py::return_value_policy::reference_internal)
        .def("__iadd__",
            py::overload_cast<float64>(&AudioStream::operator+=),
            py::return_value_policy::reference_internal)
        .def("__isub__",
            py::overload_cast<const AudioStream &>(&AudioStream::operator-=),
            py::return_value_policy::reference_internal)
        .def("__isub__",
            py::overload_cast<const Buffer &>(&AudioStream::operator-=),
            py::return_value_policy::reference_internal)
        .def("__isub__",
            py::overload_cast<float64>(&AudioStream::operator-=),
            py::return_value_policy::reference_internal)
        .def("__imul__",
            py::overload_cast<const AudioStream &>(&AudioStream::operator*=),
            py::return_value_policy::reference_internal)
        .def("__imul__",
            py::overload_cast<const Buffer &>(&AudioStream::operator*=),
            py::return_value_policy::reference_internal)
        .def("__imul__",
            py::overload_cast<float64>(&AudioStream::operator*=),
            py::return_value_policy::reference_internal)
        .def("__itruediv__",
            py::overload_cast<const AudioStream &>(&AudioStream::operator/=),
            py::return_value_policy::reference_internal)
        .def("__itruediv__",
            py::overload_cast<const Buffer &>(&AudioStream::operator/=),
            py::return_value_policy::reference_internal)
        .def("__itruediv__",
            py::overload_cast<float64>(&AudioStream::operator/=),
            py::return_value_policy::reference_internal)
        .def("__ipow__",
            py::overload_cast<const AudioStream &>(&AudioStream::operator^=),
            py::return_value_policy::reference_internal)
        .def("__ipow__",
            py::overload_cast<const Buffer &>(&AudioStream::operator^=),
            py::return_value_policy::reference_internal)
        .def("__ipow__",
            py::overload_cast<float64>(&AudioStream::operator^=),
            py::return_value_policy::reference_internal)
        .def("__ixor__",
            py::overload_cast<const AudioStream &>(&AudioStream::operator^=),
            py::return_value_policy::reference_internal)
        .def("__ixor__",
            py::overload_cast<const Buffer &>(&AudioStream::operator^=),
            py::return_value_policy::reference_internal)
        .def("__ixor__",
            py::overload_cast<float64>(&AudioStream::operator^=),
            py::return_value_policy::reference_internal)
        .def("__add__",
            [](const AudioStream & a, const AudioStream & b) { return a + b; },
            py::is_operator())
        .def("__add__",
            [](const AudioStream & a, const Buffer & b) { return a + b; },
            py::is_operator())
        .def("__add__",
            [](const AudioStream & a, float64 x) { return a + x; },
            py::is_operator())
        .def("__radd__",
            [](const AudioStream & a, float64 x) { return x + a; },
            py::is_operator())
        .def("__sub__",
            [](const AudioStream & a, const AudioStream & b) { return a - b; },
            py::is_operator())
        .def("__sub__",
            [](const AudioStream & a, const Buffer & b) { return a - b; },
            py::is_operator())
        .def("__sub__",
            [](const AudioStream & a, float64 x) { return a - x; },
            py::is_operator())
        .def("__rsub__",
            [](const AudioStream & a, float64 x) { return x - a; },
            py::is_operator())
        .def("__mul__",
            [](const AudioStream & a, const AudioStream & b) { return a * b; },
            py::is_operator())
        .def("__mul__",
            [](const AudioStream & a, const Buffer & b) { return a * b; },
            py::is_operator())
        .def("__mul__",
            [](const AudioStream & a, float64 x) { return a * x; },
            py::is_operator())
        .def("__rmul__",
            [](const AudioStream & a, float64 x) { return x * a; },
            py::is_operator())
        .def("__truediv__",
            [](const AudioStream & a, const AudioStream & b) { return a / b; },
            py::is_operator())
        .def("__truediv__",
            [](const AudioStream & a, const Buffer & b) { return a / b; },
            py::is_operator())
        .def("__truediv__",
            [](const AudioStream & a, float64 x) { return a / x; },
            py::is_operator())
        .def("__rtruediv__",
            [](const AudioStream & a, float64 x) { return x / a; },
            py::is_operator())
        .def("__pow__",
            [](const AudioStream & a, const AudioStream & b) { return a ^ b; },
            py::is_operator())
        .def("__pow__",
            [](const AudioStream & a, const Buffer & b) { return a ^ b; },
            py::is_operator())
        .def("__pow__",
            [](const AudioStream & a, float64 x) { return a ^ x; },
            py::is_operator());

    // a == other is a bool, comparing with a number gives a
    // BooleanVectorVector, which a(bvv) uses to select samples.  a(c, i) is
    // sample i of channel c.
    audio_stream
        .def("__eq__",
            [](const AudioStream & a, const AudioStream & b) { return a == b; },
            py::is_operator())
        .def("__eq__",
            [](AudioStream & a, float64 x) { return a == x; },
            py::is_operator())
        .def("__ne__",
            [](const AudioStream & a, const AudioStream & b) { return a != b; },
            py::is_operator())
        .def("__ne__",
            [](AudioStream & a, float64 x) { return a != x; },
            py::is_operator())
        .def("__gt__",
            [](AudioStream & a, float64 x) { return a > x; },
            py::is_operator())
        .def("__ge__",
            [](AudioStream & a, float64 x) { return a >= x; },
            py::is_operator())
        .def("__lt__",
            [](AudioStream & a, float64 x) { return a < x; },
            py::is_operator())
        .def("__le__",
            [](AudioStream & a, float64 x) { return a <= x; },
            py::is_operator())
        .def("__call__",
            py::overload_cast<const BooleanVectorVector &>(&AudioStream::operator()),
            py::arg("bv"),
            py::keep_alive<0, 1>())
        .def("__call__",
            py::overload_cast<uint32, uint32>(&AudioStream::operator(), py::const_),
            py::arg("channel"),
            py::arg("index"));

    // Pickling, with write() and read().
    audio_stream
        .def(py::pickle(
            [](const AudioStream & self)
            {
                return py::bytes(self.write());
            },
            [](const py::bytes & state)
            {
                std::string_view bytes = state;

                AudioStream as;
                as.read(bytes.data(), bytes.size());

                return as;
            }));
}
