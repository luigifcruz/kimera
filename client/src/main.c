#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <signal.h>

#include "utils.h"
#include "socket.h"
#include "decoder.h"
#include "loopback.h"
#include "display.h"
#include "config.h"

volatile sig_atomic_t stop;
typedef enum { STDOUT, SDL, LOOPBACK } output_t;

void inthand(int signum) {
    if (stop == 1) {
      exit(-1);
    }
    stop = 1;
}

int main(int argc, char *argv[]) {
    // Set options.
    output_t oopt = LOOPBACK;

    // Open ADB tunneling connection with the device.
    adb_open_tunnel(SOCKNAME_VS, APP_DOMAIN);
    adb_open_tunnel(SOCKNAME_AS, APP_DOMAIN);

    // Setup signaling to exit safely.
    signal(SIGINT, inthand);

    // Start LibAV decoder.
    DecoderState decoder;
    if (oopt == SDL || oopt == LOOPBACK) {
        if (!start_decoder(&decoder))
            goto cleanup;
    }

    // Start Loopback Ouput.
    int loopback_fd;
    if (oopt == LOOPBACK) {
        if ((loopback_fd = open_loopback(LOOPBACK_DEVICE)) < 0) {
            goto cleanup;
        }
    }

    // Connect to the device using UNIX Socket. 
    int socketfd = socket_open(SOCKNAME_VS); 
    if (socketfd < 0) {
      return 1;
    }
    
    // Start Decoder Loop.
    size_t out = 0;
    char header[HEADER_SIZE];

    while (!stop) { 
        out = socket_read_all(socketfd, (char*)&header, HEADER_SIZE); 
        if (out < HEADER_SIZE) {
            continue;
        }

        uint64_t pts = buffer_read64be(header);
        uint32_t len = buffer_read32be(&header[8]);
        assert(pts == NO_PTS || (pts & 0x8000000000000000) == 0);
        assert(len);

        char* packet = (char*)malloc(len);
        if (packet == NULL) {
            printf("Couldn't allocate packet.");
            goto cleanup;
        }
       
        out = socket_read_all(socketfd, packet, len); 
        assert(out == len);

        if (oopt == STDOUT) {
            fwrite(packet, sizeof(char), len, stdout);
            free(packet);
            continue;
        }

        AVFrame* frame = av_frame_alloc();
        if (decoder_push(&decoder, frame, packet, len, pts)) {
            if (oopt == SDL) {

            }

            if (oopt == LOOPBACK) {
                loopback_push_frame(loopback_fd, &decoder, frame);
            }
        };

        av_frame_free(&frame);
        free(packet);
    }

cleanup:
    close_decoder(&decoder);
    socket_close(socketfd);
}
