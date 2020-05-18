#ifndef ROUTER_H
#define ROUTER_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <libavcodec/avcodec.h>

#include "utils.h"
#include "config.h"

typedef struct {
    uint64_t pts;
    uint32_t len;
    uint32_t i;
    uint32_t n;
    char* payload;
} Packet;

typedef struct {
    int payload_size;
    int header_size;
    int packet_size;
    uint32_t checksum;
    Packet* packet;
    char* buffer;
} RouterState;

bool start_router(RouterState*, State*);
void close_router(RouterState*);

bool recv_packet(RouterState*, volatile sig_atomic_t*, int);

bool make_packet(RouterState*, AVPacket*, AVFrame*);
void send_packet(RouterState*, int);

#endif
