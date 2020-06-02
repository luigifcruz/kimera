#include "transmitter.h"

void transmitter(State* state, volatile sig_atomic_t* stop) {
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
        if (!resampler_push_frame(&resampler, state, loopback.frame)) {
            continue;
        }

        if (state->sink & DISPLAY) {
            if (!display_draw(&display, state, resampler.frame))
                break;
        }
        
        if (encoder_push(&encoder, resampler.frame)) {
            if (state->sink & STDOUT) {
                fwrite(
                    encoder.packet->data, sizeof(char),
                    encoder.packet->size, stdout);
                continue;
            }

            socket_send_packet(&socket, encoder.packet);
        }
    }
    
cleanup:
    close_display(&display);
    close_socket(&socket);
    close_resampler(&resampler);
    close_loopback(&loopback, state);
    close_encoder(&encoder);
}