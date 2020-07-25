#ifndef LOOPBACK_LINUX_H
#define LOOPBACK_LINUX_H

#include <cstddef>

extern "C" {
#include <xcb/xcb.h>
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <libavutil/time.h>
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <unistd.h>
}

#include "kimera/state.hpp"

namespace Kimera {

class XCB {
public:
    XCB(State&);
    ~XCB();

    bool SetSource();

    bool Push(AVFrame*);
    AVFrame* Pull();

private:
    State& state;

    int64_t last_frame;
    int64_t frame_duration;
    xcb_get_image_reply_t* img = NULL;
    xcb_connection_t *connection = NULL;
    xcb_screen_t *screen = NULL;
    AVFrame* frame = NULL;
};

class V4L2 {
public:
    V4L2(State&);
    ~V4L2();

    bool SetSink();
    bool SetSource();

    bool Push(AVFrame*);
    AVFrame* Pull();

private:
    State& state;

    int dev_fd;
    struct v4l2_buffer info;
    struct v4l2_format format;
    struct v4l2_requestbuffers req;
    char* buffer = NULL;
    AVFrame* frame = NULL;
};

class Loopback {
public:
    Loopback(State&);

    bool LoadSink();
    bool LoadSource();

    bool Push(AVFrame*);
    AVFrame* Pull();

private:
    State& state;

    XCB xcb;
    V4L2 v4l2;
};

unsigned int ff_to_v4l(enum AVPixelFormat);
enum AVPixelFormat v4l_to_ff(unsigned int);
unsigned int find_v4l_format(int, unsigned int);

} // namespace Kimera

#endif
