#ifndef LOOPBACK_XCB_LINUX_H
#define LOOPBACK_XCB_LINUX_H

#include <stdbool.h>
#include <xcb/xcb.h>
#include <libavutil/time.h>
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>

#include "types.h"
#include "utils.h"

#include "kimera/config.h"

void init_xcb(LoopbackState*);
void close_xcb(LoopbackState*);

bool pull_xcb_frame(LoopbackState*, State*);
bool init_xcb_source(LoopbackState*, State*);

#endif