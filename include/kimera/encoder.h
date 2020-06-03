#ifndef ENCODER_CODEC_H
#define ENCODER_CODEC_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>

#include "kimera/state.h"

typedef struct {
    AVCodecContext *codec_ctx;
    AVPacket* packet;
} EncoderState;

EncoderState* alloc_encoder();
void free_encoder(EncoderState*);

bool open_encoder(EncoderState*, State*);

bool encoder_push(EncoderState*, AVFrame*);

#endif