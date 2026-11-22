//-----------------------------------------------------------------------------
//
//  The Python bindings for the generators, envelopes and random numbers.
//
//-----------------------------------------------------------------------------

#include "bindings.h"

#include <Nsound/AudioStream.h>
#include <Nsound/Buffer.h>
#include <Nsound/Cosine.h>
#include <Nsound/EnvelopeAdsr.h>
#include <Nsound/Generator.h>
#include <Nsound/GeneratorDecay.h>
#include <Nsound/Pluck.h>
#include <Nsound/Pulse.h>
#include <Nsound/RandomNumberGenerator.h>
#include <Nsound/RngTausworthe.h>
#include <Nsound/Sawtooth.h>
#include <Nsound/Sine.h>
#include <Nsound/Square.h>
#include <Nsound/Triangle.h>

using namespace Nsound;

void
bind_generators(py::module_ & m)
{
    py::class_<RandomNumberGenerator> random_number_generator(m, "RandomNumberGenerator");

    random_number_generator
        .def("get",
            py::overload_cast<>(&RandomNumberGenerator::get))
        .def("get",
            py::overload_cast<const float64 &, const float64 &>(&RandomNumberGenerator::get),
            py::arg("min"),
            py::arg("max"))
        .def("setSeed",
            &RandomNumberGenerator::setSeed,
            py::arg("seed"));

    py::class_<RngTausworthe, RandomNumberGenerator> rng_tausworthe(m, "RngTausworthe");

    rng_tausworthe
        .def(py::init<>())
        .def("get",
            py::overload_cast<>(&RngTausworthe::get))
        .def("get",
            py::overload_cast<const Nsound::int32, const Nsound::int32>(&RngTausworthe::get),
            py::arg("min"),
            py::arg("max"))
        .def("get",
            py::overload_cast<const Nsound::float64 &, const Nsound::float64 &>(&RngTausworthe::get),
            py::arg("min"),
            py::arg("max"))
        .def("setSeed",
            &RngTausworthe::setSeed,
            py::arg("seed"));

    def_copy(rng_tausworthe);

    py::class_<Generator> generator(m, "Generator");

    generator
        .def(py::init<const float64 &>(),
            py::arg("sample_rate"))
        .def(py::init<const float64 &, const Buffer &>(),
            py::arg("sample_rate"),
            py::arg("waveform"))
        .def(py::init<const std::string &>(),
            py::arg("wave_filename"))
        .def("setRealtime",
            &Generator::setRealtime,
            py::arg("flag"))
        .def("addSlaveSync",
            &Generator::addSlaveSync,
            py::arg("slave"))
        .def("buzzInit",
            &Generator::buzzInit,
            py::arg("max_harmonics"))
        .def("buzz",
            py::overload_cast<const float64 &, const float64 &, const float64 &>(&Generator::buzz),
            py::arg("frequency"),
            py::arg("n_harmonics"),
            py::arg("delay"))
        .def("buzz",
            py::overload_cast<const float64 &, const float64 &, const float64 &, const float64 &>(&Generator::buzz),
            py::arg("duration"),
            py::arg("frequency"),
            py::arg("n_harmonics"),
            py::arg("delay"))
        .def("buzz",
            py::overload_cast<const float64 &, const Buffer &, const Buffer &, const Buffer &>(&Generator::buzz),
            py::arg("duration"),
            py::arg("frequency"),
            py::arg("n_harmonics"),
            py::arg("delay"))
        .def("setChorus",
            &Generator::setChorus,
            py::arg("n_voices"),
            py::arg("sigma") = 0.02)
        .def("drawDecay",
            &Generator::drawDecay,
            py::arg("duration"),
            py::arg("alpha") = 2.0 * M_PI)
        .def("drawGaussian",
            &Generator::drawGaussian,
            py::arg("duration"),
            py::arg("mu"),
            py::arg("sigma"),
            py::arg("normalize") = true)
        .def("drawFatGaussian",
            &Generator::drawFatGaussian,
            py::arg("duration"),
            py::arg("pass_band_percent") = 0.01)
        .def("drawLine",
            &Generator::drawLine,
            py::arg("duration"),
            py::arg("amplitude_start"),
            py::arg("amplitude_finish"))
        .def("drawParabola",
            &Generator::drawParabola,
            py::arg("duration"),
            py::arg("y1"),
            py::arg("x2"),
            py::arg("y2"),
            py::arg("y3"))
        .def("drawSine",
            py::overload_cast<const float64 &, const float64 &>(&Generator::drawSine),
            py::arg("duration"),
            py::arg("frequency"))
        .def("drawSine",
            py::overload_cast<const float64 &, const Buffer &>(&Generator::drawSine),
            py::arg("duration"),
            py::arg("frequency"))
        .def("drawSine",
            py::overload_cast<const float64 &>(&Generator::drawSine),
            py::arg("frequency"))
        .def("drawSine2",
            py::overload_cast<const float64 &, const float64 &, const float64 &>(&Generator::drawSine2),
            py::arg("duration"),
            py::arg("frequency"),
            py::arg("phase"))
        .def("drawSine2",
            py::overload_cast<const float64 &, const Buffer &, const Buffer &>(&Generator::drawSine2),
            py::arg("duration"),
            py::arg("frequency"),
            py::arg("phase"))
        .def("drawSine2",
            py::overload_cast<const float64 &, const Buffer &, const float64 &>(&Generator::drawSine2),
            py::arg("duration"),
            py::arg("frequency"),
            py::arg("phase") = 0.0)
        .def("drawSine2",
            py::overload_cast<const float64 &, const float64 &, const Buffer &>(&Generator::drawSine2),
            py::arg("duration"),
            py::arg("frequency"),
            py::arg("phase"))
        .def("drawSine2",
            py::overload_cast<const float64 &, const float64 &>(&Generator::drawSine2),
            py::arg("frequency"),
            py::arg("phase"))
        .def("drawWindow",
            &Generator::drawWindow,
            py::arg("duration"),
            py::arg("type"))
        .def("drawWindowBartlett",
            &Generator::drawWindowBartlett,
            py::arg("duration"))
        .def("drawWindowBlackman",
            &Generator::drawWindowBlackman,
            py::arg("duration"))
        .def("drawWindowBlackmanHarris",
            &Generator::drawWindowBlackmanHarris,
            py::arg("duration"))
        .def("drawWindowHamming",
            &Generator::drawWindowHamming,
            py::arg("duration"))
        .def("drawWindowHanning",
            &Generator::drawWindowHanning,
            py::arg("duration"))
        .def("drawWindowKaiser",
            &Generator::drawWindowKaiser,
            py::arg("duration"),
            py::arg("beta") = 5.0)
        .def("drawWindowNuttall",
            &Generator::drawWindowNuttall,
            py::arg("duration"))
        .def("drawWindowParzen",
            &Generator::drawWindowParzen,
            py::arg("duration"))
        .def("drawWindowRectangular",
            &Generator::drawWindowRectangular,
            py::arg("duration"))
        .def("generate",
            py::overload_cast<const float64 &>(&Generator::generate),
            py::arg("frequency"))
        .def("generate",
            py::overload_cast<const float64 &, const float64 &>(&Generator::generate),
            py::arg("duration"),
            py::arg("frequency"))
        .def("generate",
            py::overload_cast<const float64 &, const Buffer &>(&Generator::generate),
            py::arg("duration"),
            py::arg("frequencies"))
        .def("generate2",
            py::overload_cast<const float64 &, const float64 &>(&Generator::generate2),
            py::arg("frequency"),
            py::arg("phase"))
        .def("generate2",
            py::overload_cast<const float64 &, const float64 &, const float64 &>(&Generator::generate2),
            py::arg("duration"),
            py::arg("frequency"),
            py::arg("phase"))
        .def("generate2",
            py::overload_cast<const float64 &, const float64 &, const Buffer &>(&Generator::generate2),
            py::arg("duration"),
            py::arg("frequencies"),
            py::arg("phase"))
        .def("generate2",
            py::overload_cast<const float64 &, const Buffer &, const float64 &>(&Generator::generate2),
            py::arg("duration"),
            py::arg("frequencies"),
            py::arg("phase"))
        .def("generate2",
            py::overload_cast<const float64 &, const Buffer &, const Buffer &>(&Generator::generate2),
            py::arg("duration"),
            py::arg("frequencies"),
            py::arg("phase"))
        .def("removeSlaveSync",
            &Generator::removeSlaveSync,
            py::arg("slave"))
        .def("reset",
            &Generator::reset)
        .def("getRandomNumberGenerator",
            &Generator::getRandomNumberGenerator,
            py::return_value_policy::reference_internal)
        .def("setSeed",
            &Generator::setSeed,
            py::arg("seed"))
        .def("silence",
            &Generator::silence,
            py::arg("duration"))
        .def("tell",
            &Generator::tell)
        .def("whiteNoise",
            &Generator::whiteNoise,
            py::arg("duration"))
        .def("gaussianNoise",
            &Generator::gaussianNoise,
            py::arg("duration"),
            py::arg("mu"),
            py::arg("sigma"))
        .def("tanh",
            &Generator::tanh,
            py::arg("duration"));

    def_copy(generator);

    py::class_<Cosine, Generator> cosine(m, "Cosine");

    cosine
        .def(py::init<const float64 &>(),
            py::arg("sample_rate"));

    def_copy(cosine);

    py::class_<GeneratorDecay, Generator> generator_decay(m, "GeneratorDecay");

    generator_decay
        .def(py::init<const float64 &>(),
            py::arg("sample_rate"))
        .def("generate",
            py::overload_cast<const float64 &>(&GeneratorDecay::generate),
            py::arg("frequency"))
        .def("generate",
            py::overload_cast<const float64 &, const float64 &>(&Generator::generate),
            py::arg("duration"),
            py::arg("frequency"))
        .def("generate",
            py::overload_cast<const float64 &, const Buffer &>(&Generator::generate),
            py::arg("duration"),
            py::arg("frequencies"))
        .def("generate2",
            py::overload_cast<const float64 &, const float64 &>(&GeneratorDecay::generate2),
            py::arg("frequency"),
            py::arg("alpha"))
        .def("generate2",
            py::overload_cast<const float64 &, const float64 &, const float64 &>(&GeneratorDecay::generate2),
            py::arg("duration"),
            py::arg("frequency"),
            py::arg("alpha"))
        .def("generate2",
            py::overload_cast<const float64 &, const float64 &, const Buffer &>(&GeneratorDecay::generate2),
            py::arg("duration"),
            py::arg("frequency"),
            py::arg("alpha"))
        .def("generate2",
            py::overload_cast<const float64 &, const Buffer &, const float64 &>(&GeneratorDecay::generate2),
            py::arg("duration"),
            py::arg("frequencies"),
            py::arg("alpha"))
        .def("generate2",
            py::overload_cast<const float64 &, const Buffer &, const Buffer &>(&GeneratorDecay::generate2),
            py::arg("duration"),
            py::arg("frequencies"),
            py::arg("alpha"));

    def_copy(generator_decay);

    py::class_<Pluck, Generator> pluck(m, "Pluck");

    pluck
        .def(py::init<const float64 &, uint32>(),
            py::arg("sample_rate"),
            py::arg("n_smooth_samples"))
        .def("generate",
            py::overload_cast<const float64 &, const float64 &>(&Pluck::generate),
            py::arg("duration"),
            py::arg("frequency"))
        .def("generate",
            py::overload_cast<const float64 &>(&Pluck::generate),
            py::arg("f"))
        .def("generate",
            py::overload_cast<const float64 &, const Buffer &>(&Pluck::generate),
            py::arg("d"),
            py::arg("f"))
        .def("generate2",
            py::overload_cast<const float64 &, const float64 &>(&Pluck::generate2),
            py::arg("f"),
            py::arg("p"))
        .def("generate2",
            py::overload_cast<const float64 &, const float64 &, const float64 &>(&Pluck::generate2),
            py::arg("d"),
            py::arg("f"),
            py::arg("p"))
        .def("generate2",
            py::overload_cast<const float64 &, const float64 &, const Buffer &>(&Pluck::generate2),
            py::arg("d"),
            py::arg("f"),
            py::arg("p"))
        .def("generate2",
            py::overload_cast<const float64 &, const Buffer &, const float64 &>(&Pluck::generate2),
            py::arg("d"),
            py::arg("f"),
            py::arg("p"))
        .def("generate2",
            py::overload_cast<const float64 &, const Buffer &, const Buffer &>(&Pluck::generate2),
            py::arg("d"),
            py::arg("f"),
            py::arg("p"));

    def_copy(pluck);

    py::class_<Pulse, Generator> pulse(m, "Pulse");

    py::native_enum<Pulse::PulseUnits>(pulse, "PulseUnits", "enum.IntEnum")
        .value("PULSE_SECONDS", Pulse::PULSE_SECONDS)
        .value("PULSE_PERCENT", Pulse::PULSE_PERCENT)
        .export_values()
        .finalize();

    pulse
        .def(py::init<const float64 &, const float64 &, const Pulse::PulseUnits &>(),
            py::arg("sample_rate"),
            py::arg("pulse_width"),
            py::arg("units") = Pulse::PULSE_PERCENT)
        .def("setUnits",
            &Pulse::setUnits,
            py::arg("units"))
        .def("generate",
            py::overload_cast<const float64 &>(&Pulse::generate),
            py::arg("frequency"))
        .def("generate",
            py::overload_cast<const float64 &, const float64 &, const float64 &>(&Pulse::generate),
            py::arg("duration"),
            py::arg("frequency"),
            py::arg("pulse_width"))
        .def("generate",
            py::overload_cast<const float64 &, const float64 &, const Buffer &>(&Pulse::generate),
            py::arg("duration"),
            py::arg("frequency"),
            py::arg("pulse_widths"))
        .def("generate",
            py::overload_cast<const float64 &, const Buffer &, const Buffer &>(&Pulse::generate),
            py::arg("duration"),
            py::arg("frequencies"),
            py::arg("pulse_widths"))
        .def("generate",
            py::overload_cast<const float64 &, const float64 &>(&Generator::generate),
            py::arg("duration"),
            py::arg("frequency"))
        .def("generate",
            py::overload_cast<const float64 &, const Buffer &>(&Generator::generate),
            py::arg("duration"),
            py::arg("frequencies"))
        .def("generate2",
            py::overload_cast<const float64 &, const float64 &>(&Pulse::generate2),
            py::arg("frequency"),
            py::arg("pulse_width"))
        .def("generate2",
            py::overload_cast<const float64 &, const float64 &, const float64 &>(&Pulse::generate2),
            py::arg("duration"),
            py::arg("frequency"),
            py::arg("pulse_width"))
        .def("generate2",
            py::overload_cast<const float64 &, const float64 &, const Buffer &>(&Pulse::generate2),
            py::arg("duration"),
            py::arg("frequency"),
            py::arg("pulse_width"))
        .def("generate2",
            py::overload_cast<const float64 &, const Buffer &, const float64 &>(&Pulse::generate2),
            py::arg("duration"),
            py::arg("frequency"),
            py::arg("pulse_width"))
        .def("generate2",
            py::overload_cast<const float64 &, const Buffer &, const Buffer &>(&Pulse::generate2),
            py::arg("duration"),
            py::arg("frequency"),
            py::arg("pulse_width"))
        .def("reset",
            &Pulse::reset)
        .def("riseTime",
            &Pulse::riseTime)
        .def("fallTime",
            &Pulse::fallTime);

    def_copy(pulse);

    py::class_<Sawtooth, Generator> sawtooth(m, "Sawtooth");

    sawtooth
        .def(py::init<const float64 &, const int32>(),
            py::arg("sample_rate"),
            py::arg("n_harmonics") = 100);

    def_copy(sawtooth);

    py::class_<Sine, Generator> sine(m, "Sine");

    sine
        .def(py::init<const float64 &>(),
            py::arg("sample_rate"));

    def_copy(sine);

    py::class_<Square, Generator> square(m, "Square");

    square
        .def(py::init<const float64 &>(),
            py::arg("sample_rate"))
        .def(py::init<const float64 &, const int32>(),
            py::arg("sample_rate"),
            py::arg("n_harmonics"))
        .def(py::init<const float64 &, const float64 &, const float64 &, const float64 &, const float64 &, const float64 &, const float64 &>(),
            py::arg("sample_rate"),
            py::arg("percent_lambda_1"),
            py::arg("amplitude_1"),
            py::arg("percent_lambda_2"),
            py::arg("percent_lambda_3"),
            py::arg("amplitude_3"),
            py::arg("percent_lambda_4"));

    def_copy(square);

    py::class_<Triangle, Generator> triangle(m, "Triangle");

    triangle
        .def(py::init<const Nsound::float64 &>(),
            py::arg("sample_rate"))
        .def(py::init<const Nsound::float64 &, const Nsound::float64 &, const Nsound::float64 &>(),
            py::arg("sample_rate"),
            py::arg("attack_time"),
            py::arg("release_time"))
        .def(py::init<const Nsound::float64 &, const Nsound::float64 &, const Nsound::float64 &, const Nsound::float64 &, const Nsound::float64 &, const Nsound::float64 &, const Nsound::float64 &, const Nsound::float64 &, const Nsound::float64 &>(),
            py::arg("sample_rate"),
            py::arg("percent_lambda_1"),
            py::arg("percent_lambda_2"),
            py::arg("amplitude_2"),
            py::arg("percent_lambda_3"),
            py::arg("percent_lambda_4"),
            py::arg("percent_lambda_5"),
            py::arg("amplitude_5"),
            py::arg("percent_lambda_6"));

    def_copy(triangle);

    py::class_<EnvelopeAdsr> envelope_adsr(m, "EnvelopeAdsr");

    envelope_adsr
        .def(py::init<const float64 &, const float64 &, const float64 &, const float64 &, const float64 &>(),
            py::arg("sample_rate"),
            py::arg("attack_time"),
            py::arg("delay_time"),
            py::arg("sustain_amplitude"),
            py::arg("release_time"))
        .def("reset",
            &EnvelopeAdsr::reset)
        .def("setAttackTime",
            &EnvelopeAdsr::setAttackTime,
            py::arg("time"))
        .def("setDelayTime",
            &EnvelopeAdsr::setDelayTime,
            py::arg("time"))
        .def("setSustainAmplitude",
            &EnvelopeAdsr::setSustainAmplitude,
            py::arg("amp"))
        .def("setReleaseTime",
            &EnvelopeAdsr::setReleaseTime,
            py::arg("time"))
        .def("shape",
            py::overload_cast<const AudioStream &>(&EnvelopeAdsr::shape),
            py::arg("as"))
        .def("shape",
            py::overload_cast<const Buffer &>(&EnvelopeAdsr::shape),
            py::arg("buf"))
        .def("shape",
            py::overload_cast<float64, bool>(&EnvelopeAdsr::shape),
            py::arg("sample"),
            py::arg("key_on"))
        .def("is_done",
            &EnvelopeAdsr::is_done);

    def_copy(envelope_adsr);
}
