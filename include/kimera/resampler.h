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

#include "kimera/state.h"

typedef struct {
    bool configured;
    enum AVPixelFormat format;
    struct SwsContext* ctx;
    AVFrame* frame;
} ResamplerState;

ResamplerState* alloc_resampler();
void free_resampler(ResamplerState*);

bool open_resampler(ResamplerState*, enum AVPixelFormat);

bool resampler_push_frame(ResamplerState*, State*, AVFrame*);

#endif