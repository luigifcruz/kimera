#include "kimera/loopback/macos/interface.h"

#import "kimera/loopback/macos/CameraAdapter.h"

void* init_capture() {
    return (void*)[CameraAdapter new];
}

void stop_capture(void* proc) {
    [(id)proc stopCapture];
}

bool start_capture(void* proc, bool display, char* device, int fps, int width, int height) {
    return [(id)proc startCapture:display d:device f:fps w:width h:height];
}

bool pull_frame(void* proc, char* buffers[], int linesize[]) {
    return [(id)proc pullFrame:buffers linesize:linesize];
}

int get_frame_width(void* proc) {
    return [(id)proc getFrameWidth];
}

int get_frame_height(void* proc) {
    return [(id)proc getFrameHeight];
}