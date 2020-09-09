#include "kimera/loopback/linux.hpp"

namespace Kimera {

Loopback::Loopback(State& state) : state(state), xcb(state), v4l2(state) {}

bool Loopback::LoadSink() {
    return this->v4l2.SetSource();
}

bool Loopback::LoadSource() {
    if (CHECK(this->state.source, Interfaces::DISPLAY))
        return this->xcb.SetSource();
    if (CHECK(this->state.source, Interfaces::LOOPBACK))
        return this->v4l2.SetSource();
    return false;
}

bool Loopback::Push(AVFrame* frame) {
    return this->v4l2.Push(frame);
}

AVFrame* Loopback::Pull() {
    if (CHECK(this->state.source, Interfaces::DISPLAY))
        return this->xcb.Pull();
    if (CHECK(this->state.source, Interfaces::LOOPBACK))
        return this->v4l2.Pull();
    return NULL;
}

} // namespace Kimera