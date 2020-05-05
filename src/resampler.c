#include "resampler.h"

bool open_resampler(ResamplerState* resampler, State* state, AVFrame* frame) {
    resampler->frame = av_frame_alloc();
    resampler->frame->width = frame->width;
    resampler->frame->height = frame->height;
    resampler->frame->format = state->format;
    resampler->frame->pts = 0;
    if (av_frame_get_buffer(resampler->frame, 0) < 0){
        printf("[RESAMPLER] Couldn't allocate frame.\n");
        return false;
    }

    resampler->ctx = sws_getContext(state->width, state->height, frame->format,
                                    state->width, state->height, state->format, 0, 0, 0, 0);

    return true;
}

void close_resampler(ResamplerState* resampler) {
    sws_freeContext(resampler->ctx);
}

bool resampler_push_frame(ResamplerState* resampler, AVFrame* in) {
    resampler->frame->pts = in->pts;
    if (sws_scale(resampler->ctx, in->data, in->linesize, 0, in->height,
                  resampler->frame->data, resampler->frame->linesize) < 0) {
        printf("[RESAMPLER] Can't convert frame.\n");
        return false;
    }
    return true;
}