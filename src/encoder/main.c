#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h> 
#include <assert.h>
#include <signal.h>

#include "utils.h"
#include "unix_socket.h"
#include "tcp_socket.h"
#include "encoder.h"
#include "loopback.h"
#include "display.h"
#include "config.h"

volatile sig_atomic_t stop;
typedef enum { UNIX, TCP, STDOUT } output_t;

void inthand(int signum) {
    if (stop == 1) {
      exit(-1);
    }
    stop = 1;
}

int main(int argc, char *argv[]) {
    // Set options.
    output_t oopt = STDOUT;
    
    // Setup signaling to exit safely.
    signal(SIGINT, inthand);
    
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
    }
    if (socketfd < 0)
        goto cleanup;
*/

    // Start Encoder.
    EncoderState encoder;
    if (!start_encoder(&encoder))
            goto cleanup;

    // Start Loopback Input.
    LoopbackState loopback;
    if (!open_loopback(&loopback, LOOPBACK_DEVICE))
        goto cleanup;

    // Start Decoder Loop.
    while (loopback_pull_frame(&loopback) || !stop) {
        // Receive frame buffer from input device.

        if (encoder_push(&encoder, loopback.buffer)) {
            if (oopt == STDOUT) {
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
    }
*/

    close_loopback(&loopback);
    close_encoder(&encoder);
}
