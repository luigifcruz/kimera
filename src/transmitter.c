#include "transmitter.h"

void transmitter(State* state) {
/*
    // Start Socket Client. 
    int socketfd = -1; 
    switch (iopt) {
    case UNIX:
        socketfd = open_unix_socket(SOCKNAME_VS);
        break;
    case TCP:
        socketfd = open_tcp_socket(SOCKNAME_VS, SERVER_IP, SERVER_PORT);
        break;
    default:
        socketfd = -1;
        break;
    }
    if (socketfd < 0)
        goto cleanup;
*/

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
            if (state->sink == STDOUT) {
                fwrite(
                    encoder.packet->data, sizeof(char),
                    encoder.packet->size, stdout);
                continue;
            }


        }
    }

cleanup:
/*    
    switch (oopt) {
    case UNIX:
        close_unix_socket(socketfd);
        break;
    case TCP:
        close_tcp_socket(socketfd);
        break;
    default:
        break;
    }
*/

    close_loopback(&loopback);
    close_encoder(&encoder);
}
