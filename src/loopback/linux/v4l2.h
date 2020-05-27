#ifndef LOOPBACK_V4L2_LINUX_H
#define LOOPBACK_V4L2_LINUX_H

#include <stdbool.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include <libavutil/time.h>
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#include "types.h"
#include "config.h"
#include "utils.h"

void init_v4l2(LoopbackState*);
void close_v4l2(LoopbackState*, State*);

bool init_v4l2_source(LoopbackState*, State*);
bool init_v4l2_sink(LoopbackState*, State*);

bool pull_v4l2_frame(LoopbackState*);
bool push_v4l2_frame(LoopbackState*, AVFrame*);

#endif