#ifndef ENCODER_H
#define ENCODER_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>

#include "config.h"

typedef struct {
    AVCodecContext *codec_ctx;
    AVPacket* packet;
    AVFrame* frame;
    uint64_t pts;
} EncoderState;

bool start_encoder(EncoderState*);
void close_encoder(EncoderState*);
bool encoder_push(EncoderState*, char*);

#endif