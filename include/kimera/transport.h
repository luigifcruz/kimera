#ifndef TRANSPORT_H
#define TRANSPORT_H

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h> 
#include <stdbool.h>
#include <sys/un.h>
#include <unistd.h>

#include "kimera/state.h"

typedef struct sockaddr_in socket_in;
typedef struct sockaddr_un socket_un;
typedef struct sockaddr socket_t;

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

typedef struct {
    int server_fd;
    int client_fd;
    Interfaces interface;

    // Router
    RouterState router;
    Packet* packet;

    // UDP & TCP Sockets
    socket_in* client_in;
    socket_in* server_in;

    // UNIX Sockets
    socket_un* client_un;
    socket_un* server_un;
} SocketState;

//
// General Methods
//

bool open_socket_server(SocketState*, State*);
bool open_socket_client(SocketState*, State*);

void close_socket(SocketState*);

int socket_recv_packet(SocketState*);
void socket_send_packet(SocketState*, AVPacket*);

int socket_send_buffer(SocketState*, const void*, size_t);
int socket_recv_buffer(SocketState*, void*, size_t);

//
// Router Methods
//

bool start_router(RouterState*, State*);
void close_router(RouterState*);

bool router_make_packet(RouterState*, AVPacket*);
bool router_parse_packet(RouterState*);

//
// TCP Socket Methods
//

bool open_tcp_client(SocketState*, State*);
bool open_tcp_server(SocketState*, State*);
void close_tcp(SocketState*);

//
// UDP Socket Methods
//

bool open_udp_client(SocketState*, State*);
bool open_udp_server(SocketState*, State*);
void close_udp(SocketState*);

//
// UNIX Socket Methods
//

bool open_unix_client(SocketState*, State*);
bool open_unix_server(SocketState*, State*);
void close_unix(SocketState*);

//
// Util Methods
//

static inline uint32_t
buffer_read32be(const uint8_t *buf) {
    return (buf[3] << 24) | (buf[2] << 16) | (buf[1] << 8) | buf[0];
}

static inline uint64_t
buffer_read64be(const uint8_t *buf) {
    uint32_t msb = buffer_read32be(buf);
    uint32_t lsb = buffer_read32be(&buf[4]);
    return ((uint64_t) lsb << 32) | msb;
}

#endif