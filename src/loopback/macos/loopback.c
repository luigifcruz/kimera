#include "loopback.h"

static bool open_loopback_sink(LoopbackState* loopback, State* state) {
    printf("[LOOPBACK] Sink isn't supported yet on macOS.\n");
    return false;
}

static bool loopback_push_frame(LoopbackState* loopback, AVFrame* in) {
    printf("[LOOPBACK] Sink isn't supported yet on macOS.\n");
    return false;
}

static bool open_loopback_source(LoopbackState* loopback, State* state) {
    loopback->frame = av_frame_alloc();
    loopback->frame->width = state->width;
    loopback->frame->height = state->height;
    loopback->frame->format = state->format;
    loopback->frame->pts = 0;
    if (av_frame_get_buffer(loopback->frame, 0) < 0){
        printf("[LOOPBACK] Couldn't allocate frame.\n");
        return false;
    }

    return avfoundation_open_source(&loopback->state);
}

static bool loopback_pull_frame(LoopbackState* loopback) {
    if (av_frame_make_writable(loopback->frame) < 0) {
        printf("[LOOPBACK] Frame is not writable.\n");
        return false;
    }

    if (!avfoundation_pull_frame(
        &loopback->state,
        (void*)loopback->frame->data[0],
        (void*)loopback->frame->data[1],
        (void*)loopback->frame->data[2])) {
        return false;
    }

    loopback->frame->pts += 1;
    return true;
}

static void close_loopback(LoopbackState* loopback) {
    avfoundation_close_source(&loopback->state);
     if (loopback->frame)
        av_frame_free(&loopback->frame);
}