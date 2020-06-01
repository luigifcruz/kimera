#ifndef SOCKET_H
#define SOCKET_H

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h> 
#include <sys/un.h>
#include <unistd.h>

#include "kimera/config.h"

typedef struct sockaddr_in socket_in;
typedef struct sockaddr_un socket_un;
typedef struct sockaddr socket_t;

typedef struct {
    int server_fd;
    int client_fd;
    Interfaces interface;

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

int send_socket(SocketState*, const void*, size_t);
int recv_socket(SocketState*, void*, size_t);

#endif