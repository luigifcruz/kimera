
#define KIMERA_WINDOW_GLFW

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>

#include "kimera/state.h"

#include "kimera/transport.h"
#include "kimera/codec.h"
#include "kimera/loopback.h"
#include "kimera/display.h"
#include "kimera/client.h"
#include "render/render.h"
#include "render/meta.h"
#include "draw.h"

void custom_shader(CanvasState* render, void* object) {
    set_uniform1f(render->proc_shader, "count", *((float*)object));
}

void transmitter(State* state, volatile sig_atomic_t* stop) {
    kimera_print_state(state);
    
    // Start Socket Server. 
    SocketState socket;
    if (!open_socket_server(&socket, state))
        goto cleanup;

    // Start Loopback Input.
    LoopbackState loopback;
    if (!open_loopback_source(&loopback, state))
        goto cleanup;

    // Start Encoder.
    EncoderState encoder;
    if (!start_encoder(&encoder, state))
        goto cleanup;

    // Add resampler.
    ResamplerState resampler;
    open_resampler(&resampler, state->out_format);
    

    RenderState* render = alloc_render();

    render->mode = WINDOWED;
    render->api  = EGL_OPENGL_ES_API;

    if (!start_render(render, state)) goto cleanup;
    render_print_meta(render);

    CanvasState canvas;
    start_render2(&canvas, state);

    float i = 1.0;

    // Start Decoder Loop.
    while (loopback_pull_frame(&loopback, state) && !(*stop)) {
        if (!resampler_push_frame(&resampler, state, loopback.frame))
            continue;

        AVFrame* frame = resampler.frame;

        canvas.d_height = render->height;
        canvas.d_width = render->width;
        
        i++;

        if (!render_push_frame(&canvas, frame)) break;
        if (!render_proc_frame(&canvas, NULL, NULL)) break;
        if (!render_draw_frame(&canvas)) break;
        if (!render_pull_frame(&canvas)) break;
        
        if (!render_commit_frame(render)) break;

        if (encoder_push(&encoder, canvas.frame))
            socket_send_packet(&socket, encoder.packet);
    }

cleanup:
    close_socket(&socket);
    close_resampler(&resampler);
    close_loopback(&loopback, state);
    close_encoder(&encoder);
    free_render(render);
}

void receiver(State* state, volatile sig_atomic_t* stop) {}

int main(int argc, char *argv[]) {
    return kimera_client(argc, argv, transmitter, receiver);
}
