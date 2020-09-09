#include "kimera/state.hpp"

#include <pybind11/pybind11.h>

using namespace Kimera;
namespace py = pybind11;

void init_state(py::module &m) {
    py::class_<State>(m, "State")
        .def(py::init())
        .def("LoadTransmitter", &State::LoadTransmitter)
        .def("LoadReceiver", &State::LoadReceiver)
        .def("ParseConfigFile", &State::ParseConfigFile);
}
