#include <stdlib.h>
#include <stdio.h>
#include <signal.h>

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

    State *state = malloc(sizeof(State));

    state->width     = DEFAULT_WIDTH;
    state->height    = DEFAULT_HEIGHT;
    state->bitrate   = DEFAULT_BITRATE;
    state->port      = DEFAULT_PORT;
    state->address   = DEFAULT_ADDRESS;
    state->codec     = DEFAULT_CODEC;
    state->loopback  = DEFAULT_LOOPBACK;
    state->format    = DEFAULT_FORMAT;
    state->framerate = DEFAULT_FRAMERATE;

    switch (state->format) {
    case AV_PIX_FMT_YUV420P:
        state->frame_size = (state->width*state->height*3/2);
        break;
    default:
        return -1;
    }

    transmitter(state);
}