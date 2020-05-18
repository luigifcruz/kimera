#include "transmitter.h"

void transmitter(State* state, volatile sig_atomic_t* stop) {
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

    // Start Router. 
    RouterState router;
    if (state->sink & UNIX || state->sink & TCP) {
        if (!start_router(&router, state))
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

    // Performance Degradation Check
    if (state->in_format != state->out_format) {
        printf("[TRANSMITTER] Performance Degradation:\n");
        printf("[TRANSMITTER] Output pixel format is different than the input.\n");
        printf("[TRANSMITTER] - Input: %s -> Output: %s\n",
               av_get_pix_fmt_name(state->in_format),
               av_get_pix_fmt_name(state->out_format));
    }

    // Start Decoder Loop.
    while (loopback_pull_frame(&loopback) && !(*stop)) {
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

            while (make_packet(&router, encoder.packet, resampler.frame)) {
                if (state->sink & TCP)
                    send_packet(&router, tcp_socket.client_fd);
                if (state->sink & UNIX)
                    send_packet(&router, unix_socket.client_fd);
            }
        }
    }

cleanup:
    if (state->sink & TCP)
        close_tcp(&tcp_socket);

    if (state->sink & UNIX)
        close_unix(&unix_socket);

    if (state->sink & TCP || state->sink & UNIX)
        close_router(&router);

    if (state->sink & DISPLAY)
        close_display(&display);

    close_loopback(&loopback);
    close_encoder(&encoder);
}
