#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>

#include "config.h"
#include "transmitter.h"
#include "receiver.h"

void inthand(int signum) {
    if (stop == 1) {
      exit(-1);
    }
    stop = 1;
}

int main(int argc, char *argv[]) {
    // Register signal handler.
    signal(SIGINT, inthand);

    // Declare Default Settings
    State *state = malloc(sizeof(State));

    state->width        = DEFAULT_WIDTH;
    state->height       = DEFAULT_HEIGHT;
    state->bitrate      = DEFAULT_BITRATE;
    state->port         = DEFAULT_PORT;
    state->address      = DEFAULT_ADDRESS;
    state->codec        = DEFAULT_CODEC;
    state->loopback     = DEFAULT_LOOPBACK;
    state->in_format    = DEFAULT_FORMAT;
    state->out_format   = DEFAULT_FORMAT;
    state->framerate    = DEFAULT_FRAMERATE;

    // Parse Arguments
    if (argc < 2) {
        printf("Flags\n");
        return -1;
    }

    if (!strcmp(argv[1], "tx") || !strcmp(argv[1], "transmit")) {
        state->source = LOOPBACK;
        state->sink = TCP | DISPLAY;
        state->codec = "h264_nvenc";
        state->mode = TRANSMITTER;
    } else if (!strcmp(argv[1], "rx") || !strcmp(argv[1], "receive")) {
        state->source = TCP;
        state->sink = DISPLAY;
        state->codec = "h264_cuvid";
        state->mode = RECEIVER;
    } else {
        printf("Not such flag (%s)\n", argv[1]);
    }

    switch (state->out_format) {
    case AV_PIX_FMT_YUV420P:
        state->frame_size = (state->width*state->height*3/2);
        break;
    default:
        return -1;
    }

    switch (state->mode) {
    case RECEIVER:
        receiver(state);
        break;
    case TRANSMITTER:
        transmitter(state);
        break;
    }

    return 0;
}
