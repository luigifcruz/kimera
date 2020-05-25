#include "decoder.h"

bool start_decoder(DecoderState* decoder, State* state) {
    AVCodec *codec = avcodec_find_decoder_by_name(state->codec);
    if (!codec) {
        printf("[DECODER] Selected decoder (%s) not found.\n", state->codec);
        return false;
    }

    decoder->codec_ctx = avcodec_alloc_context3(codec);
    if (!decoder->codec_ctx) {
        printf("[DECODER] Couldn't allocate codec context.\n");
        close_decoder(decoder);
        return false;
    }

    decoder->codec_ctx->bit_rate = state->bitrate;
    decoder->codec_ctx->width = state->width;
    decoder->codec_ctx->height = state->height;
    decoder->codec_ctx->time_base = (AVRational){1, state->framerate};
    decoder->codec_ctx->framerate = (AVRational){state->framerate, 1};
    decoder->codec_ctx->gop_size = 10;
    decoder->codec_ctx->max_b_frames = 0;
    decoder->codec_ctx->pix_fmt = state->in_format;
    if (avcodec_open2(decoder->codec_ctx, codec, NULL) < 0) {
        printf("[DECODER] Couldn't open codec.\n");
        close_decoder(decoder);
        return false;
    }

    decoder->parser_ctx = av_parser_init(codec->id);
    if (!decoder->parser_ctx) {
        printf("[DECODER] Couldn't initialize parser.\n");
        close_decoder(decoder);
        return false;
    }

    decoder->parser_ctx->flags |= PARSER_FLAG_COMPLETE_FRAMES;
    decoder->parser_ctx->format = state->in_format;
    decoder->retard = NULL;
    decoder->frame = av_frame_alloc();

    return true;
}

void close_decoder(DecoderState* decoder) {
    if (decoder)
        return;
    if (decoder->frame)
        av_frame_free(&decoder->frame);
    if (decoder->parser_ctx)
        av_parser_close(decoder->parser_ctx);
    if (decoder->codec_ctx)
        avcodec_free_context(&decoder->codec_ctx);
    if (decoder->retard)
        av_packet_free(&decoder->retard);
}

bool parse_packet(DecoderState* decoder, AVPacket* packet) {
    uint8_t *out_data = NULL;
    int out_len = 0;
    int r = av_parser_parse2(decoder->parser_ctx, decoder->codec_ctx,
                             &out_data, &out_len, packet->data, packet->size,
                             AV_NOPTS_VALUE, AV_NOPTS_VALUE, -1);

    assert(r == packet->size);
    (void) r;
    assert(out_len == packet->size);

    if (decoder->parser_ctx->key_frame == 1) {
        packet->flags |= AV_PKT_FLAG_KEY;
    }

    int ret;
    if ((ret = avcodec_send_packet(decoder->codec_ctx, packet)) < 0) {
        printf("[DECODER] Couldn't send video packet: %d\n", ret);
        return false;
    }

    ret = avcodec_receive_frame(decoder->codec_ctx, decoder->frame);
    if (ret < 0) {
        printf("[DECODER] Couldn't receive video frame: %d\n", ret);
        return false;
    }

    return true;
}

bool decoder_push(DecoderState* decoder, char* buf, uint32_t len, uint64_t pts) {
    bool status = false;

    AVPacket* packet = av_packet_alloc();
    av_init_packet(packet);
    packet->data = (uint8_t*)buf;
    packet->size = len;
    packet->pts = pts;

    if (decoder->retard || packet->pts == AV_NOPTS_VALUE) {
        decoder->retard = av_packet_alloc();
        
        size_t offset = 0;
        if (decoder->retard) {
            offset = decoder->retard->size;
            if (av_grow_packet(decoder->retard, packet->size)) {
                printf("[DECODER] Couldn't grow packet.\n");
                goto cleanup;
            }
        } else {
            if (av_new_packet(decoder->retard, packet->size)) {
                printf("[DECODER] Couldn't create packet.\n");
                goto cleanup;
            }
        }
        memcpy(decoder->retard->data + offset, packet->data, packet->size);

        if (packet->pts != AV_NOPTS_VALUE) {
            decoder->retard->pts = packet->pts;
            decoder->retard->dts = packet->dts;
            decoder->retard->flags = packet->flags;
            av_packet_free(&packet);
            packet = decoder->retard;
        }
    }

    if (packet->pts != AV_NOPTS_VALUE) {
        bool ok = parse_packet(decoder, packet);

        if (decoder->retard)
            decoder->retard = NULL;

        if (!ok) {
            goto cleanup;
        }

        status = true;
    } else {
        printf("[DECODER] Config packet.\n");
        goto cleanup;
    }

cleanup:
    if (decoder->retard == NULL)
        av_packet_free(&packet);
    return status;
}