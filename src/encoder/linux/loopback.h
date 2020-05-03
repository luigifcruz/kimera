#ifndef LOOPBACK_H
#define LOOPBACK_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include <libavcodec/avcodec.h>
#include <sys/uio.h>
#include <sys/mman.h>

#include "../encoder.h"

typedef struct {
    int dev_fd;
    uint8_t* buffer;
    struct v4l2_buffer info;
    struct v4l2_format format;
    struct v4l2_requestbuffers req;
} LoopbackState;

bool open_loopback(LoopbackState* state, char*);
void close_loopback(LoopbackState*);
bool loopback_pull_frame(LoopbackState* state);

#endif
