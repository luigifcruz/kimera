#ifndef LOOPBACK_MAC_H
#define LOOPBACK_MAC_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>
#include <avfoundation.h>

#include "../../config.h"

typedef struct {
    AVFrame* frame;
    avfoundation_state state;
} LoopbackState;

static bool open_loopback_sink(LoopbackState*, State*);
static bool open_loopback_source(LoopbackState*, State*);

static bool loopback_push_frame(LoopbackState*, AVFrame*);
static bool loopback_pull_frame(LoopbackState*);

static void close_loopback(LoopbackState*);

#endif