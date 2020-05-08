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

#include "../../config.h"

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

#endif
