#ifndef DECODER_H
#define DECODER_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/pixfmt.h>
#include <libavutil/avutil.h>

#include "config.h"

typedef struct {
    AVCodecContext *codec_ctx;
    AVCodecParserContext *parser_ctx;
    AVPacket* retard;
    struct SwsContext* sws_ctx;
} DecoderState;

bool start_decoder(DecoderState*);
void close_decoder(DecoderState*);
int convert_frame(DecoderState*, AVFrame*, char*);
bool decoder_push(DecoderState*, AVFrame*, char*, uint32_t, uint64_t);

#endif