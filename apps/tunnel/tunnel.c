#include <stdlib.h>
#include <stdio.h>

#include "kimera/socket.h"
#include "kimera/encoder.h"
#include "kimera/decoder.h"
#include "kimera/resampler.h"
#include "kimera/loopback.h"
#include "kimera/display.h"
#include "kimera/client.h"

void receiver(State* state, volatile sig_atomic_t* stop) {
    kimera_print_state(state);

    bool ok = true;
    DecoderState* decoder = alloc_decoder();
    LoopbackState* loopback = alloc_loopback();
    ResamplerState* resampler = alloc_resampler();
    SocketState* socket = alloc_socket();
    DisplayState* display = alloc_display();

    if (state->sink & DISPLAY)
        ok |= open_display(display, state);

    if (state->sink & LOOPBACK)
        ok |= open_loopback_sink(loopback, state);

    ok |= open_decoder(decoder,state);
    ok |= open_socket_client(socket, state);
    ok |= open_resampler(resampler, state->out_format);

    if (!ok) goto cleanup;

    // Start Decoder Loop.
    while (socket_recv_packet(socket) && !(*stop)) {
        if (decoder_push(decoder, socket->packet->payload, socket->packet->len, socket->packet->pts)) {
            if (!resampler_push_frame(resampler, state, decoder->frame))
                continue;
            
            if (state->sink & DISPLAY)
                display_draw(display, state, resampler->frame);

            if (state->sink & LOOPBACK)
                loopback_push_frame(loopback, resampler->frame);
        }
    }

cleanup:
    free_loopback(loopback, state);
    free_display(display);
    free_resampler(resampler);
    free_decoder(decoder);
    free_socket(socket);
}

void transmitter(State* state, volatile sig_atomic_t* stop) {
    kimera_print_state(state);

    bool ok = true;
    EncoderState* encoder = alloc_encoder();
    SocketState* socket = alloc_socket();
    LoopbackState* loopback = alloc_loopback();
    DisplayState* display = alloc_display();
    ResamplerState* resampler = alloc_resampler();

    ok |= open_socket_server(socket, state);
    ok |= open_loopback_source(loopback, state);

    if (state->sink & DISPLAY)
        ok |= open_display(display, state);
    
    ok |= open_encoder(encoder, state);
    ok |= open_resampler(resampler, state->out_format);

    if (!ok) goto cleanup;

    // Start Decoder Loop.
    while (loopback_pull_frame(loopback, state) && !(*stop)) {
        if (!resampler_push_frame(resampler, state, loopback->frame))
            continue;

        if (state->sink & DISPLAY)
            display_draw(display, state, resampler->frame);
        
        if (encoder_push(encoder, resampler->frame))
            socket_send_packet(socket, encoder->packet);
    }

cleanup:
    free_display(display);
    free_socket(socket);
    free_resampler(resampler);
    free_loopback(loopback, state);
    free_encoder(encoder);
}

int main(int argc, char *argv[]) {
    return kimera_client(argc, argv, transmitter, receiver);
}