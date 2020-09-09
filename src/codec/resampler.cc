#include "kimera/codec.hpp"

namespace Kimera {

Resampler::Resampler(State& state, PixelFormat format) : state(state), format(format), configured(false) {};

Resampler::~Resampler() {
    if (this->frame)
        av_frame_free(&this->frame);
    if (this->ctx)
        sws_freeContext(this->ctx);
}

bool Resampler::ConfigureResampler(AVFrame* in) {
    // Recycle previous resampler.
    if (this->frame)
        av_frame_free(&this->frame);
    if (this->ctx)
        sws_freeContext(this->ctx);

    // Allocate new frame for the resampler.
    this->frame = av_frame_alloc();
    this->frame->width = state.width;
    this->frame->height = state.height;
    this->frame->format = this->format;
    this->frame->pts = in->pts;
    if (av_frame_get_buffer(this->frame, 0) < 0){
        printf("[RESAMPLER] Couldn't allocate frame.\n");
        return false;
    }

    // Create resampler with the current settings.
    this->ctx = sws_getContext(in->width,
                               in->height,
                               (PixelFormat)in->format,
                               this->frame->width,
                               this->frame->height,
                               (PixelFormat)this->frame->format,
                               SWS_BICUBIC, 0, 0, 0);

    // Performance Degradation Check
    if (in->format != this->format) {
        printf("[RESAMPLER] Performance Degradation:\n");
        printf("              Output pixel format is different than the input.\n");
        printf("                - Input: %s -> Output: %s\n",
               av_get_pix_fmt_name((PixelFormat)in->format),
               av_get_pix_fmt_name(this->format));
    }

    if (
        in->width != this->frame->width   ||
        in->height != this->frame->height
    ) {
        printf("[RESAMPLER] Performance Degradation:\n");
        printf("              Output size is different than the input.\n");
        printf("                - Input: %dx%d -> Output: %dx%d\n",
               in->width, in->height,
               this->frame->width, this->frame->height);
    }


    this->configured = true;
    return true;
}

bool Resampler::Push(AVFrame* in) {
    if (!this->configured) {
        if (!ConfigureResampler(in))
            return false;
    }

    this->frame->pts = in->pts;
    if (sws_scale(
            this->ctx, (const unsigned char* const*)in->data, in->linesize, 0,
            in->height, this->frame->data, this->frame->linesize) < 0) {
        printf("[RESAMPLER] Can't convert this frame.\n");
        return false;
    }
    return true;
}

AVFrame* Resampler::Pull() {
    return this->frame;
}

} // namespace Kimera