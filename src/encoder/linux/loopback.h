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

#include "../encoder.h"

typedef struct {
    int dev_fd;
    uint8_t* buffer;
} LoopbackState;

bool open_loopback(LoopbackState* state, char*);
void close_loopback(LoopbackState*);
void loopback_push_frame(LoopbackState* state, AVFrame*);

#endif
