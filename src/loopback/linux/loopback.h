#ifndef LOOPBACK_LINUX_H
#define LOOPBACK_LINUX_H

#include "utils.h"
#include "config.h"
#include "types.h"
#include "v4l2.h"
#include "xcb.h"

bool open_loopback_sink(LoopbackState*, State*);
bool open_loopback_source(LoopbackState*, State*);

bool loopback_push_frame(LoopbackState*, AVFrame*);
bool loopback_pull_frame(LoopbackState*, State*);

void close_loopback(LoopbackState*, State*);

#endif
