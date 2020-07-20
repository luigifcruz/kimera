#include "kimera/kimera.hpp"

char* empty_string(size_t len) {
    char* string = (char*)malloc(len);
    string[0] = '\0';
    return string;
}

Kimera::Kimera() {
    AVPacket* packet = av_packet_alloc();
            av_init_packet(packet);
            
    strcpy(loopback, DEFAULT_LOOPBACK);
    strcpy(address, DEFAULT_ADDRESS);
    strcpy(codec, DEFAULT_CODEC);
}

Kimera::~Kimera() {
    free(loopback);
    free(address);
    free(codec);
}
