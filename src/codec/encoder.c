#include "encoder.h"

bool start_encoder(EncoderState* encoder, State* state) {
    AVCodec *codec = avcodec_find_encoder_by_name(state->codec);
    if (!codec) {
        printf("[ENCODER] Selected encoder (%s) not found.\n", state->codec);
        return false;
    }

    encoder->codec_ctx = avcodec_alloc_context3(codec);
    if (!encoder->codec_ctx) {
        printf("[ENCODER] Couldn't allocate codec context.\n");
        close_encoder(encoder);
        return false;
    }

    encoder->codec_ctx->bit_rate = state->bitrate;
    encoder->codec_ctx->width = state->width;
    encoder->codec_ctx->height = state->height;
    encoder->codec_ctx->time_base = (AVRational){1, state->framerate};
    encoder->codec_ctx->framerate = (AVRational){state->framerate, 1};
    encoder->codec_ctx->gop_size = 10;
    encoder->codec_ctx->max_b_frames = 0;
    encoder->codec_ctx->pix_fmt = state->format;
    encoder->codec_ctx->flags |= AV_CODEC_FLAG_LOW_DELAY;
    if (avcodec_open2(encoder->codec_ctx, codec, NULL) < 0) {
        printf("[ENCODER] Couldn't open codec.\n");
        close_encoder(encoder);
        return false;
    }

    encoder->packet = av_packet_alloc();
    if (!encoder->packet) {
        printf("[ENCODER] Couldn't allocate packet.\n");
        close_encoder(encoder);
        return false;
    }

    return true;
}

void close_encoder(EncoderState* encoder) {
    if (encoder)
        return;
    if (encoder->packet)
        av_packet_free(&encoder->packet);
    if (encoder->codec_ctx)
        avcodec_free_context(&encoder->codec_ctx);
}

bool encoder_push(EncoderState* encoder, AVFrame* frame) {
    av_packet_unref(encoder->packet);

    if (avcodec_send_frame(encoder->codec_ctx, frame) < 0) {
        printf("[ENCODER] Couldn't send frame to context.\n");
        return false;
    }

    int ret = avcodec_receive_packet(encoder->codec_ctx, encoder->packet);

    if (ret == AVERROR(EAGAIN)) {
        return false;
    }

    if (ret < 0) {
        printf("[ENCODER] Error during encoding.\n");
        return false;
    }

    return true;
}