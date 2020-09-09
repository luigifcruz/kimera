#include "kimera/transport.hpp"

#include <pybind11/pybind11.h>

using namespace Kimera;
namespace py = pybind11;

void init_transport(py::module &m) {
    py::class_<Socket>(m, "Socket")
        .def(py::init([](State& state) {
            return std::unique_ptr<Socket>(new Socket(state));
        }))
        .def("LoadServer", &Socket::LoadServer)
        .def("LoadClient", &Socket::LoadClient)
        .def("Push", &Socket::Push, py::return_value_policy::reference)
        .def("Pull", &Socket::Pull, py::return_value_policy::reference);
}
