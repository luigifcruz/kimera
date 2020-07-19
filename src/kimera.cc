#include "kimera/kimera.hpp"

char* empty_string(size_t len) {
    char* string = (char*)malloc(len);
    string[0] = '\0';
    return string;
}

Kimera::Kimera() {
    width         = DEFAULT_WIDTH;
    height        = DEFAULT_HEIGHT;
    bitrate       = DEFAULT_BITRATE;
    port          = DEFAULT_PORT;
    in_format     = DEFAULT_FORMAT;
    out_format    = DEFAULT_FORMAT;
    framerate     = DEFAULT_FRAMERATE;
    packet_size   = DEFAULT_PACKET_SIZE;

    psk_key       = empty_string(256);
    vert_shader   = empty_string(256);
    frag_shader   = empty_string(256);
    loopback      = empty_string(64);
    address       = empty_string(64);
    codec         = empty_string(64);

    strcpy(loopback, DEFAULT_LOOPBACK);
    strcpy(address, DEFAULT_ADDRESS);
    strcpy(codec, DEFAULT_CODEC);
}

Kimera::~Kimera() {
    free(loopback);
    free(address);
    free(codec);
}
