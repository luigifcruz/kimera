
#define KIMERA_WINDOW_GLFW

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>

#include "kimera/state.h"

#include "kimera/transport.h"
#include "kimera/codec.h"
#include "kimera/loopback.h"
#include "kimera/client.h"
#include "render/render.h"

void transmitter(State* state, volatile sig_atomic_t* stop) {
    bool ok = true;
    
    DecoderState* decoder = alloc_decoder();
    LoopbackState* loopback = alloc_loopback();
    ResamplerState* resampler = alloc_resampler();
    SocketState* socket = alloc_socket();
    RenderState* render = alloc_render();

    if (state->sink & LOOPBACK)
        ok &= open_loopback_sink(loopback, state);

    ok &= open_decoder(decoder,state);
    ok &= open_socket_client(socket, state);
    ok &= open_resampler(resampler, state->out_format);
    ok &= open_render(render, state);

    if (!ok) goto cleanup;
       
    kimera_print_state(state);
    render_print_meta(render);

    // Start Decoder Loop.
    while (loopback_pull_frame(&loopback, state) && !(*stop)) {
        if (!resampler_push_frame(&resampler, state, loopback.frame))
            continue;

        if (!render_push_frame(&render, resampler.frame))
            break;
        
        if (!render_commit_frame(render)) break;

        if (encoder_push(&encoder, canvas.frame))
            socket_send_packet(&socket, encoder.packet);
    }

cleanup:
    close_socket(&socket);
    close_resampler(&resampler);
    close_loopback(&loopback, state);
    close_encoder(&encoder);
    close_render(render);
}

void receiver(State* state, volatile sig_atomic_t* stop) {}

int main(int argc, char *argv[]) {
    return kimera_client(argc, argv, transmitter, receiver);
}
