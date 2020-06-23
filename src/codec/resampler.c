#include "kimera/codec.h"

ResamplerState* init_resampler() {
    ResamplerState* state = malloc(sizeof(ResamplerState));
    state->ctx   = NULL;
    state->frame = NULL;
    return state;
}

void close_resampler(ResamplerState* resampler) {
    if (resampler->frame)
        av_frame_free(&resampler->frame);
    if (resampler->ctx)
        sws_freeContext(resampler->ctx);
    free(resampler);
}

bool open_resampler(ResamplerState* resampler, enum AVPixelFormat format) {
    resampler->configured = false;
    resampler->format = format;
    return true;
}

bool configure_resampler(ResamplerState* resampler, State* state, AVFrame* in) {
    // Recycle previous resampler.
    if (resampler->frame)
        av_frame_free(&resampler->frame);
    if (resampler->ctx)
        sws_freeContext(resampler->ctx);

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
    if (in->format != resampler->format) {
        printf("[RESAMPLER] Performance Degradation:\n");
        printf("              Output pixel format is different than the input.\n");
        printf("                - Input: %s -> Output: %s\n",
               av_get_pix_fmt_name(in->format),
               av_get_pix_fmt_name(resampler->format));
    }

    if (
        in->width != resampler->frame->width   ||
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