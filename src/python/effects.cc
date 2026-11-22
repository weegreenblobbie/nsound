//-----------------------------------------------------------------------------
//
//  The Python bindings for the FFT, spectrogram, effects and plotting.
//
//-----------------------------------------------------------------------------

#include "bindings.h"

#include <Nsound/AudioStream.h>
#include <Nsound/Buffer.h>
#include <Nsound/FFTChunk.h>
#include <Nsound/FFTransform.h>
#include <Nsound/Granulator.h>
#include <Nsound/Plotter.h>
#include <Nsound/ReverberationRoom.h>
#include <Nsound/Spectrogram.h>
#include <Nsound/Stretcher.h>
#include <Nsound/TicToc.h>
#include <Nsound/Utils.h>
#include <Nsound/Vocoder.h>

using namespace Nsound;

void
bind_effects(py::module_ & m)
{
    py::class_<FFTChunk> fft_chunk(m, "FFTChunk");

    fft_chunk
        .def(py::init<uint32, uint32, uint32>(),
            py::arg("size") = 32,
            py::arg("sample_rate") = 44100,
            py::arg("original_size") = 0)
        .def("getFrequencyAxis",
            &FFTChunk::getFrequencyAxis)
        .def("getReal",
            &FFTChunk::getReal)
        .def("getImaginary",
            &FFTChunk::getImaginary)
        .def("getMagnitude",
            &FFTChunk::getMagnitude)
        .def("getOriginalSize",
            &FFTChunk::getOriginalSize)
        .def("getPhase",
            &FFTChunk::getPhase)
        .def("isPolar",
            &FFTChunk::isPolar)
        .def("plot",
            &FFTChunk::plot,
            py::arg("title") = "",
            py::arg("dB") = true,
            py::arg("show_phase") = false)
        .def("setCartesian",
            &FFTChunk::setCartesian,
            py::arg("real"),
            py::arg("imaginary"))
        .def("setPolar",
            &FFTChunk::setPolar,
            py::arg("magnitude"),
            py::arg("phase"))
        .def("toPolar",
            &FFTChunk::toPolar)
        .def("toCartesian",
            &FFTChunk::toCartesian)
        .def("getSampleRate",
            &FFTChunk::getSampleRate)
        .def_property_readonly("real_",
            [](FFTChunk & self) -> Buffer &
            {
                return *self.real_;
            },
            py::return_value_policy::reference_internal)
        .def_property_readonly("imag_",
            [](FFTChunk & self) -> Buffer &
            {
                return *self.imag_;
            },
            py::return_value_policy::reference_internal);

    def_copy(fft_chunk);

    py::class_<FFTransform> ff_transform(m, "FFTransform");

    ff_transform
        .def(py::init<const float64 &>(),
            py::arg("sample_rate"))
        .def("fft",
            py::overload_cast<const Buffer &>(&FFTransform::fft, py::const_),
            py::arg("time_domain"))
        .def("fft",
            py::overload_cast<const Buffer &, int32, int32>(&FFTransform::fft, py::const_),
            py::arg("input"),
            py::arg("n_order"),
            py::arg("n_overlap") = 0)
        .def("ifft",
            py::overload_cast<const FFTChunkVector &>(&FFTransform::ifft, py::const_),
            py::arg("input"))
        .def("ifft",
            py::overload_cast<const Buffer &>(&FFTransform::ifft, py::const_),
            py::arg("frequency_domain"))
        .def_static("roundUp2",
            &FFTransform::roundUp2,
            py::arg("raw"))
        .def("setWindow",
            &FFTransform::setWindow,
            py::arg("type"));

    def_copy(ff_transform);

    py::class_<Spectrogram> spectrogram(m, "Spectrogram");

    spectrogram
        .def(py::init<const Buffer &, const float64 &, const float64 &, const float64 &, const WindowType &>(),
            py::arg("x"),
            py::arg("sample_rate"),
            py::arg("time_window"),
            py::arg("time_step"),
            py::arg("type"))
        .def("getFrequencyAxis",
            &Spectrogram::getFrequencyAxis)
        .def("getMagnitude",
            &Spectrogram::getMagnitude)
        .def("getTimeAxis",
            &Spectrogram::getTimeAxis)
        .def("plot",
            &Spectrogram::plot,
            py::arg("title") = "",
            py::arg("use_dB") = true,
            py::arg("squash") = 0.5)
        .def("computeMagnitude",
            &Spectrogram::computeMagnitude,
            py::arg("x"));

    def_copy(spectrogram);

    py::class_<Stretcher> stretcher(m, "Stretcher");

    stretcher
        .def(py::init<const float64 &, const float64 &, const float64 &>(),
            py::arg("sample_rate"),
            py::arg("window_size_seconds") = 0.08,
            py::arg("max_delta_window") = 0.25)
        .def("pitchShift",
            py::overload_cast<const AudioStream &, const float64 &>(&Stretcher::pitchShift),
            py::arg("x"),
            py::arg("factor"))
        .def("pitchShift",
            py::overload_cast<const AudioStream &, const Buffer &>(&Stretcher::pitchShift),
            py::arg("x"),
            py::arg("factor"))
        .def("pitchShift",
            py::overload_cast<const Buffer &, const float64 &>(&Stretcher::pitchShift),
            py::arg("x"),
            py::arg("factor"))
        .def("pitchShift",
            py::overload_cast<const Buffer &, const Buffer &>(&Stretcher::pitchShift),
            py::arg("x"),
            py::arg("factor"))
        .def("showProgress",
            &Stretcher::showProgress,
            py::arg("flag"))
        .def("timeShift",
            py::overload_cast<const AudioStream &, const float64 &>(&Stretcher::timeShift),
            py::arg("x"),
            py::arg("factor"))
        .def("timeShift",
            py::overload_cast<const AudioStream &, const Buffer &>(&Stretcher::timeShift),
            py::arg("x"),
            py::arg("factor"))
        .def("timeShift",
            py::overload_cast<const Buffer &, const float64 &>(&Stretcher::timeShift),
            py::arg("x"),
            py::arg("factor"))
        .def("timeShift",
            py::overload_cast<const Buffer &, const Buffer &>(&Stretcher::timeShift),
            py::arg("x"),
            py::arg("factor"));

    def_copy(stretcher);

    py::class_<Granulator> granulator(m, "Granulator");

    py::native_enum<Granulator::EnvelopeType_e>(granulator, "EnvelopeType_e", "enum.IntEnum")
        .value("CUSTOM", Granulator::CUSTOM)
        .value("GAUSSIAN", Granulator::GAUSSIAN)
        .value("GAUSSIAN_90", Granulator::GAUSSIAN_90)
        .value("GAUSSIAN_70", Granulator::GAUSSIAN_70)
        .value("GAUSSIAN_50", Granulator::GAUSSIAN_50)
        .value("GAUSSIAN_30", Granulator::GAUSSIAN_30)
        .value("GAUSSIAN_10", Granulator::GAUSSIAN_10)
        .value("DECAY", Granulator::DECAY)
        .value("REVERSE_DECAY", Granulator::REVERSE_DECAY)
        .export_values()
        .finalize();

    granulator
        .def(py::init<const float64 &, Granulator::EnvelopeType_e, const float64 &, const Buffer *>(),
            py::arg("sample_rate"),
            py::arg("env_type"),
            py::arg("envelope_noise") = 0.0,
            py::arg("custom_envelope") = nullptr)
        .def("generate",
            py::overload_cast<const float64 &, const float64 &, const float64 &, const float64 &>(&Granulator::generate),
            py::arg("duration"),
            py::arg("grain_frequency"),
            py::arg("waves_per_grain"),
            py::arg("grains_per_second"))
        .def("generate",
            py::overload_cast<const float64 &, const Nsound::Buffer &, const Nsound::Buffer &, const Nsound::Buffer &>(&Granulator::generate, py::const_),
            py::arg("duration"),
            py::arg("grain_frequency"),
            py::arg("waves_per_grain"),
            py::arg("grains_per_second"));

    def_copy(granulator);

    py::class_<Vocoder> vocoder(m, "Vocoder");

    vocoder
        .def(py::init<const float64 &, const float64 &, const uint32 &, const float64 &, const bool, const bool>(),
            py::arg("sample_rate"),
            py::arg("window_length"),
            py::arg("n_bands"),
            py::arg("freq_max") = 4000.0,
            py::arg("use_mel_scale") = true,
            py::arg("plot_filter_bank") = false)
        .def("filter",
            py::overload_cast<const Buffer &, const Buffer &>(&Vocoder::filter),
            py::arg("voice"),
            py::arg("carrier"))
        .def("filter",
            py::overload_cast<const float64 &, const float64 &>(&Vocoder::filter),
            py::arg("voice"),
            py::arg("carrier"))
        .def("getSampleRate",
            &Vocoder::getSampleRate)
        .def("reset",
            &Vocoder::reset);

    def_copy(vocoder);

    py::class_<ReverberationRoom> reverberation_room(m, "ReverberationRoom");

    reverberation_room
        .def(py::init<const float64 &, const float64 &, const float64 &, const float64 &, const float64 &, const float64 &>(),
            py::arg("sample_rate"),
            py::arg("room_feedback"),
            py::arg("wet_percent") = 0.5,
            py::arg("dry_percent") = 1.0,
            py::arg("low_pass_frequency_Hz") = 7644.9,
            py::arg("stereo_spread_seconds") = 0.0005215)
        .def("filter",
            py::overload_cast<const AudioStream &>(&ReverberationRoom::filter),
            py::arg("x"))
        .def("filter",
            py::overload_cast<const Buffer &>(&ReverberationRoom::filter),
            py::arg("x"))
        .def("getSampleRate",
            &ReverberationRoom::getSampleRate)
        .def("reset",
            &ReverberationRoom::reset);

    def_copy(reverberation_room);

    py::class_<Axes> axes(m, "Axes");

    axes
        .def("get_axes",
            [](Axes & self) -> py::object
            {
                if(self.get_axes() == nullptr) return py::none();

                return py::reinterpret_borrow<py::object>(self.get_axes());
            },
            "Returns the matplotlib Axes.");

    def_copy(axes);

    py::class_<Plotter> plotter(m, "Plotter");

    plotter
        .def(py::init<>())
        .def("axhline",
            &Plotter::axhline,
            py::arg("y_pos") = 0.0,
            py::arg("kwargs") = "")
        .def("axvline",
            &Plotter::axvline,
            py::arg("x_pos") = 0.0,
            py::arg("kwargs") = "")
        .def("xlim",
            &Plotter::xlim,
            py::arg("xmin"),
            py::arg("xmax"))
        .def("ylim",
            &Plotter::ylim,
            py::arg("ymin"),
            py::arg("ymax"))
        .def("figure",
            &Plotter::figure,
            py::arg("kwargs") = "")
        .def("grid",
            &Plotter::grid,
            py::arg("flag"))
        .def("hide",
            &Plotter::hide)
        .def("imagesc",
            py::overload_cast<const AudioStream &, const std::string &>(&Plotter::imagesc),
            py::arg("Z"),
            py::arg("kwargs") = "")
        .def("imagesc",
            py::overload_cast<const Buffer &, const Buffer &, const AudioStream &, const std::string &>(&Plotter::imagesc),
            py::arg("x_axis"),
            py::arg("y_axis"),
            py::arg("Z"),
            py::arg("kwargs") = "")
        .def("legend",
            &Plotter::legend,
            py::arg("kwargs") = "")
        .def("plot",
            py::overload_cast<const Buffer &, const std::string &, const std::string &>(&Plotter::plot),
            py::arg("y"),
            py::arg("fmt") = "",
            py::arg("kwargs") = "")
        .def("plot",
            py::overload_cast<const Buffer &, const Buffer &, const std::string &, const std::string &>(&Plotter::plot),
            py::arg("x"),
            py::arg("y"),
            py::arg("fmt") = "",
            py::arg("kwargs") = "")
        .def("plot",
            py::overload_cast<const float64 &, const float64 &, const std::string &, const std::string &>(&Plotter::plot),
            py::arg("x"),
            py::arg("y"),
            py::arg("fmt") = "",
            py::arg("kwargs") = "")
        .def("run_string",
            &Plotter::run_string,
            py::arg("command"))
        .def("savefig",
            &Plotter::savefig,
            py::arg("filename"),
            py::arg("kwargs") = "")
        .def("set_xscale",
            &Plotter::set_xscale,
            py::arg("s"))
        .def("set_yscale",
            &Plotter::set_yscale,
            py::arg("s"))
        .def("subplot",
            &Plotter::subplot,
            py::arg("n_rows"),
            py::arg("n_cols"),
            py::arg("n"),
            py::arg("kwargs") = "",
            py::arg("sharex") = nullptr,
            py::arg("sharey") = nullptr)
        .def("text",
            &Plotter::text,
            py::arg("x"),
            py::arg("y"),
            py::arg("text"),
            py::arg("kwargs") = "")
        .def("title",
            &Plotter::title,
            py::arg("title"),
            py::arg("kwargs") = "")
        .def("xlabel",
            &Plotter::xlabel,
            py::arg("label"),
            py::arg("kwargs") = "")
        .def("ylabel",
            &Plotter::ylabel,
            py::arg("label"),
            py::arg("kwargs") = "")
        .def_static("show",
            &Plotter::show);

    def_copy(plotter);

    m.def("Tic",
        &Tic,
        "Starts the timer Toc() reads.");

    m.def("Toc",
        &Toc,
        "Returns the processor seconds, from clock(), since Tic().");

    m.def("getFrequencyOffset",
        &getFrequencyOffset,
        py::arg("f1"),
        py::arg("octaves") = 0.0,
        py::arg("cents") = 0.0,
        "Returns the frequency a number of octaves and cents from f1.");
}
