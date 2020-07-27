#include "kimera/client.hpp"

#include <pybind11/pybind11.h>

using namespace Kimera;
namespace py = pybind11;

void init_client(py::module &m) {
    py::class_<Client>(m, "Client")
        .def(py::init([](State& state) {
            return std::unique_ptr<Client>(new Client(state));
        }))
        .def(py::init([]() {
            State state;
            return std::unique_ptr<Client>(new Client(state));
        }))
        .def("PrintVersion", &Client::PrintVersion)
        .def("PrintState", &Client::PrintState)
        .def("PrintHelp", &Client::PrintHelp)
        .def("PrintKey", &Client::PrintKey)
        .def("ShouldStop", &Client::ShouldStop);
}
