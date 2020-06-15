#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>

#include "kimera/transport.h"
#include "kimera/codec.h"
#include "kimera/loopback.h"
#include "kimera/display.h"
#include "kimera/state.h"
#include "kimera/client.h"

void receiver(State* state, volatile sig_atomic_t* stop) {
    kimera_print_state(state);

    // Start Decoder.
    DecoderState decoder;
    if (!start_decoder(&decoder,state))
        goto cleanup;

    // Start Loopback Ouput.
    LoopbackState loopback;
    if (state->sink & LOOPBACK) {
        if (!open_loopback_sink(&loopback, state))
            goto cleanup;
    }

    // Add resampler.
    ResamplerState resampler;
    open_resampler(&resampler, state->out_format);

    // Start Display Screen.
    DisplayState display;
    if (state->sink & DISPLAY) {
        if (!start_display(&display, state))
            goto cleanup;
    }

    // Start Socket Server. 
    SocketState socket;
    if (!open_socket_client(&socket, state))
        goto cleanup;
 
    // Start Decoder Loop.
    while (socket_recv_packet(&socket) && !(*stop)) {
        if (decoder_push(&decoder, socket.packet->payload, socket.packet->len, socket.packet->pts)) {
            if (!resampler_push_frame(&resampler, state, decoder.frame))
                continue;
            
            if (state->sink & DISPLAY)
                display_draw(&display, state, resampler.frame);

            if (state->sink & LOOPBACK)
                loopback_push_frame(&loopback, resampler.frame);
        }
    }

cleanup:
    close_loopback(&loopback, state);
    close_display(&display);
    close_resampler(&resampler);
    close_decoder(&decoder);
    close_socket(&socket);
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

    // Start Display Screen.
    DisplayState display;
    if (state->sink & DISPLAY) {
        if (!start_display(&display, state))
            goto cleanup;
    }

    // Start Encoder.
    EncoderState encoder;
    if (!start_encoder(&encoder, state))
        goto cleanup;

    // Add resampler.
    ResamplerState resampler;
    open_resampler(&resampler, state->out_format);

    // Start Decoder Loop.
    while (loopback_pull_frame(&loopback, state) && !(*stop)) {
        if (!resampler_push_frame(&resampler, state, loopback.frame))
            continue;

        if (state->sink & DISPLAY)
            display_draw(&display, state, resampler.frame);
        
        if (encoder_push(&encoder, resampler.frame))
            socket_send_packet(&socket, encoder.packet);
    }

cleanup:
    close_display(&display);
    close_socket(&socket);
    close_resampler(&resampler);
    close_loopback(&loopback, state);
    close_encoder(&encoder);
}

int main(int argc, char *argv[]) {
    return kimera_client(argc, argv, transmitter, receiver);
}