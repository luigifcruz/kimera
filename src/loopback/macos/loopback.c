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
    loopback->buffer = (char*)malloc(state->frame_size*2);
    if (!loopback->buffer) {
        printf("[LOOPBACK] Couldn't allocate loopback buffer.\n");
        return false;
    }
    printf("%d\n", state->frame_size);
    return avfoundation_open_source(&loopback->state);
}

static bool loopback_pull_frame(LoopbackState* loopback) {
    return avfoundation_pull_frame(&loopback->state, loopback->buffer);
}

static void close_loopback(LoopbackState* loopback) {
    avfoundation_close_source(&loopback->state);
}