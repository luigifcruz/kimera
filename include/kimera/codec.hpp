#ifndef DECODER_CODEC_H
#define DECODER_CODEC_H

#include <cstddef>

extern "C" {
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <libavutil/frame.h>
#include <libavutil/pixfmt.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/pixdesc.h>
}

#include "kimera/state.hpp"

class Decoder {
public:
    Decoder(State*);
    ~Decoder();

    bool Push(AVPacket*);
    AVFrame* Pull();

private:
    AVCodecContext *codec_ctx = NULL;
    AVCodecParserContext *parser_ctx = NULL;
    AVPacket* retard = NULL;
    AVFrame* frame = NULL;
    bool has_parser = false;

    bool HasParser(AVCodecID);
    bool ParsePacket(AVPacket*);
};

class Encoder {
public:
    Encoder(State*);
    ~Encoder();

    bool Push(AVFrame*);
    AVPacket* Pull();

private:
    AVCodecContext *codec_ctx = NULL;
    AVPacket* packet = NULL;
};

class Resampler {
public:
    Resampler(State*, enum AVPixelFormat);
    ~Resampler();

    bool Push(AVFrame*);
    AVFrame* Pull();

private:
    bool configured;
    enum AVPixelFormat format;
    struct SwsContext* ctx = NULL;
    AVFrame* frame = NULL;
    State* state = NULL;

    bool ConfigureResampler(AVFrame*);
};


#endif
