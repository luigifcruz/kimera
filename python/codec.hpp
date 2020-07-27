#include "kimera/codec.hpp"

#include <pybind11/pybind11.h>

using namespace Kimera;
namespace py = pybind11;

void init_codec(py::module &m) {
    py::class_<Decoder>(m, "Decoder")
        .def(py::init([](State& state) {
            return std::unique_ptr<Decoder>(new Decoder(state));
        }))
        .def("Push", &Decoder::Push, py::return_value_policy::reference)
        .def("Pull", &Decoder::Pull, py::return_value_policy::reference);

    py::class_<Encoder>(m, "Encoder")
        .def(py::init([](State& state) {
            return std::unique_ptr<Encoder>(new Encoder(state));
        }))
        .def("Push", &Encoder::Push, py::return_value_policy::reference)
        .def("Pull", &Encoder::Pull, py::return_value_policy::reference);

    py::class_<Resampler>(m, "Resampler")
        .def(py::init([](State& state) {
            return std::unique_ptr<Resampler>(new Resampler(state, state.out_format));
        }))
        .def(py::init([](State& state, enum AVPixelFormat fmt) {
            return std::unique_ptr<Resampler>(new Resampler(state, fmt));
        }))
        .def("Push", &Resampler::Push, py::return_value_policy::reference)
        .def("Pull", &Resampler::Pull, py::return_value_policy::reference);
}
