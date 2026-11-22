//-----------------------------------------------------------------------------
//
//  The Python bindings for the instruments, Mesh2D and Mixer.
//
//-----------------------------------------------------------------------------

#include "bindings.h"

#include <Nsound/AudioStream.h>
#include <Nsound/Buffer.h>
#include <Nsound/Clarinet.h>
#include <Nsound/DrumBD01.h>
#include <Nsound/DrumKickBass.h>
#include <Nsound/FluteSlide.h>
#include <Nsound/GuitarBass.h>
#include <Nsound/Hat.h>
#include <Nsound/Instrument.h>
#include <Nsound/Mesh2D.h>
#include <Nsound/Mixer.h>
#include <Nsound/MixerNode.h>
#include <Nsound/OrganPipe.h>

using namespace Nsound;

void
bind_instruments(py::module_ & m)
{
    py::class_<Instrument> instrument(m, "Instrument");

    instrument
        .def("play",
            py::overload_cast<>(&Instrument::play))
        .def("play",
            py::overload_cast<const float64 &, const float64 &>(&Instrument::play),
            py::arg("duration"),
            py::arg("frequency"))
        .def("getInfo",
            &Instrument::getInfo);

    py::class_<Clarinet, Instrument> clarinet(m, "Clarinet");

    clarinet
        .def(py::init<const float64 &>(),
            py::arg("sample_rate"))
        .def("play",
            py::overload_cast<>(&Clarinet::play))
        .def("play",
            py::overload_cast<const float64 &, const float64 &>(&Clarinet::play),
            py::arg("duration"),
            py::arg("frequency"))
        .def("play",
            py::overload_cast<const float64 &, const float64 &, const float64 &, const float64 &, const float64 &>(&Clarinet::play),
            py::arg("duration"),
            py::arg("frequency"),
            py::arg("presure"),
            py::arg("filter"),
            py::arg("reed_stiffness"))
        .def("getInfo",
            &Clarinet::getInfo);

    def_copy(clarinet);

    py::class_<DrumBD01, Instrument> drum_bd01(m, "DrumBD01");

    drum_bd01
        .def(py::init<const float64 &>(),
            py::arg("sample_rate"))
        .def("play",
            py::overload_cast<>(&DrumBD01::play))
        .def("play",
            py::overload_cast<const float64 &, const float64 &>(&DrumBD01::play),
            py::arg("duration"),
            py::arg("frequency"))
        .def("play",
            py::overload_cast<const float64 &, const float64 &, const float64 &>(&DrumBD01::play),
            py::arg("duration"),
            py::arg("frequency"),
            py::arg("velocity"))
        .def("getInfo",
            &DrumBD01::getInfo);

    def_copy(drum_bd01);

    py::class_<DrumKickBass, Instrument> drum_kick_bass(m, "DrumKickBass");

    drum_kick_bass
        .def(py::init<const float64 &, const float64 &, const float64 &>(),
            py::arg("sample_rate"),
            py::arg("high_frequency"),
            py::arg("low_frequency"))
        .def("play",
            py::overload_cast<>(&DrumKickBass::play))
        .def("play",
            py::overload_cast<const float64 &, const float64 &>(&DrumKickBass::play),
            py::arg("duration"),
            py::arg("frequency"))
        .def("play",
            py::overload_cast<const float64 &, const float64 &, const float64 &>(&DrumKickBass::play),
            py::arg("duration"),
            py::arg("high_frequency"),
            py::arg("low_frequency"))
        .def("play",
            py::overload_cast<const float64 &, const Buffer &, const Buffer &>(&DrumKickBass::play),
            py::arg("duation"),
            py::arg("high_frequency"),
            py::arg("low_frequency"))
        .def("getInfo",
            &DrumKickBass::getInfo);

    def_copy(drum_kick_bass);

    py::class_<FluteSlide, Instrument> flute_slide(m, "FluteSlide");

    flute_slide
        .def(py::init<const float64 &>(),
            py::arg("sample_rate"))
        .def("play",
            py::overload_cast<>(&FluteSlide::play))
        .def("play",
            py::overload_cast<const float64 &, const float64 &>(&FluteSlide::play),
            py::arg("duration"),
            py::arg("frequency"))
        .def("play",
            py::overload_cast<const float64 &, const float64 &, const float64 &, const float64 &>(&FluteSlide::play),
            py::arg("duration"),
            py::arg("frequency"),
            py::arg("presure"),
            py::arg("breath"))
        .def("getInfo",
            &FluteSlide::getInfo);

    def_copy(flute_slide);

    py::class_<GuitarBass, Instrument> guitar_bass(m, "GuitarBass");

    guitar_bass
        .def(py::init<const float64 &>(),
            py::arg("sample_rate"))
        .def("play",
            py::overload_cast<>(&GuitarBass::play))
        .def("play",
            py::overload_cast<const float64 &, const float64 &>(&GuitarBass::play),
            py::arg("duration"),
            py::arg("frequency"))
        .def("getInfo",
            &GuitarBass::getInfo);

    def_copy(guitar_bass);

    py::class_<Hat, Instrument> hat(m, "Hat");

    hat
        .def(py::init<const float64 &>(),
            py::arg("sample_rate"))
        .def("play",
            py::overload_cast<>(&Hat::play))
        .def("play",
            py::overload_cast<const float64 &, const float64 &>(&Hat::play),
            py::arg("duration"),
            py::arg("tune"))
        .def("play",
            py::overload_cast<const float64 &, const float64 &, boolean>(&Hat::play),
            py::arg("duration"),
            py::arg("tune"),
            py::arg("is_closed"))
        .def("getInfo",
            &Hat::getInfo);

    def_copy(hat);

    py::class_<OrganPipe, Instrument> organ_pipe(m, "OrganPipe");

    organ_pipe
        .def(py::init<const float64 &>(),
            py::arg("sample_rate"))
        .def("play",
            py::overload_cast<>(&OrganPipe::play))
        .def("play",
            py::overload_cast<const float64 &, const float64 &>(&OrganPipe::play),
            py::arg("duration"),
            py::arg("frequency"))
        .def("play",
            py::overload_cast<const float64 &, const float64 &, const float64 &>(&OrganPipe::play),
            py::arg("duration"),
            py::arg("frequency"),
            py::arg("pan"))
        .def("getInfo",
            &OrganPipe::getInfo);

    def_copy(organ_pipe);

    py::class_<Mesh2D> mesh2_d(m, "Mesh2D");

    mesh2_d
        .def(py::init<const float64 &, uint32, uint32, const float64 &, const float64 &, const float64 &, const float64 &>(),
            py::arg("sample_rate"),
            py::arg("width"),
            py::arg("height"),
            py::arg("leak_gain"),
            py::arg("tau"),
            py::arg("delta") = 6.0,
            py::arg("gamma") = 8.0)
        .def("clear",
            &Mesh2D::clear)
        .def("getEnergy",
            &Mesh2D::getEnergy)
        .def("strike",
            py::overload_cast<const Buffer &, const Buffer &, const Buffer &>(&Mesh2D::strike),
            py::arg("x_pos"),
            py::arg("y_pos"),
            py::arg("velocity"))
        .def("strike",
            py::overload_cast<const float64 &, const float64 &, const float64 &>(&Mesh2D::strike),
            py::arg("x_pos"),
            py::arg("y_pos"),
            py::arg("velocity"))
        .def("setMeshDumping",
            &Mesh2D::setMeshDumping,
            py::arg("flag"),
            py::arg("dirname") = ".",
            py::arg("prefix") = "mesh");

    def_copy(mesh2_d);

    py::class_<Mixer> mixer(m, "Mixer");

    mixer
        .def(py::init<>())
        .def("add",
            &Mixer::add,
            py::arg("first_beat_time"),
            py::arg("beats_per_minute"),
            py::arg("audio_stream"))
        .def("getStream",
            py::overload_cast<float64>(&Mixer::getStream, py::const_),
            py::arg("end_time"))
        .def("getStream",
            py::overload_cast<float64, float64>(&Mixer::getStream, py::const_),
            py::arg("start_time"),
            py::arg("end_time"))
        .def("clear",
            &Mixer::clear)
        .def("size",
            &Mixer::size);

    def_copy(mixer);
}
