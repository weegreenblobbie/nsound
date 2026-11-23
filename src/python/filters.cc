//-----------------------------------------------------------------------------
//
//  The Python bindings for the filters.
//
//-----------------------------------------------------------------------------

#include "bindings.h"

#include <Nsound/AudioStream.h>
#include <Nsound/Buffer.h>
#include <Nsound/DelayLine.h>
#include <Nsound/Filter.h>
#include <Nsound/FilterAllPass.h>
#include <Nsound/FilterBandPassFIR.h>
#include <Nsound/FilterBandPassIIR.h>
#include <Nsound/FilterBandPassVocoder.h>
#include <Nsound/FilterBandRejectFIR.h>
#include <Nsound/FilterBandRejectIIR.h>
#include <Nsound/FilterCombLowPassFeedback.h>
#include <Nsound/FilterDC.h>
#include <Nsound/FilterDelay.h>
#include <Nsound/FilterFlanger.h>
#include <Nsound/FilterHighPassFIR.h>
#include <Nsound/FilterHighPassIIR.h>
#include <Nsound/FilterIIR.h>
#include <Nsound/FilterLeastSquaresFIR.h>
#include <Nsound/FilterLowPassFIR.h>
#include <Nsound/FilterLowPassIIR.h>
#include <Nsound/FilterLowPassMoogVcf.h>
#include <Nsound/FilterMedian.hpp>
#include <Nsound/FilterMovingAverage.h>
#include <Nsound/FilterParametricEqualizer.h>
#include <Nsound/FilterPhaser.h>
#include <Nsound/FilterSlinky.h>
#include <Nsound/FilterStageIIR.h>
#include <Nsound/FilterTone.h>

using namespace Nsound;

void
bind_filters(py::module_ & m)
{
    py::class_<Filter> filter(m, "Filter");

    filter
        .def("setRealtime",
            &Filter::setRealtime,
            py::arg("flag"))
        .def("filter",
            py::overload_cast<const AudioStream &>(&Filter::filter),
            py::arg("x"))
        .def("filter",
            py::overload_cast<const AudioStream &, const float64 &>(&Filter::filter),
            py::arg("x"),
            py::arg("frequency"))
        .def("filter",
            py::overload_cast<const AudioStream &, const Buffer &>(&Filter::filter),
            py::arg("x"),
            py::arg("frequencies"))
        .def("filter",
            py::overload_cast<const Buffer &>(&Filter::filter),
            py::arg("x"))
        .def("filter",
            py::overload_cast<const Buffer &, const float64 &>(&Filter::filter),
            py::arg("x"),
            py::arg("frequency"))
        .def("filter",
            py::overload_cast<const Buffer &, const Buffer &>(&Filter::filter),
            py::arg("x"),
            py::arg("frequencies"))
        .def("filter",
            py::overload_cast<const float64 &>(&Filter::filter),
            py::arg("x"))
        .def("filter",
            py::overload_cast<const float64 &, const float64 &>(&Filter::filter),
            py::arg("x"),
            py::arg("frequency"))
        .def("getFrequencyAxis",
            &Filter::getFrequencyAxis,
            py::arg("n_fft") = 8192)
        .def("getFrequencyResponse",
            &Filter::getFrequencyResponse,
            py::arg("n_fft") = 8192)
        .def("getImpulseResponse",
            &Filter::getImpulseResponse,
            py::arg("n_samples") = 8192)
        .def("getKernelSize",
            &Filter::getKernelSize)
        .def("getPhaseResponse",
            &Filter::getPhaseResponse)
        .def("getSampleRate",
            &Filter::getSampleRate)
        .def("plot",
            &Filter::plot,
            py::arg("show_phase") = false)
        .def("reset",
            &Filter::reset);

    py::class_<FilterAllPass, Filter> filter_all_pass(m, "FilterAllPass");

    filter_all_pass
        .def(py::init<const float64 &, const float64 &, const float64 &>(),
            py::arg("sample_rate"),
            py::arg("max_delay_time_seconds"),
            py::arg("gain"))
        .def("filter",
            py::overload_cast<const AudioStream &>(&FilterAllPass::filter),
            py::arg("x"))
        .def("filter",
            py::overload_cast<const AudioStream &, const float64 &>(&FilterAllPass::filter),
            py::arg("x"),
            py::arg("delay_in_seconds"))
        .def("filter",
            py::overload_cast<const AudioStream &, const Buffer &>(&FilterAllPass::filter),
            py::arg("x"),
            py::arg("delay_in_seconds"))
        .def("filter",
            py::overload_cast<const Buffer &>(&FilterAllPass::filter),
            py::arg("x"))
        .def("filter",
            py::overload_cast<const Buffer &, const float64 &>(&FilterAllPass::filter),
            py::arg("x"),
            py::arg("delay_in_seconds"))
        .def("filter",
            py::overload_cast<const Buffer &, const Buffer &>(&FilterAllPass::filter),
            py::arg("x"),
            py::arg("delay_in_seconds"))
        .def("filter",
            py::overload_cast<const float64 &>(&FilterAllPass::filter),
            py::arg("x"))
        .def("filter",
            py::overload_cast<const float64 &, const float64 &>(&FilterAllPass::filter),
            py::arg("x"),
            py::arg("delay_in_seconds"))
        .def("plot",
            &FilterAllPass::plot,
            py::arg("show_fc") = true,
            py::arg("show_phase") = false)
        .def("reset",
            &FilterAllPass::reset);

    def_copy(filter_all_pass);

    py::class_<FilterBandPassVocoder, Filter> filter_band_pass_vocoder(m, "FilterBandPassVocoder");

    filter_band_pass_vocoder
        .def(py::init<const float64 &, const float64 &, const float64 &>(),
            py::arg("sample_rate"),
            py::arg("frequency_low"),
            py::arg("frequency_high"))
        .def("filter",
            py::overload_cast<const AudioStream &>(&FilterBandPassVocoder::filter),
            py::arg("x"))
        .def("filter",
            py::overload_cast<const AudioStream &, const Buffer &>(&FilterBandPassVocoder::filter),
            py::arg("x"),
            py::arg("frequencies"))
        .def("filter",
            py::overload_cast<const AudioStream &, const Buffer &, const Buffer &>(&FilterBandPassVocoder::filter),
            py::arg("x"),
            py::arg("low_frequencies"),
            py::arg("high_frequencies"))
        .def("filter",
            py::overload_cast<const Buffer &>(&FilterBandPassVocoder::filter),
            py::arg("x"))
        .def("filter",
            py::overload_cast<const Buffer &, const Buffer &>(&FilterBandPassVocoder::filter),
            py::arg("x"),
            py::arg("frequencies"))
        .def("filter",
            py::overload_cast<const Buffer &, const Buffer &, const Buffer &>(&FilterBandPassVocoder::filter),
            py::arg("x"),
            py::arg("frequencies_Hz_low"),
            py::arg("frequencies_Hz_high"))
        .def("filter",
            py::overload_cast<const float64 &>(&FilterBandPassVocoder::filter),
            py::arg("x"))
        .def("filter",
            py::overload_cast<const float64 &, const float64 &>(&FilterBandPassVocoder::filter),
            py::arg("x"),
            py::arg("frequency_Hz"))
        .def("filter",
            py::overload_cast<const float64 &, const float64 &, const float64 &>(&FilterBandPassVocoder::filter),
            py::arg("x"),
            py::arg("frequency_Hz_low"),
            py::arg("frequency_Hz_high"))
        .def("plot",
            &FilterBandPassVocoder::plot,
            py::arg("show_fc") = true,
            py::arg("show_phase") = false)
        .def("reset",
            &FilterBandPassVocoder::reset);

    def_copy(filter_band_pass_vocoder);

    py::class_<FilterCombLowPassFeedback, Filter> filter_comb_low_pass_feedback(m, "FilterCombLowPassFeedback");

    filter_comb_low_pass_feedback
        .def(py::init<const float64 &, const float64 &, const float64 &, const float64 &>(),
            py::arg("sample_rate"),
            py::arg("delay_time_seconds"),
            py::arg("feedback_gain"),
            py::arg("low_pass_frequency_Hz"))
        .def("filter",
            py::overload_cast<const AudioStream &>(&FilterCombLowPassFeedback::filter),
            py::arg("x"))
        .def("filter",
            py::overload_cast<const AudioStream &, const Buffer &>(&FilterCombLowPassFeedback::filter),
            py::arg("x"),
            py::arg("frequencies"))
        .def("filter",
            py::overload_cast<const Buffer &>(&FilterCombLowPassFeedback::filter),
            py::arg("x"))
        .def("filter",
            py::overload_cast<const Buffer &, const Buffer &>(&FilterCombLowPassFeedback::filter),
            py::arg("x"),
            py::arg("frequencies"))
        .def("filter",
            py::overload_cast<const float64 &>(&FilterCombLowPassFeedback::filter),
            py::arg("x"))
        .def("filter",
            py::overload_cast<const float64 &, const float64 &>(&FilterCombLowPassFeedback::filter),
            py::arg("x"),
            py::arg("frequency"))
        .def("plot",
            &FilterCombLowPassFeedback::plot,
            py::arg("show_fc") = true,
            py::arg("show_phase") = false)
        .def("reset",
            &FilterCombLowPassFeedback::reset);

    def_copy(filter_comb_low_pass_feedback);

    py::class_<FilterDelay, Filter> filter_delay(m, "FilterDelay");

    filter_delay
        .def(py::init<const float64 &, const float64 &>(),
            py::arg("sample_rate"),
            py::arg("max_delay_time_seconds"))
        .def("filter",
            py::overload_cast<const AudioStream &>(&FilterDelay::filter),
            py::arg("x"))
        .def("filter",
            py::overload_cast<const AudioStream &, const float64 &>(&FilterDelay::filter),
            py::arg("x"),
            py::arg("delay_in_seconds"))
        .def("filter",
            py::overload_cast<const AudioStream &, const Buffer &>(&FilterDelay::filter),
            py::arg("x"),
            py::arg("delay_in_seconds"))
        .def("filter",
            py::overload_cast<const Buffer &>(&FilterDelay::filter),
            py::arg("x"))
        .def("filter",
            py::overload_cast<const Buffer &, const float64 &>(&FilterDelay::filter),
            py::arg("x"),
            py::arg("delay_in_seconds"))
        .def("filter",
            py::overload_cast<const Buffer &, const Buffer &>(&FilterDelay::filter),
            py::arg("x"),
            py::arg("delay_in_seconds"))
        .def("filter",
            py::overload_cast<const float64 &>(&FilterDelay::filter),
            py::arg("x"))
        .def("filter",
            py::overload_cast<const float64 &, const float64 &>(&FilterDelay::filter),
            py::arg("x"),
            py::arg("delay_in_seconds"))
        .def("reset",
            &FilterDelay::reset);

    def_copy(filter_delay);

    py::class_<FilterDC> filter_dc(m, "FilterDC");

    filter_dc
        .def(py::init<const float64 &>(),
            py::arg("feedback_gain"))
        .def("filter",
            py::overload_cast<const AudioStream &>(&FilterDC::filter),
            py::arg("x"))
        .def("filter",
            py::overload_cast<const Buffer &>(&FilterDC::filter),
            py::arg("x"))
        .def("filter",
            py::overload_cast<const float64 &>(&FilterDC::filter),
            py::arg("x"));

    def_copy(filter_dc);

    py::class_<FilterIIR, Filter> filter_iir(m, "FilterIIR");

    filter_iir
        .def(py::init<const float64 &, uint32>(),
            py::arg("sample_rate"),
            py::arg("n_poles"))
        .def("designFrequencyResponse",
            &FilterIIR::designFrequencyResponse,
            py::arg("frequency_response"),
            py::arg("max_error") = 0.01,
            py::arg("max_iterations") = 1000)
        .def("designImpulseResponse",
            &FilterIIR::designImpulseResponse,
            py::arg("impulse_response"),
            py::arg("max_error") = 0.01,
            py::arg("max_iterations") = 1000)
        .def("filter",
            py::overload_cast<const AudioStream &>(&FilterIIR::filter),
            py::arg("x"))
        .def("filter",
            py::overload_cast<const AudioStream &, const Buffer &>(&FilterIIR::filter),
            py::arg("x"),
            py::arg("frequencies"))
        .def("filter",
            py::overload_cast<const Buffer &>(&FilterIIR::filter),
            py::arg("x"))
        .def("filter",
            py::overload_cast<const Buffer &, const Buffer &>(&FilterIIR::filter),
            py::arg("x"),
            py::arg("frequencies"))
        .def("filter",
            py::overload_cast<const float64 &>(&FilterIIR::filter),
            py::arg("x"))
        .def("filter",
            py::overload_cast<const float64 &, const float64 &>(&FilterIIR::filter),
            py::arg("x"),
            py::arg("frequency"))
        .def("getKernelSize",
            &FilterIIR::getKernelSize)
        .def("getImpulseResponse",
            &FilterIIR::getImpulseResponse,
            py::arg("n_samples") = 8192)
        .def("reset",
            &FilterIIR::reset);

    def_copy(filter_iir);

    py::class_<FilterStageIIR, Filter> filter_stage_iir(m, "FilterStageIIR");

    py::native_enum<FilterStageIIR::Type>(filter_stage_iir, "Type", "enum.IntEnum")
        .value("LOW_PASS", FilterStageIIR::LOW_PASS)
        .value("HIGH_PASS", FilterStageIIR::HIGH_PASS)
        .export_values()
        .finalize();

    filter_stage_iir
        .def(py::init<FilterStageIIR::Type, const float64 &, uint32, const float64 &, const float64 &>(),
            py::arg("type"),
            py::arg("sample_rate"),
            py::arg("n_poles"),
            py::arg("frequency"),
            py::arg("percent_ripple"))
        .def("filter",
            py::overload_cast<const AudioStream &>(&FilterStageIIR::filter),
            py::arg("x"))
        .def("filter",
            py::overload_cast<const AudioStream &, const float64 &>(&FilterStageIIR::filter),
            py::arg("x"),
            py::arg("f"))
        .def("filter",
            py::overload_cast<const AudioStream &, const Buffer &>(&FilterStageIIR::filter),
            py::arg("x"),
            py::arg("frequencies"))
        .def("filter",
            py::overload_cast<const Buffer &>(&FilterStageIIR::filter),
            py::arg("x"))
        .def("filter",
            py::overload_cast<const Buffer &, const float64 &>(&FilterStageIIR::filter),
            py::arg("x"),
            py::arg("f"))
        .def("filter",
            py::overload_cast<const Buffer &, const Buffer &>(&FilterStageIIR::filter),
            py::arg("x"),
            py::arg("frequencies"))
        .def("filter",
            py::overload_cast<const float64 &>(&FilterStageIIR::filter),
            py::arg("x"))
        .def("filter",
            py::overload_cast<const float64 &, const float64 &>(&FilterStageIIR::filter),
            py::arg("x"),
            py::arg("frequency"))
        .def("makeKernel",
            &FilterStageIIR::makeKernel,
            py::arg("frequency"))
        .def("reset",
            &FilterStageIIR::reset);

    def_copy(filter_stage_iir);

    py::class_<FilterLeastSquaresFIR, Filter> filter_least_squares_fir(m, "FilterLeastSquaresFIR");

    filter_least_squares_fir
        .def(py::init<const float64 &, uint32, const Buffer &, const Buffer &, const float64 &>(),
            py::arg("sample_rate"),
            py::arg("kernel_size"),
            py::arg("freq_axis"),
            py::arg("amplitude_axis"),
            py::arg("beta") = 5.0)
        .def("getKernel",
            &FilterLeastSquaresFIR::getKernel)
        .def("setKernel",
            &FilterLeastSquaresFIR::setKernel,
            py::arg("k"))
        .def("getKernelFrequencies",
            &FilterLeastSquaresFIR::getKernelFrequencies)
        .def("getKernelAmplitudes",
            &FilterLeastSquaresFIR::getKernelAmplitudes)
        .def("filter",
            py::overload_cast<const AudioStream &>(&FilterLeastSquaresFIR::filter),
            py::arg("x"))
        .def("filter",
            py::overload_cast<const AudioStream &, const float64 &>(&FilterLeastSquaresFIR::filter),
            py::arg("x"),
            py::arg("frequency"))
        .def("filter",
            py::overload_cast<const AudioStream &, const Buffer &>(&FilterLeastSquaresFIR::filter),
            py::arg("x"),
            py::arg("frequencies"))
        .def("filter",
            py::overload_cast<const Buffer &>(&FilterLeastSquaresFIR::filter),
            py::arg("x"))
        .def("filter",
            py::overload_cast<const Buffer &, const float64 &>(&FilterLeastSquaresFIR::filter),
            py::arg("x"),
            py::arg("frequency"))
        .def("filter",
            py::overload_cast<const Buffer &, const Buffer &>(&FilterLeastSquaresFIR::filter),
            py::arg("x"),
            py::arg("frequencies"))
        .def("filter",
            py::overload_cast<const float64 &>(&FilterLeastSquaresFIR::filter),
            py::arg("x"))
        .def("filter",
            py::overload_cast<const float64 &, const float64 &>(&FilterLeastSquaresFIR::filter),
            py::arg("x"),
            py::arg("frequency_Hz"))
        .def("getImpulseResponse",
            &FilterLeastSquaresFIR::getImpulseResponse)
        .def("plot",
            &FilterLeastSquaresFIR::plot,
            py::arg("show_fc") = true,
            py::arg("show_phase") = false)
        .def("reset",
            &FilterLeastSquaresFIR::reset)
        .def("setWindow",
            &FilterLeastSquaresFIR::setWindow,
            py::arg("type"))
        .def("makeKernel",
            &FilterLeastSquaresFIR::makeKernel,
            py::arg("freq_axis"),
            py::arg("amplitude_axis"));

    def_copy(filter_least_squares_fir);

    py::class_<FilterLowPassFIR, Filter> filter_low_pass_fir(m, "FilterLowPassFIR");

    filter_low_pass_fir
        .def(py::init<const float64 &, uint32, const float64 &>(),
            py::arg("sample_rate"),
            py::arg("kernel_size"),
            py::arg("cutoff_frequency_Hz"))
        .def("filter",
            py::overload_cast<const AudioStream &>(&FilterLowPassFIR::filter),
            py::arg("x"))
        .def("filter",
            py::overload_cast<const AudioStream &, const float64 &>(&FilterLowPassFIR::filter),
            py::arg("x"),
            py::arg("frequency"))
        .def("filter",
            py::overload_cast<const AudioStream &, const Buffer &>(&FilterLowPassFIR::filter),
            py::arg("x"),
            py::arg("frequencies"))
        .def("filter",
            py::overload_cast<const Buffer &>(&FilterLowPassFIR::filter),
            py::arg("x"))
        .def("filter",
            py::overload_cast<const Buffer &, const float64 &>(&FilterLowPassFIR::filter),
            py::arg("x"),
            py::arg("frequency"))
        .def("filter",
            py::overload_cast<const Buffer &, const Buffer &>(&FilterLowPassFIR::filter),
            py::arg("x"),
            py::arg("frequencies"))
        .def("filter",
            py::overload_cast<const float64 &>(&FilterLowPassFIR::filter),
            py::arg("x"))
        .def("filter",
            py::overload_cast<const float64 &, const float64 &>(&FilterLowPassFIR::filter),
            py::arg("x"),
            py::arg("frequency_Hz"))
        .def("getFrequency",
            &FilterLowPassFIR::getFrequency)
        .def("getImpulseResponse",
            &FilterLowPassFIR::getImpulseResponse)
        .def("plot",
            &FilterLowPassFIR::plot,
            py::arg("show_fc") = true,
            py::arg("show_phase") = false)
        .def("reset",
            &FilterLowPassFIR::reset)
        .def("setCutoff",
            &FilterLowPassFIR::setCutoff,
            py::arg("fc"));

    def_copy(filter_low_pass_fir);

    py::class_<FilterLowPassIIR, FilterStageIIR> filter_low_pass_iir(m, "FilterLowPassIIR");

    filter_low_pass_iir
        .def(py::init<const float64 &, uint32, const float64 &, const float64 &>(),
            py::arg("sample_rate"),
            py::arg("n_poles"),
            py::arg("frequency"),
            py::arg("percent_ripple") = 0.0)
        .def("filter",
            py::overload_cast<const AudioStream &>(&FilterLowPassIIR::filter),
            py::arg("x"))
        .def("filter",
            py::overload_cast<const AudioStream &, const float64 &>(&FilterLowPassIIR::filter),
            py::arg("x"),
            py::arg("frequency"))
        .def("filter",
            py::overload_cast<const AudioStream &, const Buffer &>(&FilterLowPassIIR::filter),
            py::arg("x"),
            py::arg("frequencies"))
        .def("filter",
            py::overload_cast<const Buffer &>(&FilterLowPassIIR::filter),
            py::arg("x"))
        .def("filter",
            py::overload_cast<const Buffer &, const float64 &>(&FilterLowPassIIR::filter),
            py::arg("x"),
            py::arg("frequency"))
        .def("filter",
            py::overload_cast<const Buffer &, const Buffer &>(&FilterLowPassIIR::filter),
            py::arg("x"),
            py::arg("frequencies"))
        .def("filter",
            py::overload_cast<const float64 &>(&FilterLowPassIIR::filter),
            py::arg("x"))
        .def("filter",
            py::overload_cast<const float64 &, const float64 &>(&FilterLowPassIIR::filter),
            py::arg("x"),
            py::arg("frequency"))
        .def("getFrequency",
            &FilterLowPassIIR::getFrequency)
        .def("plot",
            &FilterLowPassIIR::plot,
            py::arg("show_fc") = true,
            py::arg("show_phase") = false)
        .def("reset",
            &FilterLowPassIIR::reset)
        .def("setCutoff",
            &FilterLowPassIIR::setCutoff,
            py::arg("fc"));

    def_copy(filter_low_pass_iir);

    py::class_<FilterLowPassMoogVcf, Filter> filter_low_pass_moog_vcf(m, "FilterLowPassMoogVcf");

    filter_low_pass_moog_vcf
        .def(py::init<const float64 &, const float64 &, const float64 &>(),
            py::arg("sample_rate"),
            py::arg("cutoff_hz"),
            py::arg("resonance"))
        .def("filter",
            py::overload_cast<const float64 &>(&FilterLowPassMoogVcf::filter),
            py::arg("x"))
        .def("filter",
            py::overload_cast<const float64 &, const float64 &>(&FilterLowPassMoogVcf::filter),
            py::arg("x"),
            py::arg("cutoff_hz"))
        .def("filter",
            py::overload_cast<const float64 &, const float64 &, const float64 &>(&FilterLowPassMoogVcf::filter),
            py::arg("x"),
            py::arg("cutoff_hz"),
            py::arg("resonance"))
        .def("filter",
            py::overload_cast<const AudioStream &>(&Filter::filter),
            py::arg("x"))
        .def("filter",
            py::overload_cast<const AudioStream &, const float64 &>(&Filter::filter),
            py::arg("x"),
            py::arg("frequency"))
        .def("filter",
            py::overload_cast<const AudioStream &, const Buffer &>(&Filter::filter),
            py::arg("x"),
            py::arg("frequencies"))
        .def("filter",
            py::overload_cast<const Buffer &>(&Filter::filter),
            py::arg("x"))
        .def("filter",
            py::overload_cast<const Buffer &, const float64 &>(&Filter::filter),
            py::arg("x"),
            py::arg("frequency"))
        .def("filter",
            py::overload_cast<const Buffer &, const Buffer &>(&Filter::filter),
            py::arg("x"),
            py::arg("frequencies"))
        .def("plot",
            &FilterLowPassMoogVcf::plot,
            py::arg("show_fc") = true,
            py::arg("show_phase") = false)
        .def("reset",
            &FilterLowPassMoogVcf::reset);

    def_copy(filter_low_pass_moog_vcf);

    py::class_<FilterHighPassFIR, FilterLowPassFIR> filter_high_pass_fir(m, "FilterHighPassFIR");

    filter_high_pass_fir
        .def(py::init<const float64 &, uint32, const float64 &>(),
            py::arg("sample_rate"),
            py::arg("kernel_size"),
            py::arg("cutoff_frequency_Hz"))
        .def("filter",
            py::overload_cast<const AudioStream &>(&FilterHighPassFIR::filter),
            py::arg("x"))
        .def("filter",
            py::overload_cast<const AudioStream &, const float64 &>(&FilterHighPassFIR::filter),
            py::arg("x"),
            py::arg("frequency"))
        .def("filter",
            py::overload_cast<const AudioStream &, const Buffer &>(&FilterHighPassFIR::filter),
            py::arg("x"),
            py::arg("frequencies"))
        .def("filter",
            py::overload_cast<const Buffer &>(&FilterHighPassFIR::filter),
            py::arg("x"))
        .def("filter",
            py::overload_cast<const Buffer &, const float64 &>(&FilterHighPassFIR::filter),
            py::arg("x"),
            py::arg("frequency"))
        .def("filter",
            py::overload_cast<const Buffer &, const Buffer &>(&FilterHighPassFIR::filter),
            py::arg("x"),
            py::arg("frequencies"))
        .def("filter",
            py::overload_cast<const float64 &>(&FilterHighPassFIR::filter),
            py::arg("x"))
        .def("filter",
            py::overload_cast<const float64 &, const float64 &>(&FilterHighPassFIR::filter),
            py::arg("x"),
            py::arg("frequency_Hz"))
        .def("getFrequency",
            &FilterHighPassFIR::getFrequency)
        .def("getImpulseResponse",
            &FilterHighPassFIR::getImpulseResponse)
        .def("makeKernel",
            &FilterHighPassFIR::makeKernel,
            py::arg("frequency"))
        .def("plot",
            &FilterHighPassFIR::plot,
            py::arg("show_fc") = true,
            py::arg("show_phase") = false)
        .def("reset",
            &FilterHighPassFIR::reset);

    def_copy(filter_high_pass_fir);

    py::class_<FilterHighPassIIR, FilterStageIIR> filter_high_pass_iir(m, "FilterHighPassIIR");

    filter_high_pass_iir
        .def(py::init<const float64 &, uint32, const float64 &, const float64 &>(),
            py::arg("sample_rate"),
            py::arg("n_poles"),
            py::arg("frequency"),
            py::arg("percent_ripple") = 0.0)
        .def("filter",
            py::overload_cast<const AudioStream &>(&FilterHighPassIIR::filter),
            py::arg("x"))
        .def("filter",
            py::overload_cast<const AudioStream &, const float64 &>(&FilterHighPassIIR::filter),
            py::arg("x"),
            py::arg("f"))
        .def("filter",
            py::overload_cast<const AudioStream &, const Buffer &>(&FilterHighPassIIR::filter),
            py::arg("x"),
            py::arg("frequencies"))
        .def("filter",
            py::overload_cast<const Buffer &>(&FilterHighPassIIR::filter),
            py::arg("x"))
        .def("filter",
            py::overload_cast<const Buffer &, const float64 &>(&FilterHighPassIIR::filter),
            py::arg("x"),
            py::arg("f"))
        .def("filter",
            py::overload_cast<const Buffer &, const Buffer &>(&FilterHighPassIIR::filter),
            py::arg("x"),
            py::arg("frequencies"))
        .def("filter",
            py::overload_cast<const float64 &>(&FilterHighPassIIR::filter),
            py::arg("x"))
        .def("filter",
            py::overload_cast<const float64 &, const float64 &>(&FilterHighPassIIR::filter),
            py::arg("x"),
            py::arg("frequency"))
        .def("getFrequency",
            &FilterHighPassIIR::getFrequency)
        .def("makeKernel",
            &FilterHighPassIIR::makeKernel,
            py::arg("frequency"))
        .def("plot",
            &FilterHighPassIIR::plot,
            py::arg("show_fc") = true,
            py::arg("show_phase") = false)
        .def("reset",
            &FilterHighPassIIR::reset);

    def_copy(filter_high_pass_iir);

    py::class_<FilterBandRejectFIR, FilterHighPassFIR> filter_band_reject_fir(m, "FilterBandRejectFIR");

    filter_band_reject_fir
        .def(py::init<const float64 &, uint32, const float64 &, const float64 &>(),
            py::arg("sample_rate"),
            py::arg("kernel_size"),
            py::arg("frequency_Hz_low"),
            py::arg("frequency_Hz_high"))
        .def("getFrequencyLow",
            &FilterBandRejectFIR::getFrequencyLow)
        .def("getFrequencyHigh",
            &FilterBandRejectFIR::getFrequencyHigh)
        .def("filter",
            py::overload_cast<const AudioStream &>(&FilterBandRejectFIR::filter),
            py::arg("x"))
        .def("filter",
            py::overload_cast<const AudioStream &, const float64 &, const float64 &>(&FilterBandRejectFIR::filter),
            py::arg("x"),
            py::arg("f_low"),
            py::arg("f_high"))
        .def("filter",
            py::overload_cast<const AudioStream &, const Buffer &, const Buffer &>(&FilterBandRejectFIR::filter),
            py::arg("x"),
            py::arg("low_frequencies"),
            py::arg("high_frequencies"))
        .def("filter",
            py::overload_cast<const Buffer &>(&FilterBandRejectFIR::filter),
            py::arg("x"))
        .def("filter",
            py::overload_cast<const Buffer &, const float64 &, const float64 &>(&FilterBandRejectFIR::filter),
            py::arg("x"),
            py::arg("f_low"),
            py::arg("f_high"))
        .def("filter",
            py::overload_cast<const Buffer &, const Buffer &, const Buffer &>(&FilterBandRejectFIR::filter),
            py::arg("x"),
            py::arg("frequencies_Hz_low"),
            py::arg("frequencies_Hz_high"))
        .def("filter",
            py::overload_cast<const float64 &>(&FilterBandRejectFIR::filter),
            py::arg("x"))
        .def("filter",
            py::overload_cast<const float64 &, const float64 &>(&FilterBandRejectFIR::filter),
            py::arg("x"),
            py::arg("frequency_Hz"))
        .def("filter",
            py::overload_cast<const float64 &, const float64 &, const float64 &>(&FilterBandRejectFIR::filter),
            py::arg("x"),
            py::arg("frequency_Hz_low"),
            py::arg("frequency_Hz_high"))
        .def("makeKernel",
            &FilterBandRejectFIR::makeKernel,
            py::arg("frequency_Hz_low"),
            py::arg("frequency_Hz_high"))
        .def("plot",
            &FilterBandRejectFIR::plot,
            py::arg("show_fc") = true,
            py::arg("show_phase") = false)
        .def("reset",
            &FilterBandRejectFIR::reset);

    def_copy(filter_band_reject_fir);

    py::class_<FilterBandRejectIIR, Filter> filter_band_reject_iir(m, "FilterBandRejectIIR");

    filter_band_reject_iir
        .def(py::init<const float64 &, uint32, const float64 &, const float64 &, const float64 &>(),
            py::arg("sample_rate"),
            py::arg("n_poles"),
            py::arg("frequency_Hz_low"),
            py::arg("frequency_Hz_high"),
            py::arg("percent_ripple") = 0.0)
        .def("getFrequencyLow",
            &FilterBandRejectIIR::getFrequencyLow)
        .def("getFrequencyHigh",
            &FilterBandRejectIIR::getFrequencyHigh)
        .def("filter",
            py::overload_cast<const AudioStream &>(&FilterBandRejectIIR::filter),
            py::arg("x"))
        .def("filter",
            py::overload_cast<const AudioStream &, const float64 &>(&FilterBandRejectIIR::filter),
            py::arg("x"),
            py::arg("f"))
        .def("filter",
            py::overload_cast<const AudioStream &, const Buffer &>(&FilterBandRejectIIR::filter),
            py::arg("x"),
            py::arg("frequencies"))
        .def("filter",
            py::overload_cast<const AudioStream &, const float64 &, const float64 &>(&FilterBandRejectIIR::filter),
            py::arg("x"),
            py::arg("f_low"),
            py::arg("f_high"))
        .def("filter",
            py::overload_cast<const AudioStream &, const Buffer &, const Buffer &>(&FilterBandRejectIIR::filter),
            py::arg("x"),
            py::arg("low_frequencies"),
            py::arg("high_frequencies"))
        .def("filter",
            py::overload_cast<const Buffer &>(&FilterBandRejectIIR::filter),
            py::arg("x"))
        .def("filter",
            py::overload_cast<const Buffer &, const float64 &>(&FilterBandRejectIIR::filter),
            py::arg("x"),
            py::arg("f"))
        .def("filter",
            py::overload_cast<const Buffer &, const Buffer &>(&FilterBandRejectIIR::filter),
            py::arg("x"),
            py::arg("frequencies"))
        .def("filter",
            py::overload_cast<const Buffer &, const float64 &, const float64 &>(&FilterBandRejectIIR::filter),
            py::arg("x"),
            py::arg("f_low"),
            py::arg("f_high"))
        .def("filter",
            py::overload_cast<const Buffer &, const Buffer &, const Buffer &>(&FilterBandRejectIIR::filter),
            py::arg("x"),
            py::arg("frequencies_Hz_low"),
            py::arg("frequencies_Hz_high"))
        .def("filter",
            py::overload_cast<const float64 &>(&FilterBandRejectIIR::filter),
            py::arg("x"))
        .def("filter",
            py::overload_cast<const float64 &, const float64 &>(&FilterBandRejectIIR::filter),
            py::arg("x"),
            py::arg("frequency_Hz"))
        .def("filter",
            py::overload_cast<const float64 &, const float64 &, const float64 &>(&FilterBandRejectIIR::filter),
            py::arg("x"),
            py::arg("frequency_Hz_low"),
            py::arg("frequency_Hz_high"))
        .def("plot",
            &FilterBandRejectIIR::plot,
            py::arg("show_fc") = true,
            py::arg("show_phase") = false)
        .def("reset",
            &FilterBandRejectIIR::reset);

    def_copy(filter_band_reject_iir);

    py::class_<FilterBandPassFIR, Filter> filter_band_pass_fir(m, "FilterBandPassFIR");

    filter_band_pass_fir
        .def(py::init<const float64 &, uint32, const float64 &, const float64 &>(),
            py::arg("sample_rate"),
            py::arg("kernel_size"),
            py::arg("frequency_Hz_low"),
            py::arg("frequency_Hz_high"))
        .def("getFrequencyLow",
            &FilterBandPassFIR::getFrequencyLow)
        .def("getFrequencyHigh",
            &FilterBandPassFIR::getFrequencyHigh)
        .def("filter",
            py::overload_cast<const AudioStream &>(&FilterBandPassFIR::filter),
            py::arg("x"))
        .def("filter",
            py::overload_cast<const AudioStream &, const float64 &, const float64 &>(&FilterBandPassFIR::filter),
            py::arg("x"),
            py::arg("low_frequencies"),
            py::arg("high_frequencies"))
        .def("filter",
            py::overload_cast<const AudioStream &, const Buffer &, const Buffer &>(&FilterBandPassFIR::filter),
            py::arg("x"),
            py::arg("low_frequencies"),
            py::arg("high_frequencies"))
        .def("filter",
            py::overload_cast<const Buffer &>(&FilterBandPassFIR::filter),
            py::arg("x"))
        .def("filter",
            py::overload_cast<const Buffer &, const float64 &, const float64 &>(&FilterBandPassFIR::filter),
            py::arg("x"),
            py::arg("frequencies_Hz_low"),
            py::arg("frequencies_Hz_high"))
        .def("filter",
            py::overload_cast<const Buffer &, const Buffer &, const Buffer &>(&FilterBandPassFIR::filter),
            py::arg("x"),
            py::arg("frequencies_Hz_low"),
            py::arg("frequencies_Hz_high"))
        .def("filter",
            py::overload_cast<const float64 &>(&FilterBandPassFIR::filter),
            py::arg("x"))
        .def("filter",
            py::overload_cast<const float64 &, const float64 &>(&FilterBandPassFIR::filter),
            py::arg("x"),
            py::arg("frequency_Hz"))
        .def("filter",
            py::overload_cast<const float64 &, const float64 &, const float64 &>(&FilterBandPassFIR::filter),
            py::arg("x"),
            py::arg("frequency_Hz_low"),
            py::arg("frequency_Hz_high"))
        .def("plot",
            &FilterBandPassFIR::plot,
            py::arg("show_fc") = true,
            py::arg("show_phase") = false)
        .def("reset",
            &FilterBandPassFIR::reset);

    def_copy(filter_band_pass_fir);

    py::class_<FilterBandPassIIR, Filter> filter_band_pass_iir(m, "FilterBandPassIIR");

    filter_band_pass_iir
        .def(py::init<const float64 &, uint32, const float64 &, const float64 &, const float64 &>(),
            py::arg("sample_rate"),
            py::arg("kernel_size"),
            py::arg("frequency_Hz_low"),
            py::arg("frequency_Hz_high"),
            py::arg("percent_ripple") = 0.0)
        .def("getFrequencyLow",
            &FilterBandPassIIR::getFrequencyLow)
        .def("getFrequencyHigh",
            &FilterBandPassIIR::getFrequencyHigh)
        .def("filter",
            py::overload_cast<const AudioStream &>(&FilterBandPassIIR::filter),
            py::arg("x"))
        .def("filter",
            py::overload_cast<const AudioStream &, const float64 &>(&FilterBandPassIIR::filter),
            py::arg("x"),
            py::arg("f"))
        .def("filter",
            py::overload_cast<const AudioStream &, const Buffer &>(&FilterBandPassIIR::filter),
            py::arg("x"),
            py::arg("frequencies"))
        .def("filter",
            py::overload_cast<const AudioStream &, const float64 &, const float64 &>(&FilterBandPassIIR::filter),
            py::arg("x"),
            py::arg("f_low"),
            py::arg("f_high"))
        .def("filter",
            py::overload_cast<const AudioStream &, const Buffer &, const Buffer &>(&FilterBandPassIIR::filter),
            py::arg("x"),
            py::arg("low_frequencies"),
            py::arg("high_frequencies"))
        .def("filter",
            py::overload_cast<const Buffer &>(&FilterBandPassIIR::filter),
            py::arg("x"))
        .def("filter",
            py::overload_cast<const Buffer &, const float64 &>(&FilterBandPassIIR::filter),
            py::arg("x"),
            py::arg("f"))
        .def("filter",
            py::overload_cast<const Buffer &, const Buffer &>(&FilterBandPassIIR::filter),
            py::arg("x"),
            py::arg("frequencies"))
        .def("filter",
            py::overload_cast<const Buffer &, const float64 &, const float64 &>(&FilterBandPassIIR::filter),
            py::arg("x"),
            py::arg("f_low"),
            py::arg("f_high"))
        .def("filter",
            py::overload_cast<const Buffer &, const Buffer &, const Buffer &>(&FilterBandPassIIR::filter),
            py::arg("x"),
            py::arg("frequencies_Hz_low"),
            py::arg("frequencies_Hz_high"))
        .def("filter",
            py::overload_cast<const float64 &>(&FilterBandPassIIR::filter),
            py::arg("x"))
        .def("filter",
            py::overload_cast<const float64 &, const float64 &>(&FilterBandPassIIR::filter),
            py::arg("x"),
            py::arg("frequency_Hz"))
        .def("filter",
            py::overload_cast<const float64 &, const float64 &, const float64 &>(&FilterBandPassIIR::filter),
            py::arg("x"),
            py::arg("frequency_Hz_low"),
            py::arg("frequency_Hz_high"))
        .def("plot",
            &FilterBandPassIIR::plot,
            py::arg("show_fc") = true,
            py::arg("show_phase") = false)
        .def("reset",
            &FilterBandPassIIR::reset);

    def_copy(filter_band_pass_iir);

    py::class_<FilterMedian> filter_median(m, "FilterMedian");

    filter_median
        .def(py::init<uint32>(),
            py::arg("n_samples_in_pool"))
        .def("filter",
            py::overload_cast<const AudioStream &>(&FilterMedian::filter),
            py::arg("x"))
        .def("filter",
            py::overload_cast<const Buffer &>(&FilterMedian::filter),
            py::arg("b"))
        .def("filter",
            py::overload_cast<const float64 &>(&FilterMedian::filter),
            py::arg("x"))
        .def("fill",
            &FilterMedian::fill,
            py::arg("x"));

    def_copy(filter_median);

    py::class_<FilterMovingAverage, Filter> filter_moving_average(m, "FilterMovingAverage");

    filter_moving_average
        .def(py::init<uint32>(),
            py::arg("n_samples_to_average"))
        .def("filter",
            py::overload_cast<const AudioStream &>(&FilterMovingAverage::filter),
            py::arg("x"))
        .def("filter",
            py::overload_cast<const AudioStream &, const Buffer &>(&FilterMovingAverage::filter),
            py::arg("x"),
            py::arg("frequencies"))
        .def("filter",
            py::overload_cast<const Buffer &>(&FilterMovingAverage::filter),
            py::arg("x"))
        .def("filter",
            py::overload_cast<const Buffer &, const Buffer &>(&FilterMovingAverage::filter),
            py::arg("x"),
            py::arg("frequencies"))
        .def("filter",
            py::overload_cast<const float64 &>(&FilterMovingAverage::filter),
            py::arg("x"))
        .def("filter",
            py::overload_cast<const float64 &, const float64 &>(&FilterMovingAverage::filter),
            py::arg("x"),
            py::arg("frequency"))
        .def("reset",
            &FilterMovingAverage::reset);

    def_copy(filter_moving_average);

    py::class_<FilterFlanger, Filter> filter_flanger(m, "FilterFlanger");

    filter_flanger
        .def(py::init<const float64 &, const float64 &, const float64 &>(),
            py::arg("sample_rate"),
            py::arg("frequency"),
            py::arg("max_delay_time_seconds"))
        .def("filter",
            py::overload_cast<const AudioStream &>(&FilterFlanger::filter),
            py::arg("x"))
        .def("filter",
            py::overload_cast<const AudioStream &, const float64 &>(&FilterFlanger::filter),
            py::arg("x"),
            py::arg("frequency"))
        .def("filter",
            py::overload_cast<const AudioStream &, const float64 &, const float64 &>(&FilterFlanger::filter),
            py::arg("x"),
            py::arg("frequency"),
            py::arg("delay"))
        .def("filter",
            py::overload_cast<const AudioStream &, const Buffer &, const Buffer &>(&FilterFlanger::filter),
            py::arg("x"),
            py::arg("frequency"),
            py::arg("delay"))
        .def("filter",
            py::overload_cast<const Buffer &>(&FilterFlanger::filter),
            py::arg("x"))
        .def("filter",
            py::overload_cast<const Buffer &, const float64 &>(&FilterFlanger::filter),
            py::arg("x"),
            py::arg("frequency"))
        .def("filter",
            py::overload_cast<const Buffer &, const float64 &, const float64 &>(&FilterFlanger::filter),
            py::arg("x"),
            py::arg("frequency"),
            py::arg("delay"))
        .def("filter",
            py::overload_cast<const Buffer &, const Buffer &, const Buffer &>(&FilterFlanger::filter),
            py::arg("x"),
            py::arg("frequency"),
            py::arg("delay"))
        .def("filter",
            py::overload_cast<const float64 &>(&FilterFlanger::filter),
            py::arg("x"))
        .def("filter",
            py::overload_cast<const float64 &, const float64 &>(&FilterFlanger::filter),
            py::arg("x"),
            py::arg("frequency"))
        .def("filter",
            py::overload_cast<const float64 &, const float64 &, const float64 &>(&FilterFlanger::filter),
            py::arg("x"),
            py::arg("frequency"),
            py::arg("delay"))
        .def("plot",
            &FilterFlanger::plot,
            py::arg("show_fc") = false,
            py::arg("show_phase") = false)
        .def("reset",
            &FilterFlanger::reset);

    def_copy(filter_flanger);

    py::class_<FilterParametricEqualizer, Filter> filter_parametric_equalizer(m, "FilterParametricEqualizer");

    py::native_enum<FilterParametricEqualizer::Type>(filter_parametric_equalizer, "Type", "enum.IntEnum")
        .value("PEAKING", FilterParametricEqualizer::PEAKING)
        .value("LOW_SHELF", FilterParametricEqualizer::LOW_SHELF)
        .value("HIGH_SHELF", FilterParametricEqualizer::HIGH_SHELF)
        .export_values()
        .finalize();

    filter_parametric_equalizer
        .def(py::init<const FilterParametricEqualizer::Type &, const float64 &, const float64 &, const float64 &, const float64 &>(),
            py::arg("type"),
            py::arg("sample_rate"),
            py::arg("frequency"),
            py::arg("resonance") = 0.707106781187,
            py::arg("gain") = 1.0)
        .def("filter",
            py::overload_cast<const AudioStream &>(&FilterParametricEqualizer::filter),
            py::arg("x"))
        .def("filter",
            py::overload_cast<const AudioStream &, const Buffer &>(&FilterParametricEqualizer::filter),
            py::arg("x"),
            py::arg("frequencies"))
        .def("filter",
            py::overload_cast<const AudioStream &, const Buffer &, const Buffer &>(&FilterParametricEqualizer::filter),
            py::arg("x"),
            py::arg("frequencies"),
            py::arg("resonance"))
        .def("filter",
            py::overload_cast<const AudioStream &, const Buffer &, const Buffer &, const Buffer &>(&FilterParametricEqualizer::filter),
            py::arg("x"),
            py::arg("frequencies"),
            py::arg("resonance"),
            py::arg("gain"))
        .def("filter",
            py::overload_cast<const Buffer &>(&FilterParametricEqualizer::filter),
            py::arg("x"))
        .def("filter",
            py::overload_cast<const Buffer &, const Buffer &>(&FilterParametricEqualizer::filter),
            py::arg("x"),
            py::arg("frequencies"))
        .def("filter",
            py::overload_cast<const Buffer &, const Buffer &, const Buffer &>(&FilterParametricEqualizer::filter),
            py::arg("x"),
            py::arg("frequencies"),
            py::arg("resonance"))
        .def("filter",
            py::overload_cast<const Buffer &, const Buffer &, const Buffer &, const Buffer &>(&FilterParametricEqualizer::filter),
            py::arg("x"),
            py::arg("frequencies"),
            py::arg("resonance"),
            py::arg("gain"))
        .def("filter",
            py::overload_cast<const float64 &>(&FilterParametricEqualizer::filter),
            py::arg("x"))
        .def("filter",
            py::overload_cast<const float64 &, const float64 &>(&FilterParametricEqualizer::filter),
            py::arg("x"),
            py::arg("frequency"))
        .def("filter",
            py::overload_cast<const float64 &, const float64 &, const float64 &>(&FilterParametricEqualizer::filter),
            py::arg("x"),
            py::arg("frequency"),
            py::arg("resonance"))
        .def("filter",
            py::overload_cast<const float64 &, const float64 &, const float64 &, const float64 &>(&FilterParametricEqualizer::filter),
            py::arg("x"),
            py::arg("frequency"),
            py::arg("resonance"),
            py::arg("gain"))
        .def("makeKernel",
            &FilterParametricEqualizer::makeKernel,
            py::arg("frequency"),
            py::arg("resonance"),
            py::arg("gain"))
        .def("plot",
            &FilterParametricEqualizer::plot,
            py::arg("show_fc") = true,
            py::arg("show_phase") = false)
        .def("reset",
            &FilterParametricEqualizer::reset);

    def_copy(filter_parametric_equalizer);

    py::class_<FilterPhaser, Filter> filter_phaser(m, "FilterPhaser");

    filter_phaser
        .def(py::init<const float64 &, const uint32, const float64 &, const float64 &, const float64 &>(),
            py::arg("sample_rate"),
            py::arg("n_stages"),
            py::arg("frequency"),
            py::arg("frequency_step_per_stage"),
            py::arg("max_delay_in_seconds"))
        .def("filter",
            py::overload_cast<const AudioStream &>(&FilterPhaser::filter),
            py::arg("x"))
        .def("filter",
            py::overload_cast<const Buffer &>(&FilterPhaser::filter),
            py::arg("x"))
        .def("filter",
            py::overload_cast<const float64 &>(&FilterPhaser::filter),
            py::arg("x"))
        .def("filter",
            py::overload_cast<const float64 &, const float64 &>(&FilterPhaser::filter),
            py::arg("x"),
            py::arg("ignored"))
        .def("plot",
            &FilterPhaser::plot,
            py::arg("show_fc") = false,
            py::arg("show_phase") = false)
        .def("reset",
            &FilterPhaser::reset);

    def_copy(filter_phaser);

    py::class_<FilterSlinky, Filter> filter_slinky(m, "FilterSlinky");

    filter_slinky
        .def(py::init<const float64 &, const float64 &, const float64 &>(),
            py::arg("sample_rate"),
            py::arg("delay_time"),
            py::arg("frequency_window"))
        .def("filter",
            py::overload_cast<const AudioStream &>(&FilterSlinky::filter),
            py::arg("x"))
        .def("filter",
            py::overload_cast<const AudioStream &, const Buffer &>(&FilterSlinky::filter),
            py::arg("x"),
            py::arg("frequencies"))
        .def("filter",
            py::overload_cast<const Buffer &>(&FilterSlinky::filter),
            py::arg("x"))
        .def("filter",
            py::overload_cast<const Buffer &, const Buffer &>(&FilterSlinky::filter),
            py::arg("x"),
            py::arg("frequencies"))
        .def("filter",
            py::overload_cast<const float64 &>(&FilterSlinky::filter),
            py::arg("x"))
        .def("filter",
            py::overload_cast<const float64 &, const float64 &>(&FilterSlinky::filter),
            py::arg("x"),
            py::arg("frequency"))
        .def("reset",
            &FilterSlinky::reset);

    def_copy(filter_slinky);

    py::class_<FilterTone, Filter> filter_tone(m, "FilterTone");

    filter_tone
        .def(py::init<const float64 &, const float64 &>(),
            py::arg("sample_rate"),
            py::arg("half_power_frequency"))
        .def("filter",
            py::overload_cast<const AudioStream &>(&FilterTone::filter),
            py::arg("x"))
        .def("filter",
            py::overload_cast<const AudioStream &, const Buffer &>(&FilterTone::filter),
            py::arg("x"),
            py::arg("hp_frequencies"))
        .def("filter",
            py::overload_cast<const Buffer &>(&FilterTone::filter),
            py::arg("x"))
        .def("filter",
            py::overload_cast<const Buffer &, const Buffer &>(&FilterTone::filter),
            py::arg("x"),
            py::arg("hp_frequencies"))
        .def("filter",
            py::overload_cast<const float64 &>(&FilterTone::filter),
            py::arg("x"))
        .def("filter",
            py::overload_cast<const float64 &, const float64 &>(&FilterTone::filter),
            py::arg("x"),
            py::arg("hp_frequency"))
        .def("getFrequency",
            &FilterTone::getFrequency)
        .def("getKernelSize",
            &FilterTone::getKernelSize)
        .def("plot",
            &FilterTone::plot,
            py::arg("show_fc") = true,
            py::arg("show_phase") = false)
        .def("reset",
            &FilterTone::reset)
        .def("makeKernel",
            &FilterTone::makeKernel,
            py::arg("half_power_frequency"));

    def_copy(filter_tone);

    py::class_<DelayLine> delay_line(m, "DelayLine");

    delay_line
        .def(py::init<float64, float64>(),
            py::arg("sample_rate"),
            py::arg("max_delay_in_seconds"))
        .def("setRealtime",
            &DelayLine::setRealtime,
            py::arg("flag"))
        .def("delay",
            py::overload_cast<const Buffer &, const Buffer &>(&DelayLine::delay),
            py::arg("x"),
            py::arg("delay_time"))
        .def("delay",
            py::overload_cast<float64, float64>(&DelayLine::delay),
            py::arg("x"),
            py::arg("delay_time"))
        .def("read",
            py::overload_cast<>(&DelayLine::read))
        .def("read",
            py::overload_cast<float64>(&DelayLine::read),
            py::arg("delay"))
        .def("write",
            &DelayLine::write,
            py::arg("x"))
        .def("reset",
            &DelayLine::reset);

    def_copy(delay_line);
}
