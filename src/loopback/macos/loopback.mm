#include "kimera/loopback/macos/loopback.hpp"

namespace Kimera {

Loopback::Loopback(Kimera::State& state) : state(state) {}

Loopback::~Loopback() {
    stop_capture(proc);
    if (frame) av_frame_free(&frame);
}

bool Loopback::LoadSink() {
    printf("[LOOPBACK] Sink isn't supported yet on macOS.\n");
    return false;
}

bool Loopback::Push(AVFrame* frame) {
    printf("[LOOPBACK] Sink isn't supported yet on macOS.\n");
    return false;
}

bool Loopback::LoadSource() {
    proc = init_capture();

    bool display = CHECK(Interfaces::DISPLAY, state.source);
    if (!start_capture(proc, display, (char*)state.loopback.c_str(), state.framerate, state.width, state.height)) {
        return false;
    }
    
    frame = av_frame_alloc();
    frame->width = get_frame_width(proc);
    frame->height = get_frame_height(proc);
    frame->format = state.in_format;
    frame->pts = 0;

    if (av_frame_get_buffer(frame, 0) < 0){
        printf("[LOOPBACK] Couldn't allocate frame.\n");
        return false;
    }

    return true;
}

AVFrame* Loopback::Pull() {
    if (av_frame_make_writable(frame) < 0) {
        printf("[LOOPBACK] Frame is not writable.\n");
        return nullptr;
    }

    if (!pull_frame(proc, (char**)frame->data, frame->linesize)) {
        return nullptr;
    }

    frame->pts += 1;
    return frame;
}

} // namespace Kimera