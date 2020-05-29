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

    // Performance Degradation Check
    if (state->in_format != state->out_format) {
        printf("[RESAMPLER] Performance Degradation:\n");
        printf("              Output pixel format is different than the input.\n");
        printf("                - Input: %s -> Output: %s\n",
               av_get_pix_fmt_name(state->in_format),
               av_get_pix_fmt_name(state->out_format));
    }

    if (
        in->width != resampler->frame->width ||
        in->height != resampler->frame->height
    ) {
        printf("[RESAMPLER] Performance Degradation:\n");
        printf("              Output size is different than the input.\n");
        printf("                - Input: %dx%d -> Output: %dx%d\n",
               in->width, in->height,
               resampler->frame->width, resampler->frame->height);
    }

    resampler->configured = true;
    return true;
}

bool resampler_push_frame(ResamplerState* resampler, State* state, AVFrame* in) {
    if (!resampler->configured) {
        if (!configure_resampler(resampler, state, in))
            return false;
    }

    resampler->frame->pts = in->pts;
    if (sws_scale(
            resampler->ctx, (const unsigned char* const*)in->data, in->linesize, 0,
            in->height, resampler->frame->data, resampler->frame->linesize) < 0) {
        printf("[RESAMPLER] Can't convert this frame.\n");
        return false;
    }
    return true;
}