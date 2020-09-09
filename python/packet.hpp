#include "kimera/codec.hpp"

#include <pybind11/pybind11.h>

using namespace Kimera;
namespace py = pybind11;

void init_packet(py::module &m) {
    py::class_<AVPacket>(m, "AVPacket");
}
