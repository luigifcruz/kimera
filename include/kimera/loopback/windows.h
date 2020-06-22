#ifndef LOOPBACK_WINDOWS_H
#define LOOPBACK_WINDOWS_H

#include <stdbool.h>

#include "kimera/state.h"

typedef struct {
	AVFrame* frame;
} LoopbackState;

LoopbackState* init_loopback();
void close_loopback(LoopbackState*, State*);

bool open_loopback_sink(LoopbackState*, State*);
bool open_loopback_source(LoopbackState*, State*);

bool loopback_push_frame(LoopbackState*, AVFrame*);
bool loopback_pull_frame(LoopbackState*, State*);

#endif
