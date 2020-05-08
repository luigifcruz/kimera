#include "transmitter.h"

void transmitter(State* state) {
    // Start TCP Server. 
    SocketState tcp_socket;
    if (state->sink & TCP) {
        if (!open_tcp_server(&tcp_socket, state))
            goto cleanup;
    }

    // Start UNIX Server. 
    SocketState unix_socket;
    if (state->sink & UNIX) {
        if (!open_unix_server(&unix_socket, state))
            goto cleanup;
    }

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
    while (loopback_pull_frame(&loopback)) {

        if (!resampler_push_frame(&resampler, state, loopback.frame)) {
            continue;
        }

        if (state->sink & DISPLAY) {
            if (!display_draw(&display, resampler.frame))
                break;
        }
        
        if (encoder_push(&encoder, resampler.frame)) {
            if (state->sink & STDOUT) {
                fwrite(
                    encoder.packet->data, sizeof(char),
                    encoder.packet->size, stdout);
                continue;
            }

            if (state->sink & TCP) {
                write(tcp_socket.client_fd, (char*)&resampler.frame->pts, sizeof(resampler.frame->pts));
                write(tcp_socket.client_fd, (char*)&encoder.packet->size, sizeof(encoder.packet->size)); 
                write(tcp_socket.client_fd, encoder.packet->data, encoder.packet->size); 
            }

            if (state->sink & UNIX) {
                write(unix_socket.client_fd, (char*)&resampler.frame->pts, sizeof(resampler.frame->pts));
                write(unix_socket.client_fd, (char*)&encoder.packet->size, sizeof(encoder.packet->size)); 
                write(unix_socket.client_fd, encoder.packet->data, encoder.packet->size); 
            }
        }
    }

cleanup:
    if (state->sink & TCP)
        close_tcp(&tcp_socket);

    if (state->sink & UNIX)
        close_unix(&unix_socket);

    if (state->sink & DISPLAY)
        close_display(&display);

    close_loopback(&loopback);
    close_encoder(&encoder);
}
