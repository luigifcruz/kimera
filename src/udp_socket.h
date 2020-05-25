#ifndef UDP_SOCKET_H
#define UDP_SOCKET_H

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>

#include "config.h"
#include "socket.h"

bool open_udp_client(SocketState*, State*);
bool open_udp_server(SocketState*, State*);
void close_udp(SocketState*);

#endif