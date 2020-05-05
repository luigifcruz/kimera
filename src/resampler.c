#include "resampler.h"

void open_resampler(ResamplerState* resampler, enum AVPixelFormat format) {
    resampler->configured = false;
    resampler->format = format;
}

void close_resampler(ResamplerState* resampler) {
    if (!resampler->configured)
        return;

    av_frame_free(&resampler->frame);
    sws_freeContext(resampler->ctx);
    resampler->configured = false;
}

bool configure_resampler(ResamplerState* resampler, State* state, AVFrame* in) {
    // Destroy previous resampler (if it ever existed).
    close_resampler(resampler);

    // Allocate new frame for the resampler.
    resampler->frame = av_frame_alloc();
    resampler->frame->width = state->width;
    resampler->frame->height = state->height;
    resampler->frame->format = resampler->format;
    resampler->frame->pts = in->pts;
    if (av_frame_get_buffer(resampler->frame, 0) < 0){
        printf("[RESAMPLER] Couldn't allocate frame.\n");
        return false;
    }

    // Create resampler with the current settings.
    resampler->ctx = sws_getContext(in->width,
                                    in->height,
                                    in->format,
                                    resampler->frame->width,
                                    resampler->frame->height,
                                    resampler->frame->format,
                                    SWS_BICUBIC, 0, 0, 0);

    resampler->configured = true;
    return true;
}

bool resampler_push_frame(ResamplerState* resampler, State* state, AVFrame* in) {
    if (
        !resampler->configured          ||
        in->width != state->width       ||
        in->height != state->height     ||
        in->format != resampler->format
    ) {
        if (!configure_resampler(resampler, state, in))
            return false;
    }

    resampler->frame->pts = in->pts;
    if (sws_scale(resampler->ctx, in->data, in->linesize, 0, in->height,
                  resampler->frame->data, resampler->frame->linesize) < 0) {
        printf("[RESAMPLER] Can't convert this frame.\n");
        return false;
    }
    return true;
}