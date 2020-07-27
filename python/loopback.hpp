#include "kimera/loopback.hpp"

#include <pybind11/pybind11.h>

using namespace Kimera;
namespace py = pybind11;

void init_loopback(py::module &m) {
    py::class_<Loopback>(m, "Loopback")
        .def(py::init([](State& state) {
            return std::unique_ptr<Loopback>(new Loopback(state));
        }))
        .def("LoadSink", &Loopback::LoadSink)
        .def("LoadSource", &Loopback::LoadSource)
        .def("Push", &Loopback::Push)
        .def("Pull", &Loopback::Pull);
}
