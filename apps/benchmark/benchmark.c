#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <stdbool.h>
#include <string.h>

#include "kimera/transport.h"
#include "kimera/codec.h"
#include "kimera/loopback.h"
#include "kimera/render.h"
#include "kimera/client.h"

double clockToMilliseconds(clock_t ticks){
    return (ticks/(double)CLOCKS_PER_SEC)*1000.0;
}

void benchmark(State* state, volatile sig_atomic_t* stop) {
    bool ok = true;
    RenderState* render = init_render();
    AVFrame* frame = av_frame_alloc();

    clock_t deltaTime = 0;
    unsigned int frames = 0;
    double frameRate = 30;
    double averageFrameTimeMilliseconds = 33.333;

    ok &= open_render(render, state);

    if (!ok) goto cleanup;

    kimera_print_state(state);
    render_print_meta(render);

    frame->width = state->width;
    frame->height = state->height;
    frame->format = state->in_format;
    if (av_frame_get_buffer(frame, 0) < 0){
        printf("[BENCHMARK] Couldn't allocate frame.\n");
        goto cleanup;
    }

    while (!(*stop)) {
        clock_t beginFrame = clock();
        if (!render_push_frame(render, frame)) break;
        if (CHECK(state->sink, FILTER))
            if (!render_proc_frame(render)) break;
        if (CHECK(state->sink, DISPLAY))
            if (!render_draw_frame(render)) break;
        if (!render_pull_frame(render)) break;
        clock_t endFrame = clock();

        deltaTime += endFrame - beginFrame;
        frames ++;

        if (clockToMilliseconds(deltaTime) > 500.0) {
            frameRate = (double)frames*0.5 + frameRate*0.5;
            frames = 0;
            deltaTime -= CLOCKS_PER_SEC;
            averageFrameTimeMilliseconds = 1000.0/(frameRate==0?0.001:frameRate);

            printf("Frame-rate: %f\nAvg. Frame-time: %f\n", frameRate, averageFrameTimeMilliseconds);
        }
    }

cleanup:
    close_render(render);
}

int main(int argc, char *argv[]) {
    State* state = kimera_state();
    volatile sig_atomic_t stop = false;

    benchmark(state, &stop);

    return 0;
}