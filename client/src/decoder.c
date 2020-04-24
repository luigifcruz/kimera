#include "decoder.h"

bool start_decoder(DecoderState* decoder) {
    AVCodec *codec = avcodec_find_decoder(VIDEO_CODEC);
    if (codec == NULL) {
        printf("Selected decoder not found.\n");
        return false;
    }

    decoder->codec_ctx = avcodec_alloc_context3(codec);
    if (!decoder->codec_ctx) {
        printf("Couldn't allocate codec context.\n");
        close_decoder(decoder);
        return false;
    }

    if (avcodec_open2(decoder->codec_ctx, codec, NULL) < 0) {
        printf("Couldn't open codec.\n");
        close_decoder(decoder);
        return false;
    }

    decoder->parser_ctx = av_parser_init(VIDEO_CODEC);
    if (!decoder->parser_ctx) {
        printf("Couldn't initialize parser.\n");
        close_decoder(decoder);
        return false;
    }

    decoder->parser_ctx->flags |= PARSER_FLAG_COMPLETE_FRAMES;
    decoder->retard = NULL;
    decoder->frame = av_frame_alloc();

    return true;
}

void close_decoder(DecoderState* decoder) {
    if (decoder == NULL)
        return;
    if (decoder->frame != NULL)
        av_frame_free(&decoder->frame);
    if (decoder->parser_ctx != NULL)
        av_parser_close(decoder->parser_ctx);
    if (decoder->codec_ctx != NULL)
        avcodec_free_context(&decoder->codec_ctx);
    if (decoder->retard != NULL)
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
        printf("Couldn't send video packet: %d\n", ret);
        return false;
    }

    ret = avcodec_receive_frame(decoder->codec_ctx, decoder->frame);
    if (ret < 0) {
        printf("Couldn't receive video frame: %d\n", ret);
        return false;
    }

    return true;
}

bool decoder_push(DecoderState* decoder, char* buf, uint32_t len, uint64_t pts) {
    bool status = true;

    AVPacket* packet = av_packet_alloc();
    av_init_packet(packet);
    packet->data = buf;
    packet->size = len;
    packet->pts = pts;

    if (decoder->retard != NULL || packet->pts == AV_NOPTS_VALUE) {
        decoder->retard = av_packet_alloc();
        
        size_t offset = 0;
        if (decoder->retard != NULL) {
            offset = decoder->retard->size;
            if (av_grow_packet(decoder->retard, packet->size)) {
                printf("Couldn't grow packet.\n");
                goto cleanup;
            }
        } else {
            if (av_new_packet(decoder->retard, packet->size)) {
                printf("Couldn't create packet.\n");
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

        if (decoder->retard != NULL)
            decoder->retard = NULL;

        if (!ok) {
            printf("Error parsing AVPacket.\n");
            status = false;
            goto cleanup;
        }
    }

cleanup:
    if (decoder->retard == NULL)
        av_packet_free(&packet);
    return status;
}