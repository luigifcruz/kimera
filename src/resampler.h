#ifndef RESAMPLER_H
#define RESAMPLER_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>

#include "config.h"

typedef struct {
    struct SwsContext* ctx;
    AVFrame* frame;
} ResamplerState;

bool open_resampler(ResamplerState*, State* state, AVFrame* frame);
void close_resampler(ResamplerState*);

bool resampler_push_frame(ResamplerState*, AVFrame*);

#endif