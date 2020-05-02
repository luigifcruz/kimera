#include "loopback.h"

bool open_loopback(LoopbackState* state, char* interface) {
    printf("[MACOS_LOOPBACK] This isn't supported yet.\n");
    return false;
}

void close_loopback(LoopbackState* state) {
    return;
}

void loopback_push_frame(LoopbackState* state, AVFrame* frame) {
    return;
}