#ifndef LOOPBACK_UTILS_LINUX_H
#define LOOPBACK_UTILS_LINUX_H

#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include <libavcodec/avcodec.h>

unsigned int ff_to_v4l(enum AVPixelFormat);
enum AVPixelFormat v4l_to_ff(unsigned int);
unsigned int find_v4l_format(int, unsigned int);

#endif