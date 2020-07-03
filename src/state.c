#include "kimera/state.h"

char* empty_string(size_t len) {
    char* string = (char*)malloc(len);
    string[0] = '\0';
    return string;
}

State* kimera_state() {
    State *state = malloc(sizeof(State));

    state->width         = DEFAULT_WIDTH;
    state->height        = DEFAULT_HEIGHT;
    state->bitrate       = DEFAULT_BITRATE;
    state->port          = DEFAULT_PORT;
    state->in_format     = DEFAULT_FORMAT;
    state->out_format    = DEFAULT_FORMAT;
    state->framerate     = DEFAULT_FRAMERATE;
    state->packet_size   = DEFAULT_PACKET_SIZE;
 
    state->sink          = 0;
    state->pipe          = 0;
    state->source        = 0;

    state->psk_key       = empty_string(256);
    state->vert_shader   = empty_string(256);
    state->frag_shader   = empty_string(256);
    state->loopback      = empty_string(64);
    state->address       = empty_string(64);
    state->codec         = empty_string(64);

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
