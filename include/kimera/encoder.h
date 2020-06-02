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

bool start_encoder(EncoderState*, State*);
void close_encoder(EncoderState*);
bool encoder_push(EncoderState*, AVFrame*);

#endif