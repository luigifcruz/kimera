#include "receiver.h"

void receiver(State* state) {
    // Start HEVC Decoder.
    DecoderState decoder;
    if (state->source == DISPLAY || state->source == LOOPBACK) {
        if (!start_decoder(&decoder))
            goto cleanup;
    }

    // Start Loopback Ouput.
    LoopbackState loopback;
    if (state->sink == LOOPBACK) {
        if (!open_loopback_sink(&loopback, state))
            goto cleanup;
    }

    // Start Display Screen.
    DisplayState display;
    if (state->sink == DISPLAY) {
        if (!start_display(&display))
            goto cleanup;
    }

    // Start Socket Client. 
    int socketfd = -1; 
    switch (state->source) {
    case UNIX:
        socketfd = open_unix_socket(state);
        break;
    case TCP:
        socketfd = open_tcp_socket(state);
        break;
    default:
        socketfd = -1;
        break;
    }
    if (socketfd < 0)
        goto cleanup;

    // Start Decoder Loop.
    size_t out = 0;
    char header[HEADER_SIZE];

    while (stop) { 
        out = recv(socketfd, (char*)&header, HEADER_SIZE, MSG_WAITALL);
        if (out < HEADER_SIZE) {
            continue;
        }

        uint64_t pts = buffer_read64be((uint8_t*)header);
        uint32_t len = buffer_read32be((uint8_t*)&header[8]);
        assert(pts == NO_PTS || (pts & 0x8000000000000000) == 0);
        assert(len);

        char* packet = (char*)malloc(len);
        if (packet == NULL) {
            printf("[MAIN] Couldn't allocate packet.");
            goto cleanup;
        }
       
        out = recv(socketfd, packet, len, MSG_WAITALL); 
        assert(out == len);
        
        if (state->sink == STDOUT) {
            fwrite(packet, sizeof(char), len, stdout);
            free(packet);
            continue;
        }
        
        if (decoder_push(&decoder, packet, len, pts)) {
            if (state->sink == DISPLAY) {
                if (!display_draw(&display, decoder.frame))
                    break;
            }

            if (state->sink == LOOPBACK) {
                loopback_push_frame(&loopback, decoder.frame);
            }
        }
        
        free(packet);   
    }

cleanup:
    close_decoder(&decoder);
    
    switch (state->source) {
    case UNIX:
        close_unix_socket(socketfd);
        break;
    case TCP:
        close_tcp_socket(socketfd);
        break;
    default:
        break;
    }

    if (state->source == LOOPBACK)
        close_loopback(&loopback);
    if (state->source == DISPLAY)
        close_display(&display);
}
