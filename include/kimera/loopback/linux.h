#ifndef LOOPBACK_LINUX_H
#define LOOPBACK_LINUX_H

#include "kimera/state.h"

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

    V4L2State* v4l2;
    XCBState* xcb;
} LoopbackState;

//
// Loopback Methods
//

bool open_loopback_sink(LoopbackState*, State*);
bool open_loopback_source(LoopbackState*, State*);

bool loopback_push_frame(LoopbackState*, AVFrame*);
bool loopback_pull_frame(LoopbackState*, State*);

void close_loopback(LoopbackState*, State*);

//
// XCB Methods
//

void init_xcb(LoopbackState*);
void close_xcb(LoopbackState*);

bool pull_xcb_frame(LoopbackState*, State*);
bool init_xcb_source(LoopbackState*, State*);

//
// V4L2 Methods
//

void init_v4l2(LoopbackState*);
void close_v4l2(LoopbackState*, State*);

bool init_v4l2_source(LoopbackState*, State*);
bool init_v4l2_sink(LoopbackState*, State*);

bool pull_v4l2_frame(LoopbackState*);
bool push_v4l2_frame(LoopbackState*, AVFrame*);

//
// Utils Methods
//

unsigned int ff_to_v4l(enum AVPixelFormat);
enum AVPixelFormat v4l_to_ff(unsigned int);
unsigned int find_v4l_format(int, unsigned int);

#endif
