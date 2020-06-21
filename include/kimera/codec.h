#ifndef DECODER_CODEC_H
#define DECODER_CODEC_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/pixdesc.h>

#include "kimera/state.h"

typedef struct {
    AVCodecContext *codec_ctx;
    AVCodecParserContext *parser_ctx;
    bool has_parser;
    AVPacket* retard;
    AVFrame* frame;
} DecoderState;

typedef struct {
    AVCodecContext *codec_ctx;
    AVPacket* packet;
} EncoderState;

typedef struct {
    bool configured;
    enum AVPixelFormat format;
    struct SwsContext* ctx;
    AVFrame* frame;
} ResamplerState;

//
// Decoder Methods
//

bool start_decoder(DecoderState*, State*);
void close_decoder(DecoderState*);
bool decoder_push(DecoderState*, char*, uint32_t, uint64_t);
bool needs_parser(enum AVCodecID);

//
// Encoder Methods
//

bool start_encoder(EncoderState*, State*);
void close_encoder(EncoderState*);
bool encoder_push(EncoderState*, AVFrame*);

//
// Resampler Methods
//

bool open_resampler(ResamplerState*, enum AVPixelFormat);
void close_resampler(ResamplerState*);
bool resampler_push_frame(ResamplerState*, State*, AVFrame*);

#endif