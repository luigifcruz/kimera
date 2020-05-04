#ifndef TCP_SOCKET_H
#define TCP_SOCKET_H

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>

#include "config.h"
#include "socket.h"

bool open_tcp_client(SocketState*, State*);
bool open_tcp_server(SocketState*, State*);
void close_tcp(SocketState*);

#endif