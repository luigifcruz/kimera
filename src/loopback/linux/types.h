#ifndef LOOPBACK_TYPES_LINUX_H
#define LOOPBACK_TYPES_LINUX_H

#include <linux/videodev2.h>

typedef struct {
    xcb_get_image_reply_t* img;
    xcb_connection_t *connection;
    xcb_screen_t *screen;
} XCBState;

typedef struct {
    int dev_fd;
    char* buffer;
    struct v4l2_buffer info;
    struct v4l2_format format;
    struct v4l2_requestbuffers req;
} V4L2State;

typedef struct {
    AVFrame* frame;
    int64_t frame_duration;
    int64_t last_frame;
    V4L2State v4l2;
    XCBState xcb;
} LoopbackState;

#endif