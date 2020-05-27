#ifndef LOOPBACK_XCB_LINUX_H
#define LOOPBACK_XCB_LINUX_H

#include <xcb/xcb.h>
#include <xcb/composite.h>

bool pull_xcb_frame(LoopbackState*, State*);
bool init_xcb_source(LoopbackState*, State*);

#endif