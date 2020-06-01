#include "kimera/receiver.h"

void receiver(State* state, volatile sig_atomic_t* stop) {
    // Start HEVC Decoder.
    DecoderState decoder;
    if (state->sink & DISPLAY || state->sink & LOOPBACK) {
        if (!start_decoder(&decoder,state))
            goto cleanup;
    }

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

    // Start Router. 
    RouterState router;
    if (!start_router(&router, state))
            goto cleanup;
    
    // Start Socket Client. 
    SocketState socket;
    switch (state->source) {
    case TCP:
        if(!open_tcp_client(&socket, state))
            goto cleanup;
        break;
    case UDP:
        if(!open_udp_client(&socket, state))
            goto cleanup;
        break;
    case UNIX:
        if(!open_unix_client(&socket, state))
            goto cleanup;
        break;
    default:
        goto cleanup;
    }
 
    // Start Decoder Loop.
    while (recv_packet(&router, &socket, stop) && !(*stop)) {
        if (state->sink & STDOUT) {
            fwrite(router.packet->payload, sizeof(char), router.packet->len, stdout);
            continue;
        }

        if (decoder_push(&decoder, router.packet->payload, router.packet->len, router.packet->pts)) {
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
    close_resampler(&resampler);
    close_decoder(&decoder);
    close_router(&router);
    
    switch (state->source) {
    case TCP:
        close_tcp(&socket);
        break;
    case UDP:
        close_udp(&socket);
        break;
    case UNIX:
        close_unix(&socket);
        break;
    default:
        break;
    }   

    if (state->sink & LOOPBACK)
        close_loopback(&loopback, state);

    if (state->sink & DISPLAY)
        close_display(&display);
}
