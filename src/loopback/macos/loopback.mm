#include "kimera/loopback/macos/loopback.hpp"

#import "kimera/loopback/macos/CameraAdapter.hpp"

namespace Kimera {

Loopback::Loopback(Kimera::State& state) : state(state) {}

Loopback::~Loopback() {
    [(id)processor stopCapture];
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
    processor = [CameraAdapter new];

    if (![(id)processor startCapture:&state]) {
        return false;
    }
    
    frame = av_frame_alloc();
    frame->width = [(id)processor getFrameWidth];
    frame->height = [(id)processor getFrameHeight];
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

    if (![(id)processor pullFrame: frame]) {
        return nullptr;
    }

    frame->pts += 1;
    return frame;
}

} // namespace Kimera