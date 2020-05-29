#ifndef RESAMPLER_H
#define RESAMPLER_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/pixdesc.h>

#include "config.h"

typedef struct {
    bool configured;
    enum AVPixelFormat format;
    struct SwsContext* ctx;
    AVFrame* frame;
} ResamplerState;

void open_resampler(ResamplerState*, enum AVPixelFormat);
void close_resampler(ResamplerState*);

bool resampler_push_frame(ResamplerState*, State*, AVFrame*);

#endif