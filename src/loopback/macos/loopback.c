#include "loopback.h"

bool open_loopback_sink(LoopbackState* loopback, State* state) {
    printf("[MACOS_LOOPBACK] This isn't supported yet.\n");
    return false;
}

bool open_loopback_source(LoopbackState* loopback, State* state) {
    printf("[MACOS_LOOPBACK] This isn't supported yet.\n");
    return false;
}

bool loopback_push_frame(LoopbackState* loopback, AVFrame* state) {
    return false;
}

bool loopback_pull_frame(LoopbackState* loopback) {
    return false;
}

void close_loopback(LoopbackState* loopback) {
    return;
}