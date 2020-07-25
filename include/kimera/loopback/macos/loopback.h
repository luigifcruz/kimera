#ifndef LOOPBACK_MAC_H
#define LOOPBACK_MAC_H

#include "kimera/state.h"

typedef struct {
    AVFrame* frame;
    void* state;
} LoopbackState;

LoopbackState* init_loopback();
void close_loopback(LoopbackState*, State*);

bool open_loopback_sink(LoopbackState*, State*);
bool open_loopback_source(LoopbackState*, State*);

bool loopback_push_frame(LoopbackState*, AVFrame*);
bool loopback_pull_frame(LoopbackState*, State*);

#endif
