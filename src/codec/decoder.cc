#include "kimera/codec.hpp"

Decoder::~Decoder() {
    if (this->frame)
        av_frame_free(&this->frame);
    if (this->parser_ctx)
        av_parser_close(this->parser_ctx);
    if (this->codec_ctx)
        avcodec_free_context(&this->codec_ctx);
    if (this->retard)
        av_packet_free(&this->retard);
}

Decoder::Decoder(Kimera* state) {
    AVCodec *codec = avcodec_find_decoder_by_name(state->codec);
    if (!codec) {
        printf("[DECODER] Selected decoder (%s) not found.\n", state->codec);
        throw;
    }

    this->codec_ctx = avcodec_alloc_context3(codec);
    if (!this->codec_ctx) {
        printf("[DECODER] Couldn't allocate codec context.\n");
        throw;
    }

    this->codec_ctx->bit_rate = state->bitrate;
    this->codec_ctx->width = state->width;
    this->codec_ctx->height = state->height;
    this->codec_ctx->time_base = (AVRational){1, state->framerate};
    this->codec_ctx->framerate = (AVRational){state->framerate, 1};
    this->codec_ctx->gop_size = 10;
    this->codec_ctx->max_b_frames = 0;
    this->codec_ctx->pix_fmt = state->in_format;
    this->codec_ctx->flags |= AV_CODEC_FLAG_LOW_DELAY;

    if (avcodec_open2(this->codec_ctx, codec, NULL) < 0) {
        printf("[DECODER] Couldn't open codec.\n");
        throw;
    }

    this->frame = av_frame_alloc();
    this->has_parser = this->HasParser(codec->id);

    if (this->has_parser) {
        if (!(this->parser_ctx = av_parser_init(codec->id))) {
            printf("[DECODER] Couldn't initialize parser.\n");
            throw;
        }
        this->parser_ctx->flags |= PARSER_FLAG_COMPLETE_FRAMES;
        this->parser_ctx->format = state->in_format;
    }
}

bool Decoder::Push(AVPacket* packet) {
    bool status = false;

    if (this->retard || packet->pts == AV_NOPTS_VALUE) {
        this->retard = av_packet_alloc();

        size_t offset = 0;
        if (this->retard) {
            offset = this->retard->size;
            if (av_grow_packet(this->retard, packet->size)) {
                printf("[DECODER] Couldn't grow packet.\n");
                goto cleanup;
            }
        } else {
            if (av_new_packet(this->retard, packet->size)) {
                printf("[DECODER] Couldn't create packet.\n");
                goto cleanup;
            }
        }
        memcpy(this->retard->data + offset, packet->data, packet->size);

        if (packet->pts != AV_NOPTS_VALUE) {
            this->retard->pts = packet->pts;
            this->retard->dts = packet->dts;
            this->retard->flags = packet->flags;
            av_packet_free(&packet);
            packet = this->retard;
        }
    }

    if (packet->pts != AV_NOPTS_VALUE) {
        bool ok = this->ParsePacket(packet);

        if (this->retard)
            this->retard = NULL;

        if (!ok) {
            goto cleanup;
        }

        status = true;
    } else {
        printf("[DECODER] Config packet.\n");
        goto cleanup;
    }

cleanup:
    if (this->retard == NULL)
        av_packet_free(&packet);
    return status;
}

AVFrame* Decoder::Pull() {
    return this->frame;
}

bool Decoder::ParsePacket(AVPacket* packet) {
    uint8_t *out_data = NULL;

    if (this->has_parser) {
        int out_len = 0;
        int r = av_parser_parse2(this->parser_ctx, this->codec_ctx,
                                &out_data, &out_len, packet->data, packet->size,
                                AV_NOPTS_VALUE, AV_NOPTS_VALUE, -1);

        if (r != packet->size || out_len != packet->size)
            return false;

        if (this->parser_ctx->key_frame == 1) {
            packet->flags |= AV_PKT_FLAG_KEY;
        }
    }

    int ret;
    if ((ret = avcodec_send_packet(this->codec_ctx, packet)) < 0) {
        printf("[DECODER] Couldn't send video packet: %d\n", ret);
        return false;
    }

    if ((ret = avcodec_receive_frame(this->codec_ctx, this->frame)) < 0) {
        printf("[DECODER] Couldn't receive video frame: %d\n", ret);
        return false;
    }

    return true;
}

bool Decoder::HasParser(enum AVCodecID codec_id) {
    switch(codec_id){
        case AV_CODEC_ID_RAWVIDEO:
        case AV_CODEC_ID_HUFFYUV:
        case AV_CODEC_ID_FFVHUFF:
            return false;
        default:
            return true;
    }
}
