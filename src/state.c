#include "kimera/state.h"

State* kimera_state() {
    State *state = malloc(sizeof(State));

    state->width        = DEFAULT_WIDTH;
    state->height       = DEFAULT_HEIGHT;
    state->bitrate      = DEFAULT_BITRATE;
    state->port         = DEFAULT_PORT;
    state->in_format    = DEFAULT_FORMAT;
    state->out_format   = DEFAULT_FORMAT;
    state->framerate    = DEFAULT_FRAMERATE;
    state->packet_size  = DEFAULT_PACKET_SIZE;
    state->sink         = 0;
    state->source       = 0;
    state->use_gpu      = false;
    state->vert_shader  = NULL;
    state->frag_shader  = NULL;

    state->loopback = malloc(64);
    state->address = malloc(64);
    state->codec = malloc(64);

    strcpy(state->loopback, DEFAULT_LOOPBACK);
    strcpy(state->address, DEFAULT_ADDRESS);
    strcpy(state->codec, DEFAULT_CODEC);

    return state;
}

void kimera_free(State* state) {
    free(state->loopback);
    free(state->address);
    free(state->codec);
    free(state);
}