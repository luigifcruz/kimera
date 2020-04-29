#include "loopback.h"

bool open_loopback(LoopbackState* state, char* interface) {
    return true;
}

void close_loopback(LoopbackState* state) {
    return;
}

void loopback_push_frame(LoopbackState* state, AVFrame* frame) {
    return;
}