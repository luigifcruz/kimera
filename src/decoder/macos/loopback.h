#ifndef LOOPBACK_H
#define LOOPBACK_H

typedef struct {
    int dev_fd;
    uint8_t* buffer;
} LoopbackState;

bool open_loopback(LoopbackState* state, char*);
void close_loopback(LoopbackState*);
void loopback_push_frame(LoopbackState* state, AVFrame*);

#endif