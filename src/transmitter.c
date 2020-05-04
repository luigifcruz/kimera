#include "transmitter.h"

void transmitter(State* state) {
    // Start Socket Client. 
    TCPSocketState socket;
    switch (state->sink) {
    case TCP:
        if (!open_tcp_server(&socket, state))
            goto cleanup;
        break;
    default:
        goto cleanup;
    }

    // Start Encoder.
    EncoderState encoder;
    if (!start_encoder(&encoder, state))
            goto cleanup;

    // Start Loopback Input.
    LoopbackState loopback;
    if (!open_loopback_source(&loopback, state))
        goto cleanup;

    // Start Decoder Loop.
    while (loopback_pull_frame(&loopback) || !stop) {
        // Receive frame buffer from input device.

        if (encoder_push(&encoder, loopback.buffer)) {
            if (state->sink & STDOUT) {
                fwrite(
                    encoder.packet->data, sizeof(char),
                    encoder.packet->size, stdout);
                continue;
            }

        }
    }

cleanup:
    switch (state->sink) {
    case TCP:
        close_tcp(&socket);
        break;
    default:
        break;
    }

    close_loopback(&loopback);
    close_encoder(&encoder);
}
