#include "loopback.h"

inline bool open_loopback_sink(LoopbackState* loopback, State* state) {
    return init_v4l2_sink(loopback, state);
}

inline bool loopback_push_frame(LoopbackState* loopback, AVFrame* frame) {
    return push_v4l2_frame(loopback, frame);
}

inline bool open_loopback_source(LoopbackState* loopback, State* state) {
    if (state->source & DISPLAY)
        return init_xcb_source(loopback, state);
    if (state->source & LOOPBACK)
        return init_v4l2_source(loopback, state);
    return false;
}

inline bool loopback_pull_frame(LoopbackState* loopback, State* state) {
    if (state->source & DISPLAY)
        return pull_xcb_frame(loopback, state);
    if (state->source & LOOPBACK)
        return pull_v4l2_frame(loopback);
    return false;
}

void close_loopback(LoopbackState* loopback, State* state) {
    if (state->source & DISPLAY)
        return close_xcb(loopback);
    if (state->source & LOOPBACK)
        return close_v4l2(loopback, state);

    if (loopback->frame)
        av_frame_free(&loopback->frame);
}