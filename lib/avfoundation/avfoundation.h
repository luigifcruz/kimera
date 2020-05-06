#ifndef AVFOUNDATION_H
#define AVFOUNDATION_H

#include <stddef.h>
#include <stdbool.h>

typedef struct {
    void* adapter;
} avfoundation_state;

bool avfoundation_open_source(avfoundation_state*);
void avfoundation_close_source(avfoundation_state*);

bool avfoundation_pull_frame(avfoundation_state*, void* Y, void* U, void* V);

#endif