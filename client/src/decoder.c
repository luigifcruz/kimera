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
    decoder->sws_ctx = NULL;

    return true;
}

void close_decoder(DecoderState* decoder) {
    if (decoder == NULL)
        return;
    if (decoder->sws_ctx != NULL)
        sws_freeContext(decoder->sws_ctx);
    if (decoder->parser_ctx != NULL)
        av_parser_close(decoder->parser_ctx);
    if (decoder->codec_ctx != NULL)
        avcodec_free_context(&decoder->codec_ctx);
    if (decoder->retard != NULL)
        av_packet_free(&decoder->retard);
}

int convert_frame(DecoderState* decoder, AVFrame* frame, char* buf) {
    if (decoder->sws_ctx == NULL) {
        decoder->sws_ctx = sws_getContext(frame->width, frame->height, frame->format,
                                          frame->width, frame->height, 0,
                                          SWS_FAST_BILINEAR, NULL, NULL, NULL);
    }
   size_t y_len = (frame->linesize[0] * frame->height);
    size_t u_len = (frame->linesize[1] * frame->height) / 2;
    size_t v_len = (frame->linesize[2] * frame->height) / 2;
    size_t len = ((frame->linesize[0] * frame->height)     +
                 (frame->linesize[1] * frame->height / 2)  +
                 (frame->linesize[2] * frame->height / 2));

    uint8_t* a = (uint8_t*)malloc(y_len);
    uint8_t* b = (uint8_t*)malloc(u_len);
    uint8_t* c = (uint8_t*)malloc(v_len);
    int dest_linesize[4] = { frame->linesize[0]+frame->linesize[1]+frame->linesize[2], 0,0,0 };
    uint8_t* dest[4] = { a, NULL, NULL, NULL };
    printf("1\n");
    sws_scale(decoder->sws_ctx, frame->data, frame->linesize, 0, frame->height, dest, dest_linesize);
    printf("2\n");
    return dest_linesize[0];
}

bool parse_packet(DecoderState* decoder, AVFrame* frame, AVPacket* packet) {
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

    ret = avcodec_receive_frame(decoder->codec_ctx, frame);
    if (ret < 0) {
        printf("Couldn't receive video frame: %d\n", ret);
        return false;
    }

    return true;
}

bool decoder_push(DecoderState* decoder, AVFrame* frame, char* buf, uint32_t len, uint64_t pts) {
    bool status = true;

    AVPacket* packet = av_packet_alloc();
    av_init_packet(packet);
    packet->data = buf;
    packet->size = len;
    packet->pts = pts != NO_PTS ? pts : AV_NOPTS_VALUE;

    if (packet->pts != AV_NOPTS_VALUE) {
        if (!parse_packet(decoder, frame, packet)) {
            printf("Couldn't generate AVFrame.\n");
            status = false;
            goto cleanup;
        }
    } else {
        printf("Config packet.\n");
        status = false;
        goto cleanup;
    }

cleanup:
    av_packet_free(&packet);
    return status;
}