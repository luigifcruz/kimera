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

void receiver(State* state, volatile sig_atomic_t* stop) {
    state->use_gpu = true;

    bool ok = true;
    DecoderState* decoder = init_decoder();
    LoopbackState* loopback = init_loopback();
    ResamplerState* resampler = init_resampler();
    SocketState* socket = init_socket();
    RenderState* render = init_render();

    if (state->sink & LOOPBACK)
        ok &= open_loopback_sink(loopback, state);

    ok &= open_decoder(decoder,state);
    ok &= open_socket_client(socket, state);
    ok &= open_resampler(resampler, state->out_format);
    ok &= open_render(render, state);

    if (!ok) goto cleanup;

    kimera_print_state(state);
    render_print_meta(render);

    while (socket_recv_packet(socket) && !(*stop)) {
        if (decoder_push(decoder, socket->packet->payload, socket->packet->len, socket->packet->pts)) {
            AVFrame* frame = decoder->frame;

            if (state->use_gpu) {
                if (!render_push_frame(render, frame)) break;
                if (state->vert_shader && state->frag_shader)
                    if (!render_proc_frame(render)) break;
                if (state->sink & DISPLAY)
                    if (!render_draw_frame(render)) break;
                if (state->sink & LOOPBACK) {
                    if (!render_pull_frame(render)) break;
                    frame = render->frame;
                }
            }

            if (!resampler_push_frame(resampler, state, frame))
                continue;

            if (state->sink & LOOPBACK)
                loopback_push_frame(loopback, resampler->frame);
        }
    }

cleanup:
    close_loopback(loopback, state);
    close_resampler(resampler);
    close_decoder(decoder);
    close_socket(socket);
}

void transmitter(State* state, volatile sig_atomic_t* stop) {
    state->use_gpu = true;

    bool ok = true;
    EncoderState* encoder = init_encoder();
    SocketState* socket = init_socket();
    LoopbackState* loopback = init_loopback();
    RenderState* render = init_render();
    ResamplerState* resampler = init_resampler();

    ok &= open_socket_server(socket, state);
    ok &= open_loopback_source(loopback, state);
    ok &= open_encoder(encoder, state);
    ok &= open_resampler(resampler, state->out_format);
    ok &= open_render(render, state);

    if (!ok) goto cleanup;

    kimera_print_state(state);
    render_print_meta(render);

    while (loopback_pull_frame(loopback, state) && !(*stop)) {
        AVFrame* frame = loopback->frame;

        if (state->use_gpu) {
            if (!render_push_frame(render, frame)) break;
            if (state->vert_shader && state->frag_shader)
                if (!render_proc_frame(render)) break;
            if (state->sink & DISPLAY)
                if (!render_draw_frame(render)) break;
            if (!render_pull_frame(render)) break;
            frame = render->frame;
        }

        if (!resampler_push_frame(resampler, state, frame))
            continue;

        if (encoder_push(encoder, resampler->frame))
            socket_send_packet(socket, encoder->packet);
    }

cleanup:
    close_render(render);
    close_socket(socket);
    close_resampler(resampler);
    close_loopback(loopback, state);
    close_encoder(encoder);
}

int main(int argc, char *argv[]) {
    return kimera_client(argc, argv, transmitter, receiver);
}