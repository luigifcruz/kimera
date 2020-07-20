#include "kimera/codec.hpp"
#include <libavcodec/avcodec.h>

Encoder::Encoder(Kimera* state) {
    AVCodec *codec = avcodec_find_encoder_by_name(state->codec);

    if (!codec) {
        printf("[ENCODER] Selected encoder (%s) not found.\n", state->codec);
        throw;;
    }

    this->codec_ctx = avcodec_alloc_context3(codec);
    if (!this->codec_ctx) {
        printf("[ENCODER] Couldn't allocate codec context.\n");
        throw;;
    }

    this->codec_ctx->bit_rate = state->bitrate;
    this->codec_ctx->width = state->width;
    this->codec_ctx->height = state->height;
    this->codec_ctx->time_base = (AVRational){1, state->framerate};
    this->codec_ctx->framerate = (AVRational){state->framerate, 1};
    this->codec_ctx->gop_size = 10;
    this->codec_ctx->max_b_frames = 0;
    this->codec_ctx->pix_fmt = state->out_format;
    this->codec_ctx->flags |= AV_CODEC_FLAG_LOW_DELAY;
    if (avcodec_open2(this->codec_ctx, codec, NULL) < 0) {
        printf("[ENCODER] Couldn't open codec.\n");
        throw;;
    }

    this->packet = av_packet_alloc();
    if (!this->packet) {
        printf("[ENCODER] Couldn't allocate packet.\n");
        throw;;
    }
}

Encoder::~Encoder() {
    if (this->packet)
        av_packet_free(&this->packet);
    if (this->codec_ctx)
        avcodec_free_context(&this->codec_ctx);
}

bool Encoder::Push(AVFrame* frame) {
    av_packet_unref(this->packet);

    if (avcodec_send_frame(this->codec_ctx, frame) < 0) {
        printf("[ENCODER] Couldn't send frame to context.\n");
        return false;
    }

    int ret = avcodec_receive_packet(this->codec_ctx, this->packet);

    if (ret == AVERROR(EAGAIN)) {
        return false;
    }

    if (ret < 0) {
        printf("[ENCODER] Error during encoding.\n");
        return false;
    }

    this->packet->pts = frame->pts;
    return true;
}

AVPacket* Encoder::Pull() {
    return this->packet;
}
