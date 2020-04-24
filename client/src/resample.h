#ifndef RESAMPLE_H
#define RESAMPLE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>

#include "decoder.h"

typedef struct {
    struct SwsContext* sws_ctx;
} ResampleState;

int convert_frame(ResampleState*, AVFrame*, char*);

#endif