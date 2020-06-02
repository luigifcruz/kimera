#ifndef SOCKET_H
#define SOCKET_H

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h> 
#include <stdbool.h>
#include <sys/un.h>
#include <unistd.h>

#include "kimera/state.h"
#include "kimera/router.h"

typedef struct sockaddr_in socket_in;
typedef struct sockaddr_un socket_un;
typedef struct sockaddr socket_t;

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

#include "kimera/udp_socket.h"
#include "kimera/tcp_socket.h"
#include "kimera/unix_socket.h"

bool open_socket_server(SocketState*, State*);
bool open_socket_client(SocketState*, State*);

void close_socket(SocketState*);

int socket_recv_packet(SocketState*);
void socket_send_packet(SocketState*, AVPacket*);

int socket_send_buffer(SocketState*, const void*, size_t);
int socket_recv_buffer(SocketState*, void*, size_t);

#endif