#include "receiver.h"

void receiver(State* state) {
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

    // Start Display Screen.
    DisplayState display;
    if (state->sink & DISPLAY) {
        if (!start_display(&display))
            goto cleanup;
    }
    
    // Start Socket Client. 
    SocketState socket;
    switch (state->source) {
    case TCP:
        if(!open_tcp_client(&socket, state))
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
    size_t out = 0;
    char header[HEADER_SIZE];

    while (!stop) {
        out = recv(socket.server_fd, (char*)&header, HEADER_SIZE, MSG_WAITALL);
        if (out < HEADER_SIZE) {
            continue;
        }

        uint64_t pts = buffer_read64be((uint8_t*)header);
        uint32_t len = buffer_read32be((uint8_t*)&header[8]);

        char* packet = (char*)malloc(len);
        if (packet == NULL) {
            printf("[MAIN] Couldn't allocate packet.");
            goto cleanup;
        }
       
        out = recv(socket.server_fd, packet, len, MSG_WAITALL); 
        assert(out == len);

        if (state->sink & STDOUT) {
            fwrite(packet, sizeof(char), len, stdout);
            free(packet);
            continue;
        }

        if (decoder_push(&decoder, packet, len, pts)) {
            if (state->sink & DISPLAY) {
                printf("sinkd %d %d\n", decoder.frame->format, AV_PIX_FMT_NV12);
                if (!display_draw(&display, decoder.frame))
                    break;
            }

            if (state->sink & LOOPBACK) {
                printf("sinkl\n");
                loopback_push_frame(&loopback, decoder.frame);
            }
        }
        
        free(packet);   
    }

cleanup:
    close_decoder(&decoder);
    
    switch (state->source) {
    case TCP:
        close_tcp(&socket);
        break;
    case UNIX:
        close_unix(&socket);
        break;
    default:
        break;
    }

    if (state->sink == LOOPBACK)
        close_loopback(&loopback);
    if (state->sink == DISPLAY)
        close_display(&display);
}
