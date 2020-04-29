#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h> 
#include <assert.h>
#include <signal.h>

#include "utils.h"
#include "unix_socket.h"
#include "tcp_socket.h"
#include "decoder.h"
#include "loopback.h"
#include "display.h"
#include "config.h"

volatile sig_atomic_t stop;
typedef enum { STDOUT, DISPLAY, LOOPBACK } output_t;
typedef enum { UNIX, TCP } input_t;

void inthand(int signum) {
    if (stop == 1) {
      exit(-1);
    }
    stop = 1;
}

int main(int argc, char *argv[]) {
    // Set options.
    output_t oopt = DISPLAY;
    input_t iopt = TCP;

    // Open ADB tunneling connection with the device.
    if (iopt == UNIX) {
        adb_open_tunnel(SOCKNAME_VS, APP_DOMAIN);
        adb_open_tunnel(SOCKNAME_AS, APP_DOMAIN);
    }
    
    // Setup signaling to exit safely.
    signal(SIGINT, inthand);

    // Start HEVC Decoder.
    DecoderState decoder;
    if (oopt == DISPLAY || oopt == LOOPBACK) {
        if (!start_decoder(&decoder))
            goto cleanup;
    }

    // Start Loopback Ouput.
    LoopbackState loopback;
    if (oopt == LOOPBACK) {
        if (!open_loopback(&loopback, LOOPBACK_DEVICE))
            goto cleanup;
    }

    // Start Display Screen.
    DisplayState display;
    if (oopt == DISPLAY) {
        if (!start_display(&display))
            goto cleanup;
    }

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
    if (socketfd < 0) {
        goto cleanup;
    }

    // Start Decoder Loop.
    size_t out = 0;
    char header[HEADER_SIZE];

    while (!stop) { 
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
        
        if (oopt == STDOUT) {
            fwrite(packet, sizeof(char), len, stdout);
            free(packet);
            continue;
        }
        
        if (decoder_push(&decoder, packet, len, pts)) {
            if (oopt == DISPLAY) {
                if (!display_draw(&display, decoder.frame))
                    break;
            }

            if (oopt == LOOPBACK) {
                loopback_push_frame(&loopback, decoder.frame);
            }
        }
        
        free(packet);   
    }

cleanup:
    close_decoder(&decoder);
    
    switch (iopt) {
        case UNIX:
            close_unix_socket(socketfd);
            break;
        case TCP:
            close_tcp_socket(socketfd);
            break;
    }

    if (oopt == LOOPBACK)
        close_loopback(&loopback);
    if (oopt == DISPLAY)
        close_display(&display);
}
