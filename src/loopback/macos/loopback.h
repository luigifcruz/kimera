#ifndef LOOPBACK_MAC_H
#define LOOPBACK_MAC_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>

#include "config.h"

typedef struct {
    AVFrame* frame;
    void* state;
} LoopbackState;

bool open_loopback_sink(LoopbackState*, State*);
bool open_loopback_source(LoopbackState*, State*);

bool loopback_push_frame(LoopbackState*, AVFrame*);
bool loopback_pull_frame(LoopbackState*, State*);

void close_loopback(LoopbackState*, State*);

#endif
