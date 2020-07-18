#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <stdbool.h>
#include <string.h>

extern "C" {
#include "kimera/transport.h"
//#include "kimera/render.h"
#include "kimera/client.h"
}

#include "kimera/loopback.hpp"
#include "kimera/codec.hpp"

void receiver(State* state, volatile sig_atomic_t* stop) {
    bool ok = true;
    SocketState* socket = init_socket();
//    RenderState* render = init_render();

    Loopback loopback(state);
    Decoder decoder(state);
    Resampler resampler(state, state->out_format);

    if (state->sink & LOOPBACK)
         ok &= loopback.SetSink();
    ok &= open_socket_client(socket, state);
    //ok &= open_render(render, state);

    if (!ok) goto cleanup;

    kimera_print_state(state);
   // render_print_meta(render);

    while (socket_recv_packet(socket) && !(*stop)) {
        if (decoder.Push(socket->packet->payload, socket->packet->len, socket->packet->pts)) {
            AVFrame* frame = decoder.Pull();
            Interfaces pipe = state->pipe;
            Interfaces sink = state->sink;

/*
            if (pipe & GPU_RESAMPLE || pipe & FILTER || sink & DISPLAY)
                if (!render_push_frame(render, frame)) break;
            if (pipe & FILTER)
                if (!render_proc_frame(render)) break;
            if (sink & DISPLAY)
                if (!render_draw_frame(render)) break;
            if (pipe & GPU_RESAMPLE || pipe & FILTER) {
                if (!render_pull_frame(render)) break;
                frame = render->frame;
            }
*/
            if (!resampler.Push(frame)) break;

            if (state->sink & LOOPBACK)
                if (!loopback.Push(resampler.Pull())) break;
        }
    }

cleanup:
    close_socket(socket);
}

void transmitter(State* state, volatile sig_atomic_t* stop) {
    bool ok = true;
    AVFrame* frame = NULL;

    SocketState* socket = init_socket();
//  RenderState* render = init_render();

    Loopback loopback(state);
    Encoder encoder(state);
    Resampler resampler(state, state->out_format);

    ok &= open_socket_server(socket, state);
    ok &= loopback.SetSource();
//  ok &= open_render(render, state);

    if (!ok) goto cleanup;

    kimera_print_state(state);
//  render_print_meta(render);

    while ((frame = loopback.Pull()) != NULL && !(*stop)) {
        Interfaces pipe = state->pipe;
        Interfaces sink = state->source;
/*
        if (pipe & GPU_RESAMPLE || pipe & FILTER || sink & DISPLAY)
            if (!render_push_frame(render, frame)) break;
        if (pipe & FILTER)
            if (!render_proc_frame(render)) break;
        if (sink & DISPLAY)
            if (!render_draw_frame(render)) break;
        if (pipe & GPU_RESAMPLE || pipe & FILTER) {
            if (!render_pull_frame(render)) break;
            frame = render->frame;
        }
*/
        if (!resampler.Push(frame)) break;

        if (encoder.Push(resampler.Pull()))
            socket_send_packet(socket, encoder.Pull());
    }

cleanup:
//  close_render(render);
    close_socket(socket);
}

int main(int argc, char *argv[]) {
    return kimera_client(argc, argv, transmitter, receiver);
}
