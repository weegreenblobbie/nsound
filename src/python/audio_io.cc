//-----------------------------------------------------------------------------
//
//  The Python bindings for audio playback and wavefiles.
//
//-----------------------------------------------------------------------------

#include "bindings.h"

#include <Nsound/AudioBackend.h>
#include <Nsound/AudioPlayback.h>
#include <Nsound/AudioPlaybackRt.h>
#include <Nsound/AudioStream.h>
#include <Nsound/Buffer.h>
#include <Nsound/Wavefile.h>

using namespace Nsound;

void
bind_audio_io(py::module_ & m)
{
    py::class_<AudioBackend> audio_backend(m, "AudioBackend");

    py::native_enum<AudioBackend::State>(audio_backend, "State", "enum.IntEnum")
        .value("BACKEND_NOT_INITIALIZED", AudioBackend::BACKEND_NOT_INITIALIZED)
        .value("BACKEND_READY", AudioBackend::BACKEND_READY)
        .value("BACKEND_ERROR", AudioBackend::BACKEND_ERROR)
        .export_values()
        .finalize();

    audio_backend
        .def("getBackendType",
            &AudioBackend::getBackendType)
        .def("getBitsPerSample",
            &AudioBackend::getBitsPerSample)
        .def("getChannels",
            &AudioBackend::getChannels)
        .def("getError",
            &AudioBackend::getError)
        .def("getInfo",
            &AudioBackend::getInfo)
        .def("getSampleRate",
            &AudioBackend::getSampleRate)
        .def("getState",
            &AudioBackend::getState)
        .def("getStateString",
            py::overload_cast<>(&AudioBackend::getStateString))
        // Python can't overload a name with static and instance methods, the
        // static one is an instance method too.
        .def("getStateString",
            [](const AudioBackend &, const AudioBackend::State & state)
            {
                return AudioBackend::getStateString(state);
            },
            py::arg("state"))
        .def("initialize",
            &AudioBackend::initialize)
        .def("scanDevices",
            &AudioBackend::scanDevices,
            py::arg("pb"),
            py::arg("test_clip"))
        .def("setOption",
            &AudioBackend::setOption,
            py::arg("key"),
            py::arg("value"))
        .def("shutdown",
            &AudioBackend::shutdown);

    py::class_<AudioPlayback> audio_playback(m, "AudioPlayback");

    audio_playback
        .def(py::init<const float64 &, const uint32, const uint32>(),
            py::arg("sample_rate") = 44100.0,
            py::arg("channels") = 1,
            py::arg("bits_per_sample") = 16)
        .def_static("setBackendType",
            &AudioPlayback::setBackendType,
            py::arg("ab"))
        .def_static("getBackendType",
            &AudioPlayback::getBackendType)
        .def("getBitsPerSample",
            &AudioPlayback::getBitsPerSample)
        .def("setBitsPerSample",
            &AudioPlayback::setBitsPerSample,
            py::arg("bits_per_sample"))
        .def("getChannels",
            &AudioPlayback::getChannels)
        .def("setChannels",
            &AudioPlayback::setChannels,
            py::arg("channels"))
        .def("getError",
            &AudioPlayback::getError)
        .def("getInfo",
            &AudioPlayback::getInfo)
        .def("getSampleRate",
            &AudioPlayback::getSampleRate)
        .def("setSampleRate",
            &AudioPlayback::setSampleRate,
            py::arg("sample_rate"))
        .def("getState",
            &AudioPlayback::getState)
        .def("getStateString",
            py::overload_cast<>(&AudioPlayback::getStateString))
        .def("getStateString",
            py::overload_cast<const AudioBackend::State &>(&AudioPlayback::getStateString),
            py::arg("state"))
        .def("initialize",
            &AudioPlayback::initialize)
        .def("play",
            py::overload_cast<const AudioStream &>(&AudioPlayback::play),
            py::arg("a"))
        .def("play",
            py::overload_cast<const Buffer &>(&AudioPlayback::play),
            py::arg("b"))
        .def("scanDevices",
            &AudioPlayback::scanDevices)
        .def("setOption",
            &AudioPlayback::setOption,
            py::arg("key"),
            py::arg("value"))
        .def("shutdown",
            &AudioPlayback::shutdown);

    def_copy(audio_playback);

    py::class_<AudioPlaybackRtDebug> audio_playback_rt_debug(m, "AudioPlaybackRtDebug");

    audio_playback_rt_debug
        .def("__str__",
            &AudioPlaybackRtDebug::__str__)
        .def_readwrite("unknown_error_count", &AudioPlaybackRtDebug::unknown_error_count)
        .def_readwrite("overrun_count", &AudioPlaybackRtDebug::overrun_count)
        .def_readwrite("underrun_count", &AudioPlaybackRtDebug::underrun_count)
        .def_readwrite("pa_overrun_count", &AudioPlaybackRtDebug::pa_overrun_count)
        .def_readwrite("pa_underrun_count", &AudioPlaybackRtDebug::pa_underrun_count)
        .def_readwrite("pool_size", &AudioPlaybackRtDebug::pool_size)
        .def_readwrite("n_ready", &AudioPlaybackRtDebug::n_ready)
        .def_readwrite("wr_index", &AudioPlaybackRtDebug::wr_index)
        .def_readwrite("wr_ptr", &AudioPlaybackRtDebug::wr_ptr)
        .def_readwrite("rd_ptr", &AudioPlaybackRtDebug::rd_ptr)
        .def_readwrite("samplerate", &AudioPlaybackRtDebug::samplerate)
        .def_readwrite("samples_per_buffer", &AudioPlaybackRtDebug::samples_per_buffer)
        .def_readwrite("is_streaming", &AudioPlaybackRtDebug::is_streaming)
        .def_readwrite("n_history", &AudioPlaybackRtDebug::n_history);

    def_copy(audio_playback_rt_debug);

    py::class_<AudioPlaybackRt> audio_playback_rt(m, "AudioPlaybackRt");

    audio_playback_rt
        .def(py::init<float64, uint32, uint32, float64>(),
            py::arg("sample_rate") = 44100.0,
            py::arg("channels") = 1,
            py::arg("n_buffers") = 3,
            py::arg("buffer_size_sec") = - 1.0)
        .def_static("use_jack",
            &AudioPlaybackRt::use_jack,
            py::arg("flag"))
        .def_static("use_host_api",
            &AudioPlaybackRt::use_host_api,
            py::arg("name"))
        .def_static("getHostApis",
            &AudioPlaybackRt::getHostApis)
        .def("getSamplesPerBuffer",
            &AudioPlaybackRt::getSamplesPerBuffer)
        .def("getActualLatencySeconds",
            &AudioPlaybackRt::getActualLatencySeconds)
        .def("setBufferUnderrunMode",
            &AudioPlaybackRt::setBufferUnderrunMode,
            py::arg("bum"))
        .def("getInfo",
            &AudioPlaybackRt::getInfo)
        .def("play",
            py::overload_cast<const AudioStream &>(&AudioPlaybackRt::play),
            py::arg("a"))
        .def("play",
            py::overload_cast<const Buffer &>(&AudioPlaybackRt::play),
            py::arg("b"))
        .def("play",
            py::overload_cast<float64>(&AudioPlaybackRt::play),
            py::arg("sample"))
        .def("play",
            py::overload_cast<float64, float64>(&AudioPlaybackRt::play),
            py::arg("left"),
            py::arg("right"))
        .def("stop",
            &AudioPlaybackRt::stop)
        .def("debug_print",
            &AudioPlaybackRt::debug_print)
        .def("get_debug_info",
            &AudioPlaybackRt::get_debug_info);

    def_copy(audio_playback_rt);

    // pb << x and pb >> x both play x, an AudioStream, Buffer or sample.
    audio_playback_rt
        .def("__lshift__",
            py::overload_cast<const AudioStream &>(&AudioPlaybackRt::play))
        .def("__lshift__",
            py::overload_cast<const Buffer &>(&AudioPlaybackRt::play))
        .def("__lshift__",
            py::overload_cast<float64>(&AudioPlaybackRt::play))
        .def("__rshift__",
            py::overload_cast<const AudioStream &>(&AudioPlaybackRt::play))
        .def("__rshift__",
            py::overload_cast<const Buffer &>(&AudioPlaybackRt::play))
        .def("__rshift__",
            py::overload_cast<float64>(&AudioPlaybackRt::play))
        .def("__str__",
            [](AudioPlaybackRt & self)
            {
                return "Nsound.AudioPlaybackRt(): " + self.getInfo()
                    + "\n" + self.debug_print();
            })
        .def("__repr__",
            [](AudioPlaybackRt & self)
            {
                return "Nsound.AudioPlaybackRt(): " + self.getInfo()
                    + "\n" + self.debug_print();
            });

    py::class_<ID3v1Tag> id3v1_tag(m, "ID3v1Tag");

    id3v1_tag
        .def(py::init<const std::string &, boolean>(),
            py::arg("filename") = "",
            py::arg("show_warnings") = true)
        .def("read",
            &ID3v1Tag::read,
            py::arg("filename"),
            py::arg("show_warnings") = true)
        .def("write",
            &ID3v1Tag::write,
            py::arg("filename"),
            py::arg("show_warnings") = true)
        .def_readwrite("title", &ID3v1Tag::title)
        .def_readwrite("artist", &ID3v1Tag::artist)
        .def_readwrite("album", &ID3v1Tag::album)
        .def_readwrite("year", &ID3v1Tag::year)
        .def_readwrite("comment", &ID3v1Tag::comment)
        .def_readwrite("genre", &ID3v1Tag::genre);

    def_copy(id3v1_tag);

    py::class_<Wavefile> wavefile(m, "Wavefile");

    wavefile
        .def_static("decodeFormatTag",
            &Wavefile::decodeFormatTag,
            py::arg("format_tag"))
        .def_static("getDefaultSampleRate",
            &Wavefile::getDefaultSampleRate)
        .def_static("getDefaultSampleSize",
            &Wavefile::getDefaultSampleSize)
        .def_static("setDefaultSampleRate",
            py::overload_cast<const int32>(&Wavefile::setDefaultSampleRate),
            py::arg("rate"))
        .def_static("setDefaultSampleRate",
            py::overload_cast<const float64 &>(&Wavefile::setDefaultSampleRate),
            py::arg("rate"))
        .def_static("setDefaultSampleSize",
            &Wavefile::setDefaultSampleSize,
            py::arg("size"))
        .def_static("setIEEEFloat",
            &Wavefile::setIEEEFloat,
            py::arg("flag"))
        .def_static("setDefaults",
            &Wavefile::setDefaults,
            py::arg("sample_rate") = 44100.0,
            py::arg("sample_bits") = 16.0,
            py::arg("use_ieee_floats") = false)
        .def_static("read",
            py::overload_cast<const std::string &, AudioStream &>(&Wavefile::read),
            py::arg("fileName"),
            py::arg("astream"))
        .def_static("write",
            py::overload_cast<const std::string &, const AudioStream &, uint32>(&Wavefile::write),
            py::arg("fileName"),
            py::arg("as"),
            py::arg("bits_per_sample") = 16)
        .def_static("write",
            py::overload_cast<const std::string &, const Buffer &, uint32, uint32>(&Wavefile::write),
            py::arg("fileName"),
            py::arg("as"),
            py::arg("bits_per_sample"),
            py::arg("sample_rate"));

    def_copy(wavefile);

    // The wavefile format's chunk ids, format tags and sample limits, copied
    // by value: the in-class constants have no definitions to point to.
    wavefile.attr("DATA_") = uint32(Wavefile::DATA_);
    wavefile.attr("FACT_") = uint32(Wavefile::FACT_);
    wavefile.attr("FMT_") = uint32(Wavefile::FMT_);
    wavefile.attr("PEAK_") = uint32(Wavefile::PEAK_);
    wavefile.attr("RIFF_") = uint32(Wavefile::RIFF_);
    wavefile.attr("WAVE_") = uint32(Wavefile::WAVE_);
    wavefile.attr("WAVE_FORMAT_PCM_") = uint16(Wavefile::WAVE_FORMAT_PCM_);
    wavefile.attr("WAVE_FORMAT_IEEE_FLOAT_") = uint16(Wavefile::WAVE_FORMAT_IEEE_FLOAT_);
    wavefile.attr("SIGNED_64_BIT_") = raw_float64(Wavefile::SIGNED_64_BIT_);
    wavefile.attr("SIGNED_48_BIT_") = raw_float64(Wavefile::SIGNED_48_BIT_);
    wavefile.attr("SIGNED_32_BIT_") = raw_float64(Wavefile::SIGNED_32_BIT_);
    wavefile.attr("SIGNED_24_BIT_") = raw_float64(Wavefile::SIGNED_24_BIT_);
    wavefile.attr("SIGNED_16_BIT_") = raw_float64(Wavefile::SIGNED_16_BIT_);
    wavefile.attr("SIGNED_8_BIT_") = raw_float64(Wavefile::SIGNED_8_BIT_);
    wavefile.attr("UNSIGNED_64_BIT_") = raw_uint64(Wavefile::UNSIGNED_64_BIT_);
    wavefile.attr("UNSIGNED_48_BIT_") = raw_uint64(Wavefile::UNSIGNED_48_BIT_);
    wavefile.attr("UNSIGNED_32_BIT_") = raw_uint64(Wavefile::UNSIGNED_32_BIT_);
    wavefile.attr("UNSIGNED_24_BIT_") = raw_uint64(Wavefile::UNSIGNED_24_BIT_);
    wavefile.attr("UNSIGNED_16_BIT_") = raw_uint64(Wavefile::UNSIGNED_16_BIT_);
    wavefile.attr("UNSIGNED_8_BIT_") = raw_uint64(Wavefile::UNSIGNED_8_BIT_);

    m.def("use",
        py::overload_cast<const std::string &>(&use),
        py::arg("backend"),
        "Selects the audio backend AudioPlayback uses by name.");

    m.def("use",
        py::overload_cast<const AudioBackendType &>(&use),
        py::arg("type"));

    m.def("getBackends",
        &getBackends,
        "Returns the names of the audio backends compiled in.");

    m.def("getBackendTypes",
        &getBackendTypes,
        "Returns the types of the audio backends compiled in.");
}
