#ifndef LOOPBACK_LINUX_H
#define LOOPBACK_LINUX_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <sys/uio.h>
#include <sys/mman.h>

#include "config.h"

typedef struct {
    int dev_fd;
    char* buffer;
    AVFrame* frame;
    struct v4l2_buffer info;
    struct v4l2_format format;
    struct v4l2_requestbuffers req;
} LoopbackState;

bool open_loopback_sink(LoopbackState*, State*);
bool open_loopback_source(LoopbackState*, State*);

bool loopback_push_frame(LoopbackState*, AVFrame*);
bool loopback_pull_frame(LoopbackState*);

void close_loopback(LoopbackState*);

unsigned int ff_to_v4l(enum AVPixelFormat input) {
    switch (input) {
    case AV_PIX_FMT_RGB24:          return V4L2_PIX_FMT_RGB24; 
    case AV_PIX_FMT_YUYV422:        return V4L2_PIX_FMT_YUYV; 
    case AV_PIX_FMT_YUV420P:        return V4L2_PIX_FMT_YUV420; 
    case AV_PIX_FMT_YUV422P:        return V4L2_PIX_FMT_YUV422P;
    case AV_PIX_FMT_UYVY422:        return V4L2_PIX_FMT_UYVY;
    case AV_PIX_FMT_YUV411P:        return V4L2_PIX_FMT_YUV411P;
    case AV_PIX_FMT_YUV410P:        return V4L2_PIX_FMT_YUV410;
    case AV_PIX_FMT_RGB555LE:       return V4L2_PIX_FMT_RGB555;
    case AV_PIX_FMT_RGB555BE:       return V4L2_PIX_FMT_RGB555X;
    case AV_PIX_FMT_RGB565LE:       return V4L2_PIX_FMT_RGB565;
    case AV_PIX_FMT_RGB565BE:       return V4L2_PIX_FMT_RGB565X;
    case AV_PIX_FMT_BGR24:          return V4L2_PIX_FMT_BGR24;
    case AV_PIX_FMT_GRAY8:          return V4L2_PIX_FMT_GREY;
    case AV_PIX_FMT_NV12:           return V4L2_PIX_FMT_NV12;
#ifdef V4L2_PIX_FMT_SRGGB8
    case AV_PIX_FMT_BAYER_BGGR8:    return V4L2_PIX_FMT_SBGGR8;
    case AV_PIX_FMT_BAYER_GBRG8:    return V4L2_PIX_FMT_SGBRG8;
    case AV_PIX_FMT_BAYER_GRBG8:    return V4L2_PIX_FMT_SGRBG8;
    case AV_PIX_FMT_BAYER_RGGB8:    return V4L2_PIX_FMT_SRGGB8;
#endif
#ifdef V4L2_PIX_FMT_XBGR32
    case AV_PIX_FMT_BGR0:           return V4L2_PIX_FMT_XBGR32;
    case AV_PIX_FMT_0RGB:           return V4L2_PIX_FMT_XRGB32;
    case AV_PIX_FMT_BGRA:           return V4L2_PIX_FMT_ABGR32;
    case AV_PIX_FMT_ARGB:           return V4L2_PIX_FMT_ARGB32;
#elif
    case AV_PIX_FMT_BGR0:           return V4L2_PIX_FMT_BGR32;
    case AV_PIX_FMT_0RGB:           return V4L2_PIX_FMT_RGB32;
#endif
    default:
        printf("[LOOPBACK] Selected pixel format (%d) not supported.\n", input);
        return -1;
    }
}

enum AVPixelFormat v4l_to_ff(unsigned int input) {
    switch (input) {
    case V4L2_PIX_FMT_RGB24:        return AV_PIX_FMT_RGB24; 
    case V4L2_PIX_FMT_YUYV:         return AV_PIX_FMT_YUYV422; 
    case V4L2_PIX_FMT_YUV420:       return AV_PIX_FMT_YUV420P; 
    case V4L2_PIX_FMT_YUV422P:      return AV_PIX_FMT_YUV422P;
    case V4L2_PIX_FMT_UYVY:         return AV_PIX_FMT_UYVY422;
    case V4L2_PIX_FMT_YUV411P:      return AV_PIX_FMT_YUV411P;
    case V4L2_PIX_FMT_YUV410:       return AV_PIX_FMT_YUV410P;
    case V4L2_PIX_FMT_RGB555:       return AV_PIX_FMT_RGB555LE;
    case V4L2_PIX_FMT_RGB555X:      return AV_PIX_FMT_RGB555BE;
    case V4L2_PIX_FMT_RGB565:       return AV_PIX_FMT_RGB565LE;
    case V4L2_PIX_FMT_RGB565X:      return AV_PIX_FMT_RGB565BE;
    case V4L2_PIX_FMT_BGR24:        return AV_PIX_FMT_BGR24;
    case V4L2_PIX_FMT_BGR32:        return AV_PIX_FMT_BGR0;
    case V4L2_PIX_FMT_RGB32:        return AV_PIX_FMT_0RGB;
    case V4L2_PIX_FMT_GREY:         return AV_PIX_FMT_GRAY8;
    case V4L2_PIX_FMT_NV12:         return AV_PIX_FMT_NV12;
#ifdef V4L2_PIX_FMT_SRGGB8
    case V4L2_PIX_FMT_SBGGR8:       return AV_PIX_FMT_BAYER_BGGR8;
    case V4L2_PIX_FMT_SGBRG8:       return AV_PIX_FMT_BAYER_GBRG8;
    case V4L2_PIX_FMT_SGRBG8:       return AV_PIX_FMT_BAYER_GRBG8;
    case V4L2_PIX_FMT_SRGGB8:       return AV_PIX_FMT_BAYER_RGGB8;
#endif
#ifdef V4L2_PIX_FMT_XBGR32
    case V4L2_PIX_FMT_XBGR32:       return AV_PIX_FMT_BGR0;
    case V4L2_PIX_FMT_XRGB32:       return AV_PIX_FMT_0RGB;
    case V4L2_PIX_FMT_ABGR32:       return AV_PIX_FMT_BGRA;
    case V4L2_PIX_FMT_ARGB32:       return AV_PIX_FMT_ARGB;
#endif
#ifdef V4L2_PIX_FMT_Z16
    case V4L2_PIX_FMT_Z16:          return AV_PIX_FMT_GRAY16LE;
#endif
#ifdef V4L2_PIX_FMT_Y16
    case V4L2_PIX_FMT_Y16:          return AV_PIX_FMT_GRAY16LE;
#endif
    default:
        printf("[LOOPBACK] Selected pixel format (%d) not supported.\n", input);
        return -1;
    }
}

#endif
