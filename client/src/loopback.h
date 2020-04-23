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

#include "decoder.h"

int open_loopback(char*);
void loopback_push_frame(int, DecoderState*, AVFrame*);

#endif