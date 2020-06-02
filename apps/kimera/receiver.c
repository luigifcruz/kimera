#include "receiver.h"

void receiver(State* state, volatile sig_atomic_t* stop) {
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
        Packet* packet = socket.router.packet;

        if (state->sink & STDOUT) {
            fwrite(packet->payload, sizeof(char), packet->len, stdout);
            continue;
        }

        if (decoder_push(&decoder, packet->payload, packet->len, packet->pts)) {
            if (!resampler_push_frame(&resampler, state, decoder.frame)) {
                continue;
            }

            if (state->sink & DISPLAY) {
                if (!display_draw(&display, state, resampler.frame))
                    break;
            }

            if (state->sink & LOOPBACK) {
                loopback_push_frame(&loopback, resampler.frame);
            }
        }
    }

cleanup:
    close_loopback(&loopback, state);
    close_display(&display);
    close_resampler(&resampler);
    close_decoder(&decoder);
    close_socket(&socket);
}