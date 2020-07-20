#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <stdbool.h>
#include <string.h>

#include "kimera/kimera.hpp"
#include "kimera/transport.hpp"
#include "kimera/loopback.hpp"
#include "kimera/codec.hpp"
#include "kimera/client.hpp"

void receiver(Client* cli) {
    Kimera* state = cli->GetState();
    AVPacket* packet = NULL;

    Socket socket(state);
    Loopback loopback(state);
    Decoder decoder(state);
    Resampler resampler(state, state->out_format);
//  RenderKimera* render = init_render();

    if (!loopback.SetSink()) return;
    if (!socket.OpenClient()) return;
//  ok &= open_render(render, state);

    cli->PrintState();
//  render_print_meta(render);

    while (!(packet = socket.RecvPacket()) && !cli->ShouldStop()) {
        if (decoder.Push(packet)) {
            AVFrame* frame = decoder.Pull();

/*
            Interfaces pipe = state->pipe;
            Interfaces sink = state->sink;

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
}

void transmitter(Client* cli) {
    Kimera* state = cli->GetState();
    AVFrame* frame = NULL;

    Socket socket(state);
    Loopback loopback(state);
    Encoder encoder(state);
    Resampler resampler(state, state->out_format);
//  RenderKimera* render = init_render();

    if (!socket.OpenServer()) return;
    if (!loopback.SetSource()) return;
//  ok &= open_render(render, state);

    cli->PrintState();
//  render_print_meta(render);

    while ((frame = loopback.Pull()) && cli->ShouldStop()) {
/*
        Interfaces pipe = state->pipe;
        Interfaces sink = state->source;

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
            socket.SendPacket(encoder.Pull());
    }
}

int main(int argc, char *argv[]) {
    Kimera state;
    Client client(&state);
    return client.Attach(argc, argv, transmitter, receiver);
}
