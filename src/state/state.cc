#include "kimera/state.hpp"

namespace Kimera {

void State::LoadReceiver() {
    mode = Mode::RECEIVER;
}

void State::LoadTransmitter() {
    mode = Mode::TRANSMITTER;
}

} // namespace Kimera
