#include "kimera/codec.hpp"

#include <pybind11/pybind11.h>

using namespace Kimera;
namespace py = pybind11;

void init_frame(py::module &m) {
    py::class_<AVFrame>(m, "AVFrame");
}

