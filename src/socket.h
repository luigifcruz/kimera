#ifndef SOCKET_H
#define SOCKET_H

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h> 
#include <sys/un.h>

typedef struct sockaddr_in socket_in;
typedef struct sockaddr_un socket_un;
typedef struct sockaddr socket_t;

typedef struct {
    int server_fd;
    int client_fd;
} SocketState;

#endif