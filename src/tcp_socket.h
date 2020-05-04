#ifndef TCP_SOCKET_H
#define TCP_SOCKET_H

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h> 

#include "config.h"

typedef struct sockaddr_in socket_in;
typedef struct sockaddr socket_t;

typedef struct {
    int server_fd;
    int client_fd;
} TCPSocketState;

bool open_tcp_client(TCPSocketState*, State*);
bool open_tcp_server(TCPSocketState*, State*);
void close_tcp(TCPSocketState*);

#endif