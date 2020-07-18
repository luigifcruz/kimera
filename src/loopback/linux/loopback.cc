#include "kimera/loopback/linux.hpp"
#include <libavutil/frame.h>

Loopback::Loopback(State* state) : xcb(state), v4l2(state) {
    this->state = state;
}

bool Loopback::SetSink() {
    return this->v4l2.SetSource();
}

bool Loopback::SetSource() {
    if (this->state->source & DISPLAY)
        return this->xcb.SetSource();
    if (this->state->source & LOOPBACK)
        return this->v4l2.SetSource();
    return false;
}

bool Loopback::Push(AVFrame* frame) {
    return this->v4l2.Push(frame);
}

AVFrame* Loopback::Pull() {
    if (this->state->source & DISPLAY)
        return this->xcb.Pull();
    if (this->state->source & LOOPBACK)
        return this->v4l2.Pull();
    return NULL;
}
