#include "client.hpp"
#include "codec.hpp"
#include "loopback.hpp"
#include "state.hpp"
#include "transport.hpp"

#include <pybind11/pybind11.h>

using namespace Kimera;
namespace py = pybind11;

PYBIND11_MODULE(kimera, m) {
    init_loopback(m);
    init_client(m);
    init_codec(m);
    init_state(m);
    init_transport(m);
}
