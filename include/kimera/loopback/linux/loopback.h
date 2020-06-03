#ifndef LOOPBACK_LINUX_H
#define LOOPBACK_LINUX_H

#include "kimera/state.h"

#include "utils.h"
#include "types.h"
#include "v4l2.h"
#include "xcb.h"

LoopbackState* alloc_loopback();
void free_loopback(LoopbackState*, State*);

bool open_loopback_sink(LoopbackState*, State*);
bool open_loopback_source(LoopbackState*, State*);

bool loopback_push_frame(LoopbackState*, AVFrame*);
bool loopback_pull_frame(LoopbackState*, State*);

#endif
