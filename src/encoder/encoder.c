#include "encoder.h"

bool start_encoder(EncoderState* encoder) {
    AVCodec *codec = avcodec_find_encoder_by_name("hevc_nvenc");
    if (!codec) {
        printf("[ENCODER] Selected encoder not found.\n");
        return false;
    }

    encoder->codec_ctx = avcodec_alloc_context3(codec);
    if (!encoder->codec_ctx) {
        printf("[ENCODER] Couldn't allocate codec context.\n");
        close_encoder(encoder);
        return false;
    }

    encoder->codec_ctx->bit_rate = 15000000;
    encoder->codec_ctx->width = 1920;
    encoder->codec_ctx->height = 1080;
    encoder->codec_ctx->time_base = (AVRational){1, 60};
    encoder->codec_ctx->framerate = (AVRational){60, 1};
    encoder->codec_ctx->gop_size = 10;
    encoder->codec_ctx->max_b_frames = 0;
    encoder->codec_ctx->pix_fmt = AV_PIX_FMT_YUV420P;

    if (avcodec_open2(encoder->codec_ctx, codec, NULL) < 0) {
        printf("[ENCODER] Couldn't open codec.\n");
        close_encoder(encoder);
        return false;
    }

    encoder->pts = 0;
    encoder->frame = av_frame_alloc();
    encoder->packet = av_packet_alloc();

    encoder->frame->width = encoder->codec_ctx->width;
    encoder->frame->height = encoder->codec_ctx->height;
    encoder->frame->format = encoder->codec_ctx->pix_fmt;
    encoder->frame->pts = 0;

    if (av_frame_get_buffer(encoder->frame, 0) < 0){
        printf("[ENCODER] Couldn't allocate frame.\n");
        close_encoder(encoder);
        return false;
    }

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
    if (encoder->frame)
        av_frame_free(&encoder->frame);
    if (encoder->codec_ctx)
        avcodec_free_context(&encoder->codec_ctx);
}

bool encoder_push(EncoderState* encoder, char* in_buf) {
    av_packet_unref(encoder->packet);

    if (av_frame_make_writable(encoder->frame) < 0) {
        printf("[ENCODER] Frame is not writable.\n");
        return false;
    }

    // Draw picture into frame.

    encoder->pts += 1;

    if (avcodec_send_frame(encoder->codec_ctx, encoder->frame) < 0) {
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

    //printf("encoded frame %ld (size=%5d)\n",  encoder->frame->pts, encoder->packet->size);
    return true;
}